package csystem

import (
	"github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	"github.com/jurgen-kluft/centry/package"
	"github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'csystem'
func GetPackage() *denv.Package {
	// Dependencies
	unittestpkg := cunittest.GetPackage()
	entrypkg := centry.GetPackage()
	basepkg := cbase.GetPackage()

	// The main (csystem) package
	mainpkg := denv.NewPackage("csystem")
	mainpkg.AddPackage(unittestpkg)
	mainpkg.AddPackage(entrypkg)
	mainpkg.AddPackage(basepkg)

	// 'csystem' library
	mainlib := denv.SetupDefaultCppLibProject("csystem", "github.com\\jurgen-kluft\\csystem")
	mainlib.Dependencies = append(mainlib.Dependencies, basepkg.GetMainLib())

	// 'csystem' unittest project
	maintest := denv.SetupDefaultCppTestProject("csystem_test", "github.com\\jurgen-kluft\\csystem")
	maintest.Dependencies = append(maintest.Dependencies, unittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, entrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, basepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
