#include "FmodAudioSystem.h"

#include "Application.h"
#include "Logger/Logger.h"

bool FmodAudioSystem::Initialize()
{
    FMOD_RESULT result;

    result = FMOD::Studio::System::create(&myStudioSystem);

    if (result != FMOD_OK)
    {
        /*MVLOG(Error, "Failed to create FMOD Studio system: {}",
            FMOD_ErrorString(result));*/
        MVLOG(Error, "Failed to create FMOD Studio system");

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

        MVLOG(Error, "Failed to initialize FMOD Studio");

        myStudioSystem->release();
        myStudioSystem = nullptr;

        return false;
    }

    MVLOG(Log, "FMOD initialized successfully.");

    return true;
}

void FmodAudioSystem::Update()
{
    if (myStudioSystem)
    {
        myStudioSystem->update();
    }
}

void FmodAudioSystem::Shutdown()
{
    if (!myStudioSystem)
    {
        return;
    }

    myStudioSystem->release();
    myStudioSystem = nullptr;

    MVLOG(Log, "FMOD shut down.");
}

FmodAudioSystem::~FmodAudioSystem()
{
    Shutdown();
}