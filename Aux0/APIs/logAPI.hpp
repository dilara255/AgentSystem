#pragma once

/*
Define macros de API para facilitar log em console. Usa spdlog.
No momento não dá pra passar variáveis nem gravar em arquivo, 
apesar da biblioteca suportar isso.

TODO: Generalize the whole macro thingy and all that's associated with it
*/

#include "core.hpp"

//para os outros projetos poderem linkar as funções, declaradas em log.hpp
#ifndef AUX0
    #include "../include/log.hpp"
#endif

//The default for the following definitions is 0

//All LOG_TRACE logs will be supressed if this is set to anything but 0
#define SUPRESS_TRACES 0

//Setting this to anything but 0 makes release supress only LOG_TRACEs
#define RELEASE_SUPRESS_ONLY_TRACES 0

//Setting this to anything but 0 makes release as verbose as debug
#define VERBOSE_RELEASE 0

//Setting this to anything but 0 makes debug not ask for keypresses on GETCHAR_PAUSE
#define DONT_ASK_KEYPRESS_DEBUG 0

//Setting this to anything but 0 makes release ASK for keypresses on GETCHAR_PAUSE
#define ASK_KEYPRESS_ON_RELEASE 0

//Setting this to anything but 0 makes GETCHAR_FORCE_PAUSE act the same as GETCHAR_PAUSE
#define DONT_FORCE_KEYPRESS 0

//TODO: way to define level for test header

//NOTE: changing these may lead to full recompile : (

namespace az {
    //Usage: LOG_*LEVEL*("message", int extraNewlinesAfter = 0) logs:
    //- Depending on level, file path and source line;
    //- Message, color coded by level and with time and project stamps; and
    //- Adds extraNewlinesAfter the message.
    void log(std::shared_ptr<spdlog::logger> logger, const int degree, const char* file, 
             const int line, const char* message, uint32_t trailingNewlines = 0, 
                                                           bool supress = false);

    uint16_t RGB24toRGB565(uint8_t r, uint8_t g, uint8_t b);
}

namespace az {
    
}

#define L_TRACE 0
#define L_DEBUG 1
#define L_INFO 2
#define L_WARN 3
#define L_ERROR 4
#define L_CRITICAL 5

//Only macros from here on : )


//MACROS for user interaction

//TODO: BUG: FIX: this is using getchar() withouth taking care to exausting the buffer!
#if (defined AS_DEBUG && !DONT_ASK_KEYPRESS_DEBUG) || (defined AS_RELEASE && ASK_KEYPRESS_ON_RELEASE)
	#define GETCHAR_PAUSE getchar()
#else
	#define GETCHAR_PAUSE puts("\n")
#endif // AS_DEBUG

#if (DONT_FORCE_KEYPRESS)
    #define GETCHAR_FORCE_PAUSE GETCHAR_PAUSE;
#else
	#define GETCHAR_FORCE_PAUSE getchar()
#endif // AS_DEBUG

//MACROS log Agent System

#ifdef AS_AGENTSYSTEM
    #define GETLOGGER az::Log::GetASLogger()
#endif

#ifdef AS_COMMLAYER
    #define GETLOGGER az::Log::GetCLLogger()
#endif

#ifdef AS_TESTAPP
#define GETLOGGER az::Log::GetTALogger()
#endif

//Usage: LOG_CRITICAL("message", int extraNewlinesAfter = 0)
#define LOG_CRITICAL(...) az::log(GETLOGGER, L_CRITICAL,\
                                   __FILE__, __LINE__, __VA_ARGS__)

#ifdef AS_DISTRO
    #define LOG_ERROR(...)
#else
    //Usage: LOG_ERROR("message", int extraNewlinesAfter = 0)
    #define LOG_ERROR(...) az::log(GETLOGGER, L_ERROR,\
                                   __FILE__, __LINE__, __VA_ARGS__)
#endif

#if !VERBOSE_RELEASE && defined AS_RELEASE
    #define LOG_WARN(...)
#else
    //Usage: LOG_WARN("message", int extraNewlinesAfter = 0)
    #define LOG_WARN(...) az::log(GETLOGGER, L_WARN,\
                                  __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef AS_DISTRO
    #define LOG_INFO(...)
#else
    //Usage: LOG_INFO("message", int extraNewlinesAfter = 0)
    #define LOG_INFO(...) az::log(GETLOGGER, L_INFO,\
                                  __FILE__, __LINE__, __VA_ARGS__)
#endif

#if (!VERBOSE_RELEASE && defined AS_RELEASE) || SUPRESS_TRACES
    #define LOG_TRACE(...)
#else
    //Usage: LOG_TRACE("message", int extraNewlinesAfter = 0)
    #define LOG_TRACE(...) az::log(GETLOGGER, L_TRACE,\
                                   __FILE__, __LINE__, __VA_ARGS__)
#endif

#if !(RELEASE_SUPRESS_ONLY_TRACES || VERBOSE_RELEASE) && defined AS_RELEASE
    #define LOG_DEBUG(...)
#else
    //Usage: LOG_TRACE("message", int extraNewlinesAfter = 0)
    #define LOG_DEBUG(...) az::log(GETLOGGER, L_DEBUG,\
                                   __FILE__, __LINE__, __VA_ARGS__)
#endif

