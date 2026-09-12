#pragma once

#include "Logger/Logger.h"

#include "CommonUtilities/Input/Input.h"
#include "CommonUtilities/Time/Timer.h"

#include "AssetHandling/AssetRegistry.h"
#include "WindowManager.h"

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
	AssetRegistry AssetRegistry;
	WindowManager WindowManager;
};

inline ApplicationData Application;