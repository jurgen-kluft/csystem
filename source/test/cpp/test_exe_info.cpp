#include "cbase/c_target.h"
#include "csystem/c_system.h"
#include "csystem/private/c_system_win32.h"

#include "cunittest/cunittest.h"


UNITTEST_SUITE_BEGIN(exe_info)
{
	UNITTEST_FIXTURE(info)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			ncore::s32 codeSize = ncore::xsystem::getCodeSegmentSize(gCtxt);
			CHECK_TRUE(codeSize >= 0);

			ncore::s32 dataSize = ncore::xsystem::getDataSegmentSize(gCtxt);
			CHECK_TRUE(dataSize >= 0);

			ncore::s32 bssSize	= ncore::xsystem::getBssSegmentSize(gCtxt);
			CHECK_TRUE(bssSize >= 0);

			ncore::s32 stackSize = ncore::xsystem::getMainThreadStackSize(gCtxt);
			CHECK_TRUE(stackSize >= 0);

		}
	}
}
UNITTEST_SUITE_END