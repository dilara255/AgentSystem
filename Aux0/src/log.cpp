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
	std::shared_ptr<spdlog::logger> Log::s_DebugAuxLogger;
	int Log::initialized = 0;

	uint16_t RGB24toRGB565(uint8_t r, uint8_t g, uint8_t b) {
		return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
	}

	void log(std::shared_ptr<spdlog::logger> logger, const int degree, const char* file, 
		const int line, const char* message, uint32_t trailingNewlines, bool supress) {
		
		if(!supress || (degree != L_TRACE) ) { 
			Log::log(logger, degree, file, line, message, trailingNewlines); }
	}

	void Log::init() {

		if (!Log::initialized) {

			spdlog::set_pattern("%^\t[%T] %n: \t%v%$");

			s_AgentSystemLogger = spdlog::stdout_color_mt("AS");
			s_AgentSystemLogger->set_level(spdlog::level::trace);		
			auto color_sink = 
				static_cast<spdlog::sinks::stdout_color_sink_mt*>(s_AgentSystemLogger->sinks()[0].get());
			color_sink->set_color(spdlog::level::trace, AZ_LOG_TRACE_COLOR);
			color_sink->set_color(spdlog::level::debug, AZ_LOG_DEBUG_COLOR);
			color_sink->set_color(spdlog::level::info, AZ_LOG_INFO_COLOR);	

			s_CommLayerLogger = spdlog::stdout_color_mt("CL");
			s_CommLayerLogger->set_level(spdlog::level::trace);
			color_sink =
				static_cast<spdlog::sinks::stdout_color_sink_mt*>(s_CommLayerLogger->sinks()[0].get());
			color_sink->set_color(spdlog::level::trace, AZ_LOG_TRACE_COLOR);
			color_sink->set_color(spdlog::level::debug, AZ_LOG_DEBUG_COLOR);
			color_sink->set_color(spdlog::level::info, AZ_LOG_INFO_COLOR);

			s_TestAppLogger = spdlog::stdout_color_mt("TA");
			s_TestAppLogger->set_level(spdlog::level::trace);
			color_sink =
				static_cast<spdlog::sinks::stdout_color_sink_mt*>(s_TestAppLogger->sinks()[0].get());
			color_sink->set_color(spdlog::level::trace, AZ_LOG_TRACE_COLOR);
			color_sink->set_color(spdlog::level::debug, AZ_LOG_DEBUG_COLOR);
			color_sink->set_color(spdlog::level::info, AZ_LOG_INFO_COLOR);
			
			s_DebugAuxLogger = spdlog::stdout_color_mt("DA");
			s_DebugAuxLogger->set_level(spdlog::level::trace);			
			s_DebugAuxLogger->set_pattern("%^%v%$");

			Log::initialized = true;
		}

		if (!initialized)
			std::cerr << "\n\nERROR: LOGGERS COULDN'T BE INITIALIZED\n\n";
	}

	void Log::log(std::shared_ptr<spdlog::logger> logger, const int degree, const char* file,
		          const int line, const char* message, uint32_t trailingNewlines) {
		
		for (uint32_t i = 0; i < trailingNewlines; i++) {
			puts("\n");
		}

		#if (defined AS_DEBUG) || VERBOSE_RELEASE
			std::string lineInfo = std::string(file) + " @ line " + std::to_string(line) + ":";
			s_DebugAuxLogger->debug(lineInfo.c_str());
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
		case L_DEBUG:
			logger->debug(message);
			break;
		default:
		{
			logger->critical("Error on logging degree: will show as critical:");
			logger->critical(message);
		}
		}
	}
}

