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

namespace az {
    void log(const char* message, std::shared_ptr<spdlog::logger> logger
        , const int degree, const char* file, const int line);
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

#ifdef AS_DISTRO
    #define LOG_CRITICAL(...)
#else
    #define LOG_CRITICAL(...) az::log(__VA_ARGS__, GETLOGGER, L_CRITICAL,\
                                      __FILE__, __LINE__)
#endif

#ifdef AS_DISTRO
    #define LOG_ERROR(...)
#else
    #define LOG_ERROR(...) az::log(__VA_ARGS__, GETLOGGER, L_ERROR,\
                                   __FILE__, __LINE__)
#endif

#ifdef AS_RELEASE
    #define LOG_WARN(...)
#else
    #define LOG_WARN(...) az::log(__VA_ARGS__, GETLOGGER, L_WARN,\
                                  __FILE__, __LINE__)
#endif

#ifdef AS_DISTRO
    #define LOG_INFO(...)
#else
    #define LOG_INFO(...) az::log(__VA_ARGS__, GETLOGGER, L_INFO,\
                                  __FILE__, __LINE__)
#endif

#ifdef AS_RELEASE
    #define LOG_TRACE(...)
#else
    #define LOG_TRACE(...) az::log(__VA_ARGS__, GETLOGGER, L_TRACE,\
                                   __FILE__, __LINE__)
#endif