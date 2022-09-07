package main

import (
	"github.com/jurgen-kluft/ccode"
	"github.com/jurgen-kluft/csystem/package"
)

func main() {
	ccode.Init()
	ccode.Generate(csystem.GetPackage())
}
