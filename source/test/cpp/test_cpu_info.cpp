#include "xbase/x_target.h"
#include "xsystem/x_system.h"
#include "xsystem/private/x_system_win32.h"
#include "xunittest/xunittest.h"

extern xcore::x_iallocator* gTestAllocator;
extern xcore::xsystem::xctxt* gCtxt;

UNITTEST_SUITE_BEGIN(cpu_info)
{
	UNITTEST_FIXTURE(info)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			

		}
	}
}
UNITTEST_SUITE_END