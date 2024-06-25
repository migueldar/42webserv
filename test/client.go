package main

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"os"
	"strconv"
	"strings"
	"time"
)

const (
	readTimeout  = 300 * time.Millisecond
	checkTimeout = 100 * time.Millisecond
)

const (
	IP   = "192.168.122.1"
	PORT = "8080"
)

var scanner *bufio.Scanner
var connection net.Conn
var modeSelect int

func main() {
	var mode string
	modeSelect = 1

	for {
		var input string
		if modeSelect == 1 {
			if connection != nil {
				closeConnection()
			}
			modeSelect = 0
			mode = selectMode()
			switch mode {
			case "2":
				handleFileInputMode()
			case "1":
				handleSingleMode()
			default:
				fmt.Println("Err bad mode")
				modeSelect = 1
				continue
			}
		}
		if connection == nil {
			fmt.Println("Connection refused, try again later")
			modeSelect = 1
			continue
		}

		inputCh := make(chan string)
		outputCh := make(chan string)

		go func() {
			for {
				select {
				case msg := <-inputCh:
					if isConnectionAvailable(connection) {
						if msg != "" {
							fmt.Fprintf(connection, msg)
						}
					}
				}
			}
		}()

		go func() {
			for {
				select {
				case <-time.After(1 * time.Millisecond):
					outputCh <- printMultiMessages(connection)
				}
			}
		}()

		go func() {
			for {
				select {
				case msg := <-outputCh:
					if msg != "" {
						fmt.Printf("Received message: %s\n", msg)
					}
				}
			}
		}()

		switch mode {
		case "2":
			input = modeFileReq()
		default:
			input = modeSingleReq()
		}
		if input != "" {
			processInput(&input)
			fmt.Println("(" + input + ")")

			inputCh <- input
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

func isConnectionAvailable(conn net.Conn) bool {
	conn.SetReadDeadline(time.Now().Add(checkTimeout))
	defer conn.SetReadDeadline(time.Time{})

	_, err := conn.Read([]byte{0})
	if err != nil {
		if netErr, ok := err.(net.Error); ok && netErr.Timeout() {
			return true
		}
		return false
	}

	return true
}

func handleFileInputMode() {
	connection = establishConnection()
}

func handleSingleMode() {
	connection = establishConnection()
}

func newScanner() *bufio.Scanner {
	return bufio.NewScanner(os.Stdin)
}

func selectMode() string {
	fmt.Print("\nInput Req Mode:\n----> Single Request: \"1\"\n----> File Request:   \"2\"\n")
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
	if filePath == "" {
		return ""
	}

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

func establishConnection() net.Conn {
	conn, err := net.Dial("tcp", IP+":"+PORT)
	if err != nil {
		fmt.Printf("Error connecting to server: %s\n", err)
		return nil
	}
	return conn
}

const blockSize = 1024

func receiveMultiMessagesToString(conexion net.Conn) string {
	var buffer bytes.Buffer

	var message bytes.Buffer
	buf := make([]byte, blockSize)

	for {
		n, err := conexion.Read(buf)
		if err != nil {
			if err == io.EOF {
				break
			} else {
				return ""
			}
		}

		message.Write(buf[:n])
		if n < blockSize {
			break
		}
	}

	buffer.WriteString(fmt.Sprint(message.String()))

	return buffer.String()
}

func printMultiMessages(conexion net.Conn) string {
	message := receiveMultiMessagesToString(conexion)
	if strings.Contains(message, "Connection: close") {
		fmt.Println("Connection closed by remote server")
		modeSelect = 1
		return ""
	}
	return message
}

func exit() {
	fmt.Println("Exiting...")
	os.Exit(0)
}

func closeConnection() {
	connection.Close()
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