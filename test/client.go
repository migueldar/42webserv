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
var conexiones []net.Conn
var modeSelect int

func main() {
	var mode string
	modeSelect = 1

	for {
		var input string

		if modeSelect == 1 {
			if conexiones != nil{
				closeConnections()
			}
			modeSelect = 0
			mode = selectMode()
			switch mode {
			case "multi":
				handleMultiMode()
			case "fileInput":
				handleFileInputMode()
			case "single":
				handleSingleMode()
			default:
				fmt.Println("Err bad mode")
				modeSelect = 1
				continue
			}
		}
		if conexiones == nil {
			fmt.Println("Connection refused try again later")
			modeSelect = 1
			continue
		}

		switch mode {
		case "fileInput":
			input = modeFileReq()
		default:
			input = modeSingleReq()
		}

		if input != "" {
			processInput(&input)

			fmt.Printf("Message to send: %s\n", input)

			for i := 0; i < len(conexiones); i++ {
				fmt.Fprintf(conexiones[i], input)
			}

			receiveMessages(conexiones, mode)
		}
	}
}

func processInput(input *string) {
	if strings.Contains(*input, "%") {
		percentageExpander(input)
	}

	if strings.Contains(*input, "\\") {
		inverseBarExpander(input)
	}
}

func handleMultiMode() {
	fmt.Print("Input number of connections to make:\n")
	fmt.Print("> ")
	scanner := newScanner()
	scanner.Scan()
	if err := scanner.Err(); err != nil {
		fmt.Println("Error scanning input:", err)
		return
	}

	connectionNum, err := strconv.Atoi(scanner.Text())
	if err != nil {
		fmt.Println("Error parsing number of connections:", err)
		return
	}

	logger, err := os.Create("./multiTestClientLogs/logsTxtMulti")
	if err != nil {
		fmt.Println("Error creating log file:", err)
		return
	}
	defer logger.Close()

	conexiones = establishConnections(connectionNum)
}

func handleFileInputMode() {
	conexiones = establishConnections(1)
}

func handleSingleMode() {
	conexiones = establishConnections(1)
}

func newScanner() *bufio.Scanner {
	return bufio.NewScanner(os.Stdin)
}

func selectMode() string {
	fmt.Print("Input Req Mode:\n----> Single Request: \"single\"\n----> File Request:   \"fileInput\"\n----> Multi Request:  \"multi\"\n")
	fmt.Print("> ")

	scanner := newScanner()
	if !scanner.Scan() {
		if err := scanner.Err(); err != nil {
			fmt.Println("Error scanning input:", err)
		} else {
			exit()
		}
	}

	mode := scanner.Text()
	return mode
}

func modeFileReq() string {
	fmt.Print("Filepath:> ")

	scanner := newScanner()
	if !scanner.Scan() {
		if err := scanner.Err(); err != nil {
			fmt.Println("Error scanning file path:", err)
		} else {
			modeSelect = 1
		}
		return ""
	}

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

	scanner := newScanner()
	if !scanner.Scan() {
		if err := scanner.Err(); err != nil {
			fmt.Println("Error scanning input:", err)
		} else {
			modeSelect = 1
		}
		return ""
	}

	input := scanner.Text()
	return input
}

func establishConnections(numConnections int) []net.Conn {
	var conexiones []net.Conn

	for i := 0; i < numConnections; i++ {
		conn, err := net.Dial("tcp", IP+":"+PORT)
		if err != nil {
			fmt.Printf("Error connecting to server %d: %s\n", i, err)
			return nil
		}
		conexiones = append(conexiones, conn)
	}

	return conexiones
}

func receiveMessages(conexiones []net.Conn, mode string) {
	switch mode {
	case "multi":
		logger, err := os.OpenFile("./multiTestClientLogs/logsTxtMulti", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
		if err != nil {
			fmt.Println("Error opening log file:", err)
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

		if err := logger.Sync(); err != nil {
			fmt.Println("Error syncing log file:", err)
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

func exit() {
	fmt.Println("Exiting...")
	os.Exit(0)
}

func closeConnections(){
	for i := 0; i < len(conexiones); i++{
		(conexiones[i]).Close()
	}
}

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
