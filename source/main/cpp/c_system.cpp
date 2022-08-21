#include "cbase/c_target.h"

#include "cbase/c_debug.h"
#include "cbase/c_string_ascii.h"

#include "csystem/c_system.h"


namespace ncore
{
	namespace nsystem
	{
		const char* sLanguageStr[] = 
		{
			"English",
			"French",
			"Italian",
			"German",
			"Spanish",
			"Greek",
			"English US",
			"French US",
			"Portuguese",
			"Brazilian",
			"Japanese",
			"Chinese",
			"Korean",
			"Russian",
			"Dutch",
			"Chinese traditional",
			"Chinese simplified",
			"Finnish",
			"Swedish",
			"Danish",
			"Norwegian",
			"Polish",
		};

		const char*		gToString(ELanguage language)
		{
			return (sLanguageStr[language]);
		}

	};
};


