#pragma once
#include <Fmod/fmod_studio.hpp>
#include <CommonUtilities/Math/Vector.hpp>


enum SoundEffectID
{

};

enum MusicID
{

};

enum BusID
{
    Master,
    SFX,
    Music
};

class FmodAudioSystem
{
public:
    static FmodAudioSystem& Get();

    FmodAudioSystem(const FmodAudioSystem&) = delete;
    FmodAudioSystem& operator=(const FmodAudioSystem&) = delete;
    
    ~FmodAudioSystem();

    bool Initialize();
    void Update();
    bool LoadBank(const std::string& aPath);
    bool PlayEvent(const std::string& eventPath);
    void StopEvent();
    void SetListener();



    void PlaySoundEffect(SoundEffectID aSoundID);
    void PlaySoundEffectEvent(SoundEffectID aSoundID);
    void PlaySoundEffectEventAtPosition(SoundEffectID aSoundID, CommonUtilities::Vector2<float> aPos);
    void StopSoundEffect(SoundEffectID aSoundEffect);

    void PlayMusic(MusicID aMusicID, bool aInterrupt = true);
    
    
    


    void Shutdown();

    FmodAudioSystem() = default;

private:


    FMOD::Studio::System* myStudioSystem = nullptr;
};