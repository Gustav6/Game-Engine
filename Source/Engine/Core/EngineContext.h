#pragma once

#include <filesystem>

#include "CommonUtilities/Input/Input.h"
#include "CommonUtilities/Time/Timer.h"

struct EngineContext
{
	CommonUtilities::Input InputManager;
	CommonUtilities::Timer TimeManager;
	std::filesystem::path ContentPath;
};

inline EngineContext EngineContext;

std::filesystem::path GetExecutableDirectory();