#pragma once

//Declaração funções log. Usa Spdlog. Exposto via macros em logAPI.hpp

#include "miscStdHeaders.h"

#include "core.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace az {

	class Log
	{
	public:
		static void init();
		inline static std::shared_ptr<spdlog::logger>& GetASLogger() {
			if (!initialized) init();  return s_AgentSystemLogger;
		}
		inline static std::shared_ptr<spdlog::logger>& GetCLLogger() {
			if (!initialized) init(); return s_CommLayerLogger;
		}
		inline static std::shared_ptr<spdlog::logger>& GetTALogger() {
			if (!initialized) init(); return s_TestAppLogger;
		}
		inline static std::shared_ptr<spdlog::logger>& GetDALogger() {
			if (!initialized) init(); return s_DebugAuxLogger;
		}
		inline static void log(std::shared_ptr<spdlog::logger> logger, const int degree,
			                   const char* file, const int line, 
							   const char* message, unsigned trailingNewlines = 0);

	private:
		static std::shared_ptr<spdlog::logger> s_AgentSystemLogger;
		static std::shared_ptr<spdlog::logger> s_CommLayerLogger;
		static std::shared_ptr<spdlog::logger> s_TestAppLogger;
		static std::shared_ptr<spdlog::logger> s_DebugAuxLogger;
		static int initialized;
	};

}

