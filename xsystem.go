package main

import (
	"github.com/jurgen-kluft/xcode"
	"github.com/jurgen-kluft/xsystem/package"
)

func main() {
	xcode.Init()
	xcode.Generate(xsystem.GetPackage())
}
