#pragma once

#include <memory>

#include "CommonUtilities/Input/Input.h"
#include "CommonUtilities/Time/Timer.h"

#include "WindowManager.h"
#include "Audio/FmodAudioSystem.h"

#include "Logger/Logger.h"
#include "Ensure.h"

#if _DEBUG
DECLARE_LOG_CATEGORY_WITH_NAME(LogModelViewer, ModelViewer, Verbose);
#else
DECLARE_LOG_CATEGORY_WITH_NAME(LogModelViewer, ModelViewer, Warning);
#endif

#define MVLOG(Verbosity, Message, ...) LOG(LogModelViewer, Verbosity, Message, ##__VA_ARGS__)

class FmodAudioSystem;

struct ApplicationData
{
	CommonUtilities::Input InputManager;
	CommonUtilities::Timer TimeManager;
	WindowManager WindowManager;

    std::unique_ptr<FmodAudioSystem> AudioSystem;
};

inline ApplicationData Application;

/**
 * For more info -
 * https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew
 * @return
 */
static std::filesystem::path GetExecutableDirectory()
{
    wchar_t buf[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, buf, MAX_PATH);

    // Ensure the fetching of the path did not fail!
    ensure(len != 0 && len != MAX_PATH);

    std::filesystem::path exePath(buf);

    return exePath.parent_path();
}