#include "GameFramework.pch.h"
#include "DebugSettingsManager.h"

DebugSettingsManager& DebugSettingsManager::Get()
{
    static DebugSettingsManager instance;
    return instance;
}

void DebugSettingsManager::SetSettings(const DebugSettings& aSettings)
{
    mySettings = aSettings;
}

const DebugSettings& DebugSettingsManager::GetSettings() const
{
    return mySettings;
}
