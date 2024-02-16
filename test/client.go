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
	IP = "127.0.0.1"
	PORT = "8080"
)

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

func inverseBarExpander(input *string){
	*input = strings.Replace(*input, "\\n", "\n", -1)

	*input = strings.Replace(*input, "\\r", "\r", -1)
}

func main() {
	conn, err := net.Dial("tcp", IP + ":" + PORT)
	if err != nil {
		fmt.Println("Error connecting:", err)
		return
	}
	defer conn.Close()

	scanner := bufio.NewScanner(os.Stdin)

	args := os.Args

	for {
		var input string

		if len(args) < 2 {
			fmt.Print("> ")
			scanner.Scan()
			input = scanner.Text()
		} else {
			filePath := args[1]

			f, err := os.Open(filePath)
			if err != nil {
				fmt.Println("Error opening file:", err)
				return
			}
			defer f.Close()

			fileContent, err := ioutil.ReadAll(f)
			if err != nil {
				fmt.Println("Error reading file:", err)
				return
			}

			input = string(fileContent)
		}

		if input != "" {
			if strings.Contains(input, "%") {
				percentageExpander(&input)
			}

			if strings.Contains(input, "\\") {
				inverseBarExpander(&input)
			}

			fmt.Printf("Message to send: %s\n", input)

			fmt.Fprintf(conn, input)

			message, err := bufio.NewReader(conn).ReadString(0)
			if err != nil {
				fmt.Println("Error reading from server:", err)
				return
			}

			fmt.Printf("Message received: %s\n", message)

			if len(args) >= 2 {
				return
			}
		}
	}
}
