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

	private:
		static std::shared_ptr<spdlog::logger> s_AgentSystemLogger;
		static std::shared_ptr<spdlog::logger> s_CommLayerLogger;
		static std::shared_ptr<spdlog::logger> s_TestAppLogger;
		static int initialized;
	};

}

