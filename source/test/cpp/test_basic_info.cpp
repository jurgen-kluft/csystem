#include "xbase\x_types.h"
#include "xsystem/x_system.h"
#include "xsystem\private\x_system_win32.h"
#include "xunittest\xunittest.h"




UNITTEST_SUITE_BEGIN(basic_info)
{
	UNITTEST_FIXTURE(console)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			xcore::xsystem::EConsoleType consoleType = xcore::xsystem::getConsoleType();
#ifdef TARGET_PC
			CHECK_TRUE(consoleType == xcore::xsystem::EConsoleType::CONSOLE_DEVKIT);
#endif
			
		}
	}


	UNITTEST_FIXTURE(media)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			xcore::xsystem::EMediaType mediaType = xcore::xsystem::getMediaType();
#ifdef TARGET_PC
			CHECK_TRUE(mediaType == xcore::xsystem::EMediaType::MEDIA_HOST);
#endif

		}
	}

	UNITTEST_FIXTURE(language)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			xcore::xsystem::setLanguage(xcore::xsystem::ELanguage::LANGUAGE_ENGLISH);//xcore::xsystem::GetLanguage();
			xcore::xsystem::ELanguage lan = xcore::xsystem::getLanguage();
			CHECK_TRUE(lan == xcore::xsystem::ELanguage::LANGUAGE_ENGLISH);

		}
	}

	UNITTEST_FIXTURE(otherinfo)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			const char* path = xcore::xsystem::getExePath();
			CHECK_TRUE(path != NULL);

			//xcore::u32 maxSystemMemory = xcore::xsystem::getMaxSystemMemory();
			//CHECK_TRUE(maxSystemMemory >= 0);

			//xcore::u32 currentSystemMemory = xcore::xsystem::getCurrentSystemMemory();
			//CHECK_TRUE(currentSystemMemory>= 0);

		}
	}


}
UNITTEST_SUITE_END