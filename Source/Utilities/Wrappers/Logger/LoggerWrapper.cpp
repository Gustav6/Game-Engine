#include "LoggerWrapper.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <filesystem>
#include <string>

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/null_sink.h>

void Logger::Initialize()
{
    if (spdlog::get("application") != nullptr)
    {
        return;
    }

#ifdef _RETAIL

    auto logger = std::make_shared<spdlog::logger>(
        "application",
        std::make_shared<spdlog::sinks::null_sink_mt>());

    logger->set_level(spdlog::level::off);

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    return;

#else

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

    auto aplicationLogs = std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "application.log").string(), true);
    auto diagnosticsLogs = std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "diagnostics.log").string(), true);

    aplicationLogs->set_level(spdlog::level::trace);
    diagnosticsLogs->set_level(spdlog::level::warn);

    auto logger = std::make_shared<spdlog::async_logger>(
        "application",
        spdlog::sinks_init_list{ console, aplicationLogs, diagnosticsLogs, debugger },
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);

    logger->set_level(spdlog::level::trace);
    logger->set_pattern("[%H:%M:%S] [%^%l%$] %v");

    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::err); 

#endif
}

void Logger::Shutdown()
{
    spdlog::drop_all();
    spdlog::shutdown();
}
