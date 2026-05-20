#ifndef __CSYSTEM_SYSTEM_H__
#define __CSYSTEM_SYSTEM_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE
#    pragma once
#endif

namespace ncore
{
    class alloc_t;
    
    namespace nsystem
    {
        enum ELanguage
        {
            LANGUAGE_INVALID    = -1,
            LANGUAGE_ENGLISH    = 0,
            LANGUAGE_FRENCH     = 1,
            LANGUAGE_ITALIAN    = 2,
            LANGUAGE_GERMAN     = 3,
            LANGUAGE_SPANISH    = 4,
            LANGUAGE_GREEK      = 5,
            LANGUAGE_ENGLISH_US = 6,
            LANGUAGE_FRENCH_US  = 7,
            LANGUAGE_PORTUGUESE = 8,
            LANGUAGE_BRAZILIAN  = 9, ///< Brazilian Portuguese
            LANGUAGE_JAPANESE   = 10,
            LANGUAGE_KOREAN     = 11, ///< Korean
            LANGUAGE_RUSSIAN    = 12, ///< Russian
            LANGUAGE_DUTCH      = 13,
            LANGUAGE_CHINESE_T  = 14,
            LANGUAGE_CHINESE_S  = 15,
            LANGUAGE_FINNISH    = 16,
            LANGUAGE_SWEDISH    = 17,
            LANGUAGE_DANISH     = 18,
            LANGUAGE_NORWEGIAN  = 19,
            LANGUAGE_POLISH     = 20,

            LANGUAGE_COUNT,

            LANGUAGE_DEFAULT = LANGUAGE_ENGLISH,
            LANGUAGE_MAIN    = LANGUAGE_DEFAULT
        };

        enum EConsoleType
        {
            CONSOLE_DESKTOP,
            CONSOLE_DEVKIT,
            CONSOLE_TESTKIT,
            CONSOLE_RETAIL,
        };

        enum EMediaType
        {
            MEDIA_LOCAL   = 0x01,
            MEDIA_NETWORK = 0x02,
            MEDIA_CD      = 0x04,
            MEDIA_DVD     = 0x08,
            MEDIA_BLURAY  = 0x10,
            MEDIA_SD      = 0x20,
            MEDIA_HDD     = 0x40,
            MEDIA_SSD     = 0x80
        };

        enum EDataSource
        {
            DATA_FROM_HOST     = 0x00,
            DATA_FROM_DVD      = 0x10,
            DATA_FROM_NETWORK  = 0x20,
            DATA_FROM_HARDDISK = 0x40,
            DATA_FROM_SSD      = 0x80,
            DATA_FROM_MASK     = 0xFF,
        };

        enum EEnvMode
        {
            ENVIRONMENT_OPEN        = 0,
            ENVIRONMENT_DEBUG       = 1,
            ENVIRONMENT_DEVELOPMENT = 2,
            ENVIRONMENT_QA          = 3,
            ENVIRONMENT_RETAIL      = 4,
        };

        static void init(alloc_t*);
        static void update();
        static void shutdown();

        static bool isLittleEndian();
        static bool isBigEndian();
        static bool is64BitOS();

        static EConsoleType getConsoleType();
        static EMediaType   getMediaType();
        static EDataSource  getDataSource();
        static EEnvMode     getEnvMode();

        static ELanguage   getLanguage();
        static const char* getLanguageString(ELanguage);
        static void        setLanguage(ELanguage language);

        static u64 getTotalMemorySize();
        static u64 getCurrentSystemMemory();

        static const char* getUnixEndLine();
        static const char* getWindowsEndLine();
        static const char* getEndLine();

        static void setUserName(const char* szUserName);
        static void getUserName(char* szBuffer, u32 szBufferMaxLen);

        static bool isCircleButtonBack();

        static void        setAppTitle(const char* title);
        static const char* getAppTitle();
        static const char* getExePath();

        static s32 getNumCores();
        static s32 getNumHwThreadsPerCore();
        static u64 getCoreClockFrequency();

        static const char* getPlatformName();    // Name of platform
        static const char* getBuildConfigName(); // Debug, Release, Final
        static const char* getBuildModeName();   // Dev, Client, Retail
        static const char* getConsoleTypeName(); //
        static const char* getMediaTypeName();   //
        static const char* getLanguageName();

        static u64 getCodeSegmentSize();
        static u64 getBssSegmentSize();
        static u64 getDataSegmentSize();
        static u64 getMainThreadStackSize();
    }

} // namespace ncore

#endif // __CSYSTEM_SYSTEM_H__
