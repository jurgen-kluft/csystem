#include "xbase\x_types.h"
#include "xsystem/x_system.h"
#include "xsystem\private\x_system_win32.h"
#include "xunittest\xunittest.h"




UNITTEST_SUITE_BEGIN(exe_info)
{
	UNITTEST_FIXTURE(info)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			xcore::s32 codeSize = xcore::xsystem::getCodeSegmentSize();
			CHECK_TRUE(codeSize >= 0);

			xcore::s32 dataSize = xcore::xsystem::getDataSegmentSize();
			CHECK_TRUE(dataSize >= 0);

			xcore::s32 bssSize	= xcore::xsystem::getBssSegmentSize();
			CHECK_TRUE(bssSize >= 0);

			xcore::s32 stackSize = xcore::xsystem::getMainThreadStackSize();
			CHECK_TRUE(stackSize >= 0);

		}
	}
}
UNITTEST_SUITE_END