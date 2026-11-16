#pragma once

#include <filesystem>
#include <memory>

#include "CommonUtilities/Input/Input.h"
#include "CommonUtilities/Time/Timer.h"
#include  "Core/Scene/World/GameCamera.h"

struct EngineContext
{
	CommonUtilities::Input InputManager;
	CommonUtilities::Timer TimeManager;
	std::filesystem::path ContentRoot;
	std::filesystem::path ShaderRoot;
	std::unique_ptr<GameCamera> GameCamera = nullptr;
};

inline EngineContext GEngineContext;

std::filesystem::path GetExecutableDirectory();