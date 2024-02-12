package test

import (
	"fmt"
	"os/exec"
	"testing"
)

const (
	GOOD = false
	BAD  = true
)

type Test struct {
	DesiredResult bool
	FileToTest    string
}

func runTestParser(file string) (int, error) {
	cmd := exec.Command("./webserv", file)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return cmd.ProcessState.ExitCode(), fmt.Errorf("Error ejecutando el programa C++: %v, output %s", err, output)
	}

	return cmd.ProcessState.ExitCode(), nil
}

func testGoodExec(file string, t *testing.T) bool {
	exitCode, err := runTestParser(file)
	if exitCode == 0 && err == nil {
		return true
	}
	t.Fatalf("Error en la prueba exitosa para el archivo %s: %v", file, err)
	return false
}

func testBadExec(file string, t *testing.T) bool {
	exitCode, err := runTestParser(file)
	if err != nil || exitCode == 1 {
		return true
	}
	t.Fatalf("Error en la prueba fallida para el archivo %s: %v", file, err)
	return false
}

func tester(t *testing.T) {
	tests := []Test{
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
		{DesiredResult: GOOD, FileToTest: "/"},
	}

	for _, test := range tests {
		if test.DesiredResult {
			if !testGoodExec(test.FileToTest, t) {
				t.Errorf("La prueba exitosa para el archivo %s falló", test.FileToTest)
			}
		} else {
			if !testBadExec(test.FileToTest, t) {
				t.Errorf("La prueba fallida para el archivo %s debería haber fallado pero tuvo éxito", test.FileToTest)
			}
		}
	}
}
