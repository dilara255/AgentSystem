#pragma once

/*
Define macros de API para facilitar log em console. Usa spdlog.
No momento não dá pra passar variáveis nem gravar em arquivo, 
apesar da biblioteca suportar isso.

TO DO: Generalize the whole macro thingy and all that's associated with it
*/

#include "core.hpp"

//para os outros projetos poderem linkar as funções, declaradas em log.hpp
#ifndef AUX0
    #include "../include/log.hpp"
#endif

namespace az {
    void log(std::shared_ptr<spdlog::logger> logger, const int degree, const char* file, 
             const int line, const char* message, unsigned trailingNewlines = 0);

    uint16_t RGB24toRGB565(uint8_t r, uint8_t g, uint8_t b);
}

namespace az {
    
}

#define L_TRACE 0
#define L_INFO 1
#define L_WARN 2
#define L_ERROR 3
#define L_CRITICAL 4

//Only macros from here on : )

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

#define LOG_CRITICAL(...) az::log(GETLOGGER, L_CRITICAL,\
                                   __FILE__, __LINE__, __VA_ARGS__)

#ifdef AS_DISTRO
    #define LOG_ERROR(...)
#else
    #define LOG_ERROR(...) az::log(GETLOGGER, L_ERROR,\
                                   __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef AS_RELEASE
    #define LOG_WARN(...)
#else
    #define LOG_WARN(...) az::log(GETLOGGER, L_WARN,\
                                  __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef AS_DISTRO
    #define LOG_INFO(...)
#else
    #define LOG_INFO(...) az::log(GETLOGGER, L_INFO,\
                                  __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef AS_RELEASE
    #define LOG_TRACE(...)
#else
    #define LOG_TRACE(...) az::log(GETLOGGER, L_TRACE,\
                                   __FILE__, __LINE__, __VA_ARGS__)
#endif