// THis include
#include "SoundSystem.h"

// Local includes
#include "LogManager.h"

SoundSystem* SoundSystem::sm_pInstance = nullptr;

SoundSystem& SoundSystem::GetInstance()
{
	if (sm_pInstance == nullptr)
	{
		sm_pInstance = new SoundSystem();
	}
	return *sm_pInstance;
}

void SoundSystem::DestroyInstance()
{
	if (sm_pInstance != nullptr)
	{
		delete sm_pInstance;
		sm_pInstance = nullptr;
	}
}

SoundSystem::SoundSystem()
	: m_pFMODSystem(nullptr)
{
}

SoundSystem::~SoundSystem()
{
	if (m_pFMODSystem)
	{
		Shutdown();
	}
}

bool SoundSystem::Initialise(int maxChannels, FMOD_INITFLAGS flags, void* extraDriverData)
{
	FMOD_RESULT result = FMOD::System_Create(&m_pFMODSystem);
	if (result != FMOD_OK)
	{
		LogManager::GetInstance().Log("Sound System FMOD::System_Create has failed!");
		m_pFMODSystem = nullptr;
		return false;
	}

	result = m_pFMODSystem->init(maxChannels, flags, extraDriverData);
	if (result != FMOD_OK)
	{
		LogManager::GetInstance().Log("SoundSystem: FMOD::System::init failed!");
		if (m_pFMODSystem)
		{
			m_pFMODSystem->release(); // Clean up the created system object
			m_pFMODSystem = nullptr;
		}
		return false;
	}
	LogManager::GetInstance().Log("SoundSystem Initialised successfully.");
	return true;
}

void SoundSystem::Shutdown()
{
	if (m_pFMODSystem)
	{
		for (auto& soundPair : m_sounds)
		{
			FMOD::Sound* sound = soundPair.second;
			if (sound)
			{
				sound->release();
			}
		}
		m_sounds.clear();

		m_pFMODSystem->close();
		m_pFMODSystem->release();
		m_pFMODSystem = nullptr;
		LogManager::GetInstance().Log("SoundSystem Shutdown.");
	}
}

void SoundSystem::Update()
{
	if (m_pFMODSystem)
	{
		m_pFMODSystem->update();
	}
}

bool SoundSystem::LoadSound(const std::string& pcFilename, const std::string& soundID, bool loop, bool stream)
{
	if (!m_pFMODSystem)
	{
		LogManager::GetInstance().Log("SoundSystem: Cannot load sound - FMOD system not initialized!");
		return false;
	}

	if (m_sounds.find(soundID) != m_sounds.end())
	{
		LogManager::GetInstance().Log(("Sound System: Sound with ID " + soundID + " already exists!").c_str());
		return true;
	}

	FMOD_MODE mode = FMOD_DEFAULT;
	mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	mode |= stream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

	FMOD::Sound* pSound = nullptr;
	FMOD_RESULT result = m_pFMODSystem->createSound(pcFilename.c_str(), mode, nullptr, &pSound);

	if (result != FMOD_OK)
	{
		LogManager::GetInstance().Log(("SoundSystem: Failed to load sound " + pcFilename + " with error: " + std::to_string(result)).c_str());;
		return false;
	}

	m_sounds[soundID] = pSound;
	LogManager::GetInstance().Log(("SoundSystem: Loaded sound " + pcFilename + " as " + soundID).c_str());;
	return true;
}

void SoundSystem::UnloadSound(const std::string& soundID)
{
	auto it = m_sounds.find(soundID);
	if (it != m_sounds.end())
	{
		it->second->release();
		m_sounds.erase(it);
		LogManager::GetInstance().Log(("SoundSystem: Unloaded sound " + soundID).c_str());;
	}
}

FMOD::Channel* SoundSystem::PlaySound(const std::string& soundID, bool paused)
{
	auto it = m_sounds.find(soundID);
	if (it == m_sounds.end() || !m_pFMODSystem)
	{
		LogManager::GetInstance().Log(("SoundSystem: Cannot play sound " + soundID + " - not found or system not initialized!").c_str());;
		return nullptr;
	}

	FMOD::Channel* pChannel = nullptr;
	FMOD_RESULT result = m_pFMODSystem->playSound(it->second, nullptr, paused, &pChannel);

	if (result != FMOD_OK)
	{
		LogManager::GetInstance().Log(("SoundSystem: Failed to play sound " + soundID + " with error: " + std::to_string(result)).c_str());;
		return nullptr;
	}

	return pChannel;
}

void SoundSystem::StopChannel(FMOD::Channel* channel)
{
	if (channel)
	{
		channel->stop();
	}
}

void SoundSystem::SetChannelPaused(FMOD::Channel* channel, bool paused)
{
	if (channel)
	{
		channel->setPaused(paused);
	}
}

void SoundSystem::SetChannelVolume(FMOD::Channel* channel, float volume)
{
	if (channel)
	{
		channel->setVolume(volume);
	}
}