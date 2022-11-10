package main

import (
	"github.com/jurgen-kluft/ccode"
	csystem "github.com/jurgen-kluft/csystem/package"
)

func main() {
	ccode.Generate(csystem.GetPackage())
}
