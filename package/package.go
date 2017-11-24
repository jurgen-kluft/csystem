package xhash

import (
	"github.com/jurgen-kluft/xbase/package"
	"github.com/jurgen-kluft/xcode/denv"
	"github.com/jurgen-kluft/xentry/package"
	"github.com/jurgen-kluft/xunittest/package"
)

// GetPackage returns the package object of 'xhash'
func GetPackage() *denv.Package {
	// Dependencies
	xunittestpkg := xunittest.GetPackage()
	xentrypkg := xentry.GetPackage()
	xbasepkg := xbase.GetPackage()

	// The main (xhash) package
	mainpkg := denv.NewPackage("xhash")
	mainpkg.AddPackage(xunittestpkg)
	mainpkg.AddPackage(xentrypkg)
	mainpkg.AddPackage(xbasepkg)

	// 'xhash' library
	mainlib := denv.SetupDefaultCppLibProject("xhash", "github.com\\jurgen-kluft\\xhash")
	mainlib.Dependencies = append(mainlib.Dependencies, xbasepkg.GetMainLib())

	// 'xhash' unittest project
	maintest := denv.SetupDefaultCppTestProject("xhash_test", "github.com\\jurgen-kluft\\xhash")
	maintest.Dependencies = append(maintest.Dependencies, xunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xentrypkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, xbasepkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}
