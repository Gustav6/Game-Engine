#pragma once

#include <memory>
#include <filesystem>

#include "CommonUtilities/Input/Input.h"
#include "CommonUtilities/Time/Timer.h"

#include "WindowManager.h"

#include "Logger/Logger.h"
#include "Ensure.h"

#if _DEBUG
DECLARE_LOG_CATEGORY_WITH_NAME(LogModelViewer, ModelViewer, Verbose);
#else
DECLARE_LOG_CATEGORY_WITH_NAME(LogModelViewer, ModelViewer, Warning);
#endif

#define MVLOG(Verbosity, Message, ...) LOG(LogModelViewer, Verbosity, Message, ##__VA_ARGS__)

struct ApplicationData
{
	CommonUtilities::Input InputManager;
	CommonUtilities::Timer TimeManager;
	WindowManager WindowManager;
};

inline ApplicationData Application;

/**
 * For more info -
 * https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew
 * @return
 */
std::filesystem::path GetExecutableDirectory();