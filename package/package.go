package csystem

import (
	cbase "github.com/jurgen-kluft/cbase/package"
	"github.com/jurgen-kluft/ccode/denv"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'csystem'
func GetPackage() *denv.Package {
	// Dependencies
	cunittestpkg := cunittest.GetPackage()
	cbasepkg := cbase.GetPackage()

	// The main (csystem) package
	mainpkg := denv.NewPackage("csystem")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(cbasepkg)

	// 'csystem' library
	mainlib := denv.SetupCppLibProject("csystem", "github.com\\jurgen-kluft\\csystem")
	mainlib.AddDependencies(cbasepkg.GetMainLib()...)

	// 'csystem' unittest project
	maintest := denv.SetupDefaultCppTestProject("csystem_test", "github.com\\jurgen-kluft\\csystem")
	maintest.AddDependencies(cunittestpkg.GetMainLib()...)
	maintest.AddDependencies(cbasepkg.GetMainLib()...)
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
