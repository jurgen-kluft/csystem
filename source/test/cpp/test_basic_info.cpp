#include "cbase/c_target.h"
#include "xsystem/x_system.h"
#include "xsystem/private/x_system_win32.h"
#include "cunittest/cunittest.h"

extern ncore::x_iallocator* gTestAllocator;
extern ncore::xsystem::xctxt* gCtxt;

UNITTEST_SUITE_BEGIN(basic_info)
{
	UNITTEST_FIXTURE(console)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			ncore::xsystem::EConsoleType consoleType = ncore::xsystem::getConsoleType(gCtxt);
#ifdef TARGET_PC
			CHECK_TRUE(consoleType == ncore::xsystem::EConsoleType::CONSOLE_DESKTOP);
#endif
			
		}
	}


	UNITTEST_FIXTURE(media)
	{
		UNITTEST_FIXTURE_SETUP() { }
		UNITTEST_FIXTURE_TEARDOWN() { }

		UNITTEST_TEST(check)
		{
			ncore::xsystem::EMediaType mediaType = ncore::xsystem::getMediaType(gCtxt);
#ifdef TARGET_PC
			CHECK_TRUE(mediaType == ncore::xsystem::EMediaType::MEDIA_HDD);
#endif

		}
	}

	UNITTEST_FIXTURE(language)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			ncore::xsystem::setLanguage(gCtxt, ncore::xsystem::ELanguage::LANGUAGE_ENGLISH);//ncore::xsystem::GetLanguage();
			ncore::xsystem::ELanguage lan = ncore::xsystem::getLanguage(gCtxt);
			CHECK_TRUE(lan == ncore::xsystem::ELanguage::LANGUAGE_ENGLISH);

		}
	}

	UNITTEST_FIXTURE(otherinfo)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			const char* path = ncore::xsystem::getExePath(gCtxt);
			CHECK_TRUE(path != NULL);

			//ncore::u32 maxSystemMemory = ncore::xsystem::getMaxSystemMemory();
			//CHECK_TRUE(maxSystemMemory >= 0);

			//ncore::u32 currentSystemMemory = ncore::xsystem::getCurrentSystemMemory();
			//CHECK_TRUE(currentSystemMemory>= 0);

		}
	}


}
UNITTEST_SUITE_END