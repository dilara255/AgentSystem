#pragma once

//defini��es para controle dos projetos, sistemas, compila��o, linkagem, etc
//TODO: This could all be on the build system. Is that preferable? 
//Either way, go all the way EITHER there OR here

#ifndef AUX0
	#include "logAPI.hpp"
#endif

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
#else
	#error falta suporte pra linux ainda : /
#endif

/* These have all been moved to the build system

#ifdef DEBUG
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

#define BIT(x) (1 << x)


