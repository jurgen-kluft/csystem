#include "xbase/x_target.h"


//==============================================================================
// INCLUDES
//==============================================================================

#include "xbase/x_debug.h"
#include "xbase/x_string_std.h"

#include "xsystem/x_system.h"


//==============================================================================
// xCore namespace
//==============================================================================
namespace xcore
{
	namespace xsystem
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

	//==============================================================================
	// END xCore namespace
	//==============================================================================
};


