#include "ccore/c_target.h"
#include "cbase/c_allocator.h"

#include "csystem/c_system.h"
#include "csystem/test_allocator.h"

#include "cunittest/cunittest.h"


UNITTEST_SUITE_BEGIN(exe_info)
{
	UNITTEST_FIXTURE(info)
	{
        UNITTEST_ALLOCATOR;

		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			ncore::system_t s;
            s.init(Allocator);

			ncore::s32 codeSize = s.getCodeSegmentSize();
			CHECK_TRUE(codeSize >= 0);

			ncore::s32 dataSize = s.getDataSegmentSize();
			CHECK_TRUE(dataSize >= 0);

			ncore::s32 bssSize = s.getBssSegmentSize();
			CHECK_TRUE(bssSize >= 0);

			ncore::s32 stackSize = s.getMainThreadStackSize();
			CHECK_TRUE(stackSize >= 0);

		}
	}
}
UNITTEST_SUITE_END