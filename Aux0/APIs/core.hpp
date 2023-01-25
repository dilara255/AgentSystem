#pragma once

//definições para controle dos projetos, sistemas, compilação, linkagem, etc
//TODO: This could all be on the build system. Is that preferable? 
//Either way, go all the way EITHER there OR here

#ifndef AUX0
	#include "logAPI.hpp"
#endif

#define BIT(x) (1 << x)

#ifdef AS_PLATFORM_WINDOWS
	//#define SYSTEM_NAME "Windows" //Moved to the build system
	#ifdef AS_COMMLAYER
		#define	CL_API __declspec(dllexport)
	#else
		#define	CL_API __declspec(dllimport)
	#endif
	#ifdef AS_AGENTSYSTEM
		#define	AS_API __declspec(dllexport)
	#else
		#define	AS_API __declspec(dllimport)
	#endif

	#define AZ_LOG_TRACE_COLOR (FOREGROUND_BLUE | FOREGROUND_GREEN)
	#define AZ_LOG_INFO_COLOR (FOREGROUND_GREEN | FOREGROUND_INTENSITY)

#else
	#error no support for other platforms as of yet : /
#endif

/* These have all been moved to the build system

#ifdef AS_DEBUG
	#define CONFIG_NAME "Debug"
#elif RELEASE 
	#define CONFIG_NAME "Release"
#endif

#ifdef X64
	#define PLATFORM_NAME "64 bits"
#elif X86
	#define PLATFORM_NAME "32 bits"
#endif

*/




