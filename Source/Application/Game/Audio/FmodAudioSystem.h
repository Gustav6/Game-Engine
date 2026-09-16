#pragma once

#include <Fmod/fmod_studio.hpp>

class FmodAudioSystem
{
public:
    FmodAudioSystem() = default;
    ~FmodAudioSystem();

    FmodAudioSystem(const FmodAudioSystem&) = delete;
    FmodAudioSystem& operator=(const FmodAudioSystem&) = delete;

    bool Initialize();
    void Update();
    void Shutdown();

private:
    FMOD::Studio::System* myStudioSystem = nullptr;
};