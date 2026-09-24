#include "FmodAudioSystem.h"
#include "Logger/Logger.h"
#include <algorithm>

DECLARE_LOG_CATEGORY_WITH_NAME(AudioLog, AUDIO, Verbose);

DEFINE_LOG_CATEGORY(AudioLog);

FmodAudioSystem& FmodAudioSystem::Get()
{
	static FmodAudioSystem instance;
	return instance;
}

static FMOD_VECTOR ToFmodVector(const CommonUtilities::Vector3<float>& aVector)
{
	FMOD_VECTOR result{};

	result.x = aVector.x;
	result.y = aVector.y;
	result.z = aVector.z;

	return result;
}

bool FmodAudioSystem::Initialize()
{
	if (myStudioSystem)
	{
		return true;
	}

	FMOD_RESULT result;

	result = FMOD::Studio::System::create(&myStudioSystem);

	if (result != FMOD_OK)
	{
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
		LOG(AudioLog, Error, "Failed to initialize FMOD Studio");

		myStudioSystem->release();
		myStudioSystem = nullptr;

		return false;
	}

	result = myStudioSystem->getBus("bus:/Master", &myMasterBus);
	if (result != FMOD_OK)
	{
		myMasterBus = nullptr;
	}
	result = myStudioSystem->getBus("bus:/Music", &myMusicBus);
	if (result != FMOD_OK)
	{
		myMusicBus = nullptr;
	}
	result = myStudioSystem->getBus("bus:/SFX", &mySFXBus);
	if (result != FMOD_OK)
	{
		mySFXBus = nullptr;
	}

	LOG(AudioLog, Log, "FMOD initialized successfully.");

	return true;
}

void FmodAudioSystem::Update()
{
	if (!myStudioSystem)
	{
		return;
	}

	myStudioSystem->update();

	CleanupFinishedInstances();
}

bool FmodAudioSystem::LoadBank(const std::string& aPath)
{
	if (!myStudioSystem)
	{
		return false;
	}

	if (myLoadedBanks.contains(aPath))
	{
		LOG(AudioLog, Log, "{} bank is already loaded", aPath);
		return true;
	}

	FMOD::Studio::Bank* bank = nullptr;
	FMOD_RESULT result = myStudioSystem->loadBankFile(aPath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "Failed to load bank {}", aPath);
		return false;
	}

	myLoadedBanks.emplace(aPath, bank);

	LOG(AudioLog, Log, "Loaded bank {}", aPath);

	return true;
}

bool FmodAudioSystem::UnloadBank(const std::string& aPath)
{
	auto it = myLoadedBanks.find(aPath);

	if (it == myLoadedBanks.end())
	{
		LOG(AudioLog, Warning, "Tried to unload FMOD bank that is not loaded: {}", aPath);
		return false;
	}

	FMOD::Studio::Bank* bank = it->second;

	if (!bank)
	{
		myLoadedBanks.erase(it);
		return false;
	}

	FMOD_RESULT result = bank->unload();

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "Failed to unloadFMOD bank: {}", aPath);
		return false;
	}

	myLoadedBanks.erase(it);

	LOG(AudioLog, Log, "Unloaded FMOD bank: {}", aPath);

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

FmodAudioSystem::AudioHandle FmodAudioSystem::PlayEvent3D(const std::string& aEventPath, const CommonUtilities::Vector3<float>& aPos)
{
	if (!myStudioSystem)
	{
		return 0;
	}

	FMOD::Studio::EventDescription* description = nullptr;
	FMOD::Studio::EventInstance* instance = nullptr;

	FMOD_RESULT result = myStudioSystem->getEvent(aEventPath.c_str(), &description);

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "FMOD could not find 3D event {}", aEventPath);
		return 0;
	}

	result = description->createInstance(&instance);

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "FMOD could not create event instance {}", aEventPath);
		return 0;
	}

	FMOD_3D_ATTRIBUTES attributes{};
	attributes.position = ToFmodVector(aPos);

	attributes.forward = { .0f, .0f, 1.f };

	attributes.up = { .0f, 1.f, .0f };

	attributes.velocity = { .0f, .0f, .0f };

	result = instance->set3DAttributes(&attributes);

	float minDist = .0f;
	float maxDist = .0f;

	result = instance->getMinMaxDistance(&minDist, &maxDist);

	if (result == FMOD_OK)
	{
		LOG(AudioLog, Log, "3D Event min dist: {}, max dist: {}", minDist, maxDist);
	}

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "FMOD could not set 3D attributes for {}", aEventPath);
		instance->release();
		return 0;
	}

	result = instance->start();

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "FMOD could not start 3D event {}", aEventPath);
		instance->release();
		return 0;
	}

	AudioHandle handle = myNextAudioHandle++;

	if (handle == 0)
	{
		handle = myNextAudioHandle++;
	}

	myAudioInstances.emplace(handle, instance);

	LOG(AudioLog, Log, "FMOD started 3D event {} witch handle {}", aEventPath, handle);

	return handle;
}

void FmodAudioSystem::Stop(AudioHandle aHandle)
{
	FMOD::Studio::EventInstance* instance = GetInstance(aHandle);

	if (!instance)
	{
		return;
	}

	instance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);

	instance->release();

	myAudioInstances.erase(aHandle);
}

void FmodAudioSystem::Pause(AudioHandle aHandle, bool aPause)
{
	FMOD::Studio::EventInstance* instance = GetInstance(aHandle);

	if (!instance)
	{
		return;
	}

	instance->setPaused(aPause);
}

void FmodAudioSystem::SetVolume(AudioHandle aHandle, float aVolume)
{
	FMOD::Studio::EventInstance* instance = GetInstance(aHandle);

	if (!instance)
	{
		return;
	}

	aVolume = std::clamp(aVolume, .0f, 1.f);

	instance->setVolume(aVolume);
}

void FmodAudioSystem::SetParameter(AudioHandle aHandle, const std::string& aParameter, float aValue)
{
	FMOD::Studio::EventInstance* instance = GetInstance(aHandle);

	if (!instance)
	{
		return;
	}

	instance->setParameterByName(aParameter.c_str(), aValue);
}

void FmodAudioSystem::Set3DAttributes(AudioHandle aHandle, const Audio3DState& aState)
{
	FMOD::Studio::EventInstance* instance = GetInstance(aHandle);

	if (!instance)
	{
		return;
	}

	FMOD_3D_ATTRIBUTES attributes{};

	attributes.position  = ToFmodVector(aState.Position);
	attributes.forward   = ToFmodVector(aState.Forward);
	attributes.up        = ToFmodVector(aState.Up);
	attributes.velocity  = ToFmodVector(aState.Velocity);

	FMOD_RESULT result = instance->set3DAttributes(&attributes);

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "FMOD failed to set 3D attributes for handle {}", aHandle);
	}
}

bool FmodAudioSystem::IsPlaying(AudioHandle aHandle) const
{
	const FMOD::Studio::EventInstance* instance = GetInstance(aHandle);

	if (!instance)
	{
		return false;
	}

	FMOD_STUDIO_PLAYBACK_STATE state;

	if (instance->getPlaybackState(&state) != FMOD_OK)
	{
		return false;
	}

	return state == FMOD_STUDIO_PLAYBACK_PLAYING;
}

void FmodAudioSystem::SetMasterVolume(float aVolume)
{
	if (!myMasterBus)
	{
		return;
	}

	aVolume = std::clamp(aVolume, .0f, 1.f);

	myMasterBus->setVolume(aVolume);
}

void FmodAudioSystem::SetMusicVolume(float aVolume)
{
	if (!myMusicBus)
	{
		return;
	}

	aVolume = std::clamp(aVolume, .0f, 1.f);

	myMusicBus->setVolume(aVolume);
}

void FmodAudioSystem::SetSFXVolume(float aVolume)
{
	if (!mySFXBus)
	{
		return;
	}

	aVolume = std::clamp(aVolume, .0f, 1.f);

	mySFXBus->setVolume(aVolume);
}

float FmodAudioSystem::GetMasterVolume() const
{
	if (!myMasterBus)
	{
		return .0f;
	}

	float volume = .0f;

	myMasterBus->getVolume(&volume, nullptr);

	return volume;
}

float FmodAudioSystem::GetMusicVolume() const
{
	if (!myMusicBus)
	{
		return .0f;
	}

	float volume = .0f;

	myMusicBus->getVolume(&volume, nullptr);

	return volume;
}

float FmodAudioSystem::GetSFXVolume() const
{
	if (!mySFXBus)
	{
		return .0f;
	}

	float volume = .0f;

	mySFXBus->getVolume(&volume, nullptr);

	return volume;
}

void FmodAudioSystem::SetMasterMuted(bool aMuted)
{
	if (myMasterBus)
	{
		myMasterBus->setMute(aMuted);
	}
}

void FmodAudioSystem::SetMusicMuted(bool aMuted)
{
	if (myMusicBus)
	{
		myMusicBus->setMute(aMuted);
	}
}

void FmodAudioSystem::SetSFXMuted(bool aMuted)
{
	if (mySFXBus)
	{
		mySFXBus->setMute(aMuted);
	}
}

bool FmodAudioSystem::GetMasterMuted() const
{
	if (!myMasterBus)
	{
		return false;
	}

	bool muted = false;

	myMasterBus->getMute(&muted);

	return muted;
}

bool FmodAudioSystem::GetMusicMuted() const
{
	if (!myMusicBus)
	{
		return false;
	}

	bool muted = false;

	myMusicBus->getMute(&muted);

	return muted;
}

bool FmodAudioSystem::GetSFXMuted() const
{
	if (!mySFXBus)
	{
		return false;
	}

	bool muted = false;

	mySFXBus->getMute(&muted);

	return muted;
}

void FmodAudioSystem::Shutdown()
{
	if (!myStudioSystem)
	{
		return;
	}

	for (auto& [handle, instance] : myAudioInstances)
	{
		if (instance)
		{
			instance->stop(FMOD_STUDIO_STOP_IMMEDIATE);
			instance->release();
		}
	}

	myAudioInstances.clear();

	for (auto& [path, bank] : myLoadedBanks)
	{
		if (bank)
		{
			bank->unload();
		}
	}

	myMasterBus = nullptr;
	myMusicBus = nullptr;
	mySFXBus = nullptr;

	myStudioSystem->release();
	myStudioSystem = nullptr;

	LOG(AudioLog, Log, "FMOD shut down");
}

bool FmodAudioSystem::IsInitialized() const
{
	return myStudioSystem != nullptr;
}

void FmodAudioSystem::SetListener(const Audio3DState& aState)
{
	if (!myStudioSystem)
	{
		return;
	}

	FMOD_3D_ATTRIBUTES attributes{};

	attributes.position  = ToFmodVector(aState.Position);
	attributes.forward   = ToFmodVector(aState.Forward);
	attributes.up		 = ToFmodVector(aState.Up);
	attributes.velocity  = ToFmodVector(aState.Velocity);

	FMOD_RESULT result = myStudioSystem->setListenerAttributes(0, &attributes);

	if (result != FMOD_OK)
	{
		LOG(AudioLog, Error, "FMOD failed to set listner attributes");
		return;
	}
}

void FmodAudioSystem::CleanupFinishedInstances()
{
	for (auto it = myAudioInstances.begin(); it != myAudioInstances.end();)
	{
		FMOD::Studio::EventInstance* instance = it->second;

		FMOD_STUDIO_PLAYBACK_STATE state;

		FMOD_RESULT result = instance->getPlaybackState(&state);

		if (result != FMOD_OK || state == FMOD_STUDIO_PLAYBACK_STOPPED)
		{
			instance->release();
			it = myAudioInstances.erase(it);
		}
		else
		{
			++it;
		}
	}
}

FMOD::Studio::EventInstance* FmodAudioSystem::GetInstance(AudioHandle aHandle)
{
	auto it = myAudioInstances.find(aHandle);

	if (it == myAudioInstances.end())
	{
		return nullptr;
	}

	return it->second;
}

const FMOD::Studio::EventInstance* FmodAudioSystem::GetInstance(AudioHandle aHandle) const
{
	auto it = myAudioInstances.find(aHandle);

	if (it == myAudioInstances.end())
	{
		return nullptr;
	}

	return it->second;
}