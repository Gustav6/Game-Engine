#pragma once

#include <filesystem>

#include "CommonUtilities/Input/Input.h"
#include "CommonUtilities/Time/Timer.h"

struct ApplicationData
{
	CommonUtilities::Input InputManager;
	CommonUtilities::Timer TimeManager;
	std::filesystem::path ContentPath;
};

inline ApplicationData Application;

/**
 * For more info -
 * https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew
 * @return
 */
std::filesystem::path GetExecutableDirectory();