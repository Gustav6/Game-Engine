#pragma once

// More info can be found at their repo:
// https://github.com/gabime/spdlog

// Guides:
// https://github.com/gabime/spdlog/wiki/#creating-loggers-with-multiple-sinks 

#ifdef _DEBUG
	#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#ifdef _RELEASE
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_WARN
#endif

#include <spdlog/spdlog.h>

// TODO: Add a prefix to log messages

namespace Logger
{
    void Initialize();
    void Shutdown();
}