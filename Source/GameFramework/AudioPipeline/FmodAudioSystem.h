#pragma once
#include <Fmod/fmod_studio.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <CommonUtilities/Math/Vector.hpp>

struct Audio3DState
{
    CommonUtilities::Vector3<float> Position;
    CommonUtilities::Vector3<float> Forward;
    CommonUtilities::Vector3<float> Up;
    CommonUtilities::Vector3<float> Velocity;
};

class FmodAudioSystem
{
public:
    using AudioHandle = uint32_t;

    static FmodAudioSystem& Get();

    FmodAudioSystem(const FmodAudioSystem&) = delete;
    FmodAudioSystem& operator=(const FmodAudioSystem&) = delete;
    

    bool Initialize();
    void Update();
    void Shutdown();
    bool IsInitialized() const;

    bool LoadBank(const std::string& aPath);
    //Dont unload banks in use
    bool UnloadBank(const std::string& aPath);

    bool PlayEvent(const std::string& aEventPath);
    AudioHandle PlayEvent3D(const std::string& aEventPath, const CommonUtilities::Vector3<float>& aPos);
    //TODO: allow for ALLOWFADEOUT and IMMIDIATE?
    void Stop(AudioHandle aHandle);
    void Pause(AudioHandle aHandle, bool aPause);
    void SetVolume(AudioHandle aHandle, float aVolume);
    void SetParameter(AudioHandle aHandle, const std::string& aParameter, float aValue);
    void Set3DAttributes(AudioHandle aHandle, const Audio3DState& aState);
    bool IsPlaying(AudioHandle aHandle) const;

    void SetMasterVolume(float aVolume);
    void SetMusicVolume(float aVolume);
    void SetSFXVolume(float aVolume);

    float GetMasterVolume() const;
    float GetMusicVolume() const;
    float GetSFXVolume() const;

    void SetMasterMuted(bool aMuted);
    void SetMusicMuted(bool aMuted);
    void SetSFXMuted(bool aMuted);

    bool GetMasterMuted() const;
    bool GetMusicMuted() const;
    bool GetSFXMuted() const;

    void SetListener(const Audio3DState& aState);

private:

    FmodAudioSystem() = default;
    ~FmodAudioSystem() = default;

    void CleanupFinishedInstances();

    FMOD::Studio::EventInstance* GetInstance(AudioHandle aHandle);
    const FMOD::Studio::EventInstance* GetInstance(AudioHandle aHandle) const;

    FMOD::Studio::System* myStudioSystem = nullptr;

    FMOD::Studio::Bus* myMasterBus = nullptr;
    FMOD::Studio::Bus* myMusicBus = nullptr;
    FMOD::Studio::Bus* mySFXBus = nullptr;

    std::unordered_map<AudioHandle, FMOD::Studio::EventInstance*> myAudioInstances;
    std::unordered_map<std::string, FMOD::Studio::Bank*> myLoadedBanks;

    AudioHandle myNextAudioHandle = 1;
};

//TODO: add in the future
//StopAll function
//SetGlobalParameters
//SetPitch
//IsPaused
//GetPlaybackState
//A unordered map of listeners? (do we need more than one?)