#include "FmodAudioSystem.h"
#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(AudioLog, AUDIO, Verbose);

DEFINE_LOG_CATEGORY(AudioLog);

FmodAudioSystem& FmodAudioSystem::Get()
{
    static FmodAudioSystem instance;
    return instance;
}

bool FmodAudioSystem::Initialize()
{
    FMOD_RESULT result;

    result = FMOD::Studio::System::create(&myStudioSystem);

    if (result != FMOD_OK)
    {
        /*MVLOG(Error, "Failed to create FMOD Studio system: {}",
            FMOD_ErrorString(result));*/
        LOG(AudioLog, Error, "Failed to create FMOD Studio system");

        myStudioSystem = nullptr;
        return false;
    }

    result = myStudioSystem->initialize(
        1024,
        FMOD_STUDIO_INIT_NORMAL,
        FMOD_INIT_NORMAL,
        nullptr
    );

    if (result != FMOD_OK)
    {
        /*MVLOG(Error, "Failed to initialize FMOD Studio: {}",
            FMOD_ErrorString(result));*/

        LOG(AudioLog, Error, "Failed to initialize FMOD Studio");

        myStudioSystem->release();
        myStudioSystem = nullptr;

        return false;
    }

    LOG(AudioLog, Log, "FMOD initialized successfully.");

    return true;
}

void FmodAudioSystem::Update()
{
    if (myStudioSystem)
    {
        myStudioSystem->update();
    }
}

bool FmodAudioSystem::LoadBank(const std::string& aPath)
{
    if (!myStudioSystem)
    {
        return false;
    }

    FMOD::Studio::Bank* bank = nullptr;
    FMOD_RESULT result = myStudioSystem->loadBankFile(aPath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);

    if (result != FMOD_OK)
    {
        LOG(AudioLog, Error, "Failed to load bank {}", aPath);
        return false;
    }

    LOG(AudioLog, Log, "Loaded bank {}", aPath);

    return true;
}

bool FmodAudioSystem::PlayEvent(const std::string& aEventPath)
{
    if (!myStudioSystem)
    {
        return false;
    }

    FMOD::Studio::EventDescription* description = nullptr;
    FMOD::Studio::EventInstance* instance = nullptr;

    FMOD_RESULT result = myStudioSystem->getEvent(aEventPath.c_str(), &description);

    if (result != FMOD_OK)
    {
        LOG(AudioLog, Error, "Fmod could not find event {}", aEventPath);
        return false;
    }

    result = description->createInstance(&instance);

    if (result != FMOD_OK)
    {
        LOG(AudioLog, Error, "Fmof could not create event instance {}", aEventPath);
        return false;
    }

    result = instance->start();

    if (result != FMOD_OK)
    {
        LOG(AudioLog, Error, "Fmod could not start event {}", aEventPath);
        instance->release();
        return false;
    }

    instance->release();

    LOG(AudioLog, Log, "Fmod played event {}", aEventPath);

    return true;
}

void FmodAudioSystem::Shutdown()
{
    if (!myStudioSystem)
    {
        return;
    }

    myStudioSystem->release();
    myStudioSystem = nullptr;

    LOG(AudioLog, Error, "FMOD shut down.");
}

FmodAudioSystem::~FmodAudioSystem()
{
    Shutdown();
}