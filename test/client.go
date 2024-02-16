package main

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"strconv"
	"strings"
)

const (
	IP   = "127.0.0.1"
	PORT = "8080"
)

var scanner *bufio.Scanner

func percentageExpander(input *string) {
	var index int

	for index = strings.Index(*input, "%"); index != -1; index = strings.Index(*input, "%") {
		hexStr := strings.ToLower((*input)[index+1 : index+3])
		hexInt, err := strconv.ParseInt(hexStr, 16, 32)
		if err != nil {
			fmt.Println("Error parsing hex:", err)
			return
		}
		character := rune(hexInt)

		*input = strings.Replace(*input, (*input)[index:index+3], string(character), -1)
	}
}

func inverseBarExpander(input *string) {
	*input = strings.Replace(*input, "\\n", "\n", -1)
	*input = strings.Replace(*input, "\\r", "\r", -1)
}

func modeFileReq() string {
	fmt.Print("Filepath:> ")
	scanner.Scan()
	filePath := scanner.Text()

	fileContent, err := ioutil.ReadFile(filePath)
	if err != nil {
		fmt.Println("Error reading file:", err)
		return ""
	}

	input := string(fileContent)

	return input
}

func modeSingleReq() string {
	fmt.Print("> ")
	scanner.Scan()
	input := scanner.Text()

	return input
}

func establishConnections(numConnections int) []net.Conn {
	var conexiones []net.Conn

	for i := 0; i < numConnections; i++ {
		conn, err := net.Dial("tcp", IP+":"+PORT)
		if err != nil {
			fmt.Println("Error connecting:", err)
			return nil
		}
		conexiones = append(conexiones, conn)
		defer conn.Close()
	}

	return conexiones
}

func receiveMessages(conexiones []net.Conn, mode string) {
	switch mode {
	case "multi":
		logger, err := os.Create("/multiTestClientLogs")
		if err != nil {
			fmt.Println("Error creating log file:", err)
			return
		}
		defer logger.Close()

		for i := 0; i < len(conexiones); i++ {
			message, err := bufio.NewReader(conexiones[i]).ReadString(0)
			if err != nil {
				fmt.Println("Error reading from server:", err)
				return
			}
			logger.WriteString("CLIENT NUM(" + strconv.Itoa(i) + ") OUTPUT:\n")
			logger.WriteString(message)
		}
	default:
		message, err := bufio.NewReader(conexiones[0]).ReadString(0)
		if err != nil {
			fmt.Println("Error reading from server:", err)
			return
		}
		fmt.Printf("Message received: %s\n", message)
	}
}

func main() {
	var conexiones []net.Conn

	scanner = bufio.NewScanner(os.Stdin)

	fmt.Print("Input Req Mode:\n----> Single Request: \"single\"\n----> File Request:   \"fileInput\"\n----> Multi Request:  \"multi\"\n")
	fmt.Print("> ")
	scanner.Scan()
	mode := scanner.Text()

	switch mode {
	case "multi":
		fmt.Print("Input number of connections to make:\n")
		fmt.Print("> ")
		scanner.Scan()
		connectionNum, err := strconv.Atoi(scanner.Text())
		if err != nil {
			fmt.Println("Error parsing number of connections:", err)
			return
		}

		conexiones = establishConnections(connectionNum)
	default:
		conexiones = establishConnections(1)
	}

	for {
		var input string

		switch mode {
		case "fileInput":
			input = modeFileReq()
		default:
			input = modeSingleReq()
		}

		if input != "" {
			if strings.Contains(input, "%") {
				percentageExpander(&input)
			}

			if strings.Contains(input, "\\") {
				inverseBarExpander(&input)
			}

			fmt.Printf("Message to send: %s\n", input)

			for i := 0; i < len(conexiones); i++ {
				fmt.Fprintf(conexiones[i], input)
			}

			receiveMessages(conexiones, mode)
		}
	}
}

