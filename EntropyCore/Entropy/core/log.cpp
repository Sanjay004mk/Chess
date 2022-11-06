#include <etpch.h>
#include <spdlog\sinks\stdout_color_sinks.h>
#include <spdlog\sinks\basic_file_sink.h>

#include "log.h"

namespace et
{
	std::shared_ptr<spdlog::logger> Log::sLogger;

	void Log::Init()
	{
		std::vector<spdlog::sink_ptr> logSinks;

		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Entropy.log", true));

		logSinks[0]->set_pattern("%^[%T] %n: %v%$");
		logSinks[0]->set_level(spdlog::level::info);
		logSinks[1]->set_pattern("[%T] [%l] %n: %v");

		sLogger = std::make_shared<spdlog::logger>("ENTROPY", logSinks.begin(), logSinks.end());
		spdlog::register_logger(sLogger);
		sLogger->set_level(spdlog::level::trace);
		sLogger->flush_on(spdlog::level::trace);
	}
}