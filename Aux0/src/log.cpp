/*
Loggers. Usa Spdlog. Exposto via macros em logAPI.hpp
*/

#include "miscStdHeaders.h"
#include "log.hpp"
#include "logAPI.hpp"

namespace az {

	std::shared_ptr<spdlog::logger> Log::s_AgentSystemLogger;
	std::shared_ptr<spdlog::logger> Log::s_CommLayerLogger;
	std::shared_ptr<spdlog::logger> Log::s_TestAppLogger;
	int Log::initialized = 0;

	void Log::init() {

		if (!Log::initialized) {

			spdlog::set_pattern("%^[%T] %n: %v%$");
			s_AgentSystemLogger = spdlog::stdout_color_mt("AGENT SYSTEM");
			s_AgentSystemLogger->set_level(spdlog::level::trace);

			s_CommLayerLogger = spdlog::stdout_color_mt("COMM LAYER");
			s_CommLayerLogger->set_level(spdlog::level::trace);

			s_TestAppLogger = spdlog::stdout_color_mt("TEST APP");
			s_TestAppLogger->set_level(spdlog::level::trace);

			Log::initialized = true;
		}

		if (!initialized)
			std::cerr << "\n\nERROR: LOGGERS COULDN'T BE INITIALIZED\n\n";
	}


	void log(const char* message, std::shared_ptr<spdlog::logger> logger
		       , const int degree, const char* file, const int line) {
		
		#ifdef AS_DEBUG
				std::cout << "\t\t(" << file << " @ line " << line << "->)" << std::endl;
		#endif

		switch (degree) {
		case L_TRACE:
			logger->trace(message);
			break;
		case L_INFO:
			logger->info(message);
			break;
		case L_WARN:
			logger->warn(message);
			break;
		case L_ERROR:
			logger->error(message);
			break;
		case L_CRITICAL:
			logger->critical(message);
			break;
		default:
		{
			logger->critical("Error on logging degree: will show as critical:");
			logger->critical(message);
		}
		}
	}
}

