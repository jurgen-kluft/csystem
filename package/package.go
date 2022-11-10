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
	cunittestpkg := cunittest.GetPackage()
	centrypkg := centry.GetPackage()
	cbasepkg := cbase.GetPackage()

	// The main (csystem) package
	mainpkg := denv.NewPackage("csystem")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(centrypkg)
	mainpkg.AddPackage(cbasepkg)

	// 'csystem' library
	mainlib := denv.SetupDefaultCppLibProject("csystem", "github.com\\jurgen-kluft\\csystem")
	mainlib.Dependencies = append(mainlib.Dependencies, cbasepkg.GetMainLib())

	// 'csystem' unittest project
	maintest := denv.SetupDefaultCppTestProject("csystem_test", "github.com\\jurgen-kluft\\csystem")
	maintest.Dependencies = append(maintest.Dependencies, cunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, centrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, cbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
