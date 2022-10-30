#pragma once

//definições para controle dos projetos, sistemas, compilação, linkagem, etc
//TODO: Migrar parte disso pro build system?

#ifdef AS_PLATFORM_WINDOWS
	#define SYSTEM_NAME "Windows"
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

#define BIT(x) (1 << x)


