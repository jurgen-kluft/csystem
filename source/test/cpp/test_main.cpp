#include "xunittest\xunittest.h"
#include "xbase\x_types.h"
#include "xsystem\x_system.h"

UNITTEST_SUITE_LIST(xCoreUnitTest);

UNITTEST_SUITE_DECLARE(xCoreUnitTest, cpu_info);
UNITTEST_SUITE_DECLARE(xCoreUnitTest, exe_info);
UNITTEST_SUITE_DECLARE(xCoreUnitTest, basic_info);


int main(int argc, const char** argv)
{
	xcore::xsystem::ESystemFlags eSysFlag = xcore::xsystem::ESystemFlags::MODE_DEV;
	xcore::xsystem::Initialise("D:\\project\xcode\\xsystem\\target\\xsystem\\outdir\\xsystem_test_TestRelease_Win32",eSysFlag);

	int r = 0;

	UnitTest::TestReporterStdout reporter;
	r = UNITTEST_SUITE_RUN(reporter, xCoreUnitTest);

	xcore::xsystem::shutdown();

	return r;
}
