package main

import (
	"fmt"
	"io/ioutil"
	"os"
)

func main() {
	data, err := ioutil.ReadAll(os.Stdin)
	if err != nil {
		fmt.Println("Error al leer stdin:", err)
		return
	}

	fmt.Println("Hola! Bienvenido al mundo de Go!\n")

	for _, env := range os.Environ() {
		fmt.Println(env)
	}

	fmt.Println(string(data))
}
