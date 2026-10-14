#include "Logger.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <filesystem>
#include <string>

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>

void Logger::Init()
{
    if (spdlog::get("engine") != nullptr)
    {
        return;
    }

    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);

    if (len == 0 || len == MAX_PATH)
    {
        throw std::runtime_error("Logger: could not determine executable path");
    }

    std::filesystem::path exe(buf);

    const std::filesystem::path logDirectory = exe.parent_path() / "logs";
    std::filesystem::create_directories(logDirectory);

    spdlog::init_thread_pool(8192, 1);

    auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto debugger = std::make_shared<spdlog::sinks::msvc_sink_mt>();

    auto allFile = std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "all.log").string(), true);
    auto warnFile = std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "warnings.log").string(), true);
    auto errFile = std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "errors.log").string(), true);
    auto criticalFile = std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "critical.log").string(), true);

    allFile->set_level(spdlog::level::trace);
    warnFile->set_level(spdlog::level::warn);
    errFile->set_level(spdlog::level::err);
    criticalFile->set_level(spdlog::level::critical);

    auto logger = std::make_shared<spdlog::async_logger>(
        "engine",
        spdlog::sinks_init_list{ console, allFile, warnFile, errFile, criticalFile, debugger },
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    logger->set_level(spdlog::level::trace);
    logger->set_pattern("[%H:%M:%S] [%^%l%$] %v");

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::err); 
	
	spdlog::flush_every(std::chrono::seconds(3));
}

void Logger::Shutdown()
{
    spdlog::shutdown();
}
