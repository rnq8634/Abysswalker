#ifndef __SOUNDSYSTEM_H
#define  __SOUNDSYSTEM_H

// Includes
#include "fmod.hpp"
#include <string>
#include <map>

class SoundSystem
{
public:
	static SoundSystem& GetInstance();
	static void DestroyInstance();

	SoundSystem(const SoundSystem&) = delete;
	SoundSystem& operator=(const SoundSystem&) = delete;

	bool Initialise(int maxChannels = 128, FMOD_INITFLAGS flags = FMOD_INIT_NORMAL, void* extraDriverData = nullptr);
	void Shutdown();
	void Update();

	bool LoadSound(const std::string& pcFilename, const std::string& soundID, bool loop = false, bool stream = false);
	void UnloadSound(const std::string& soundID);

	FMOD::Channel* PlaySound(const std::string& soundID, bool paused = false);

	void StopChannel(FMOD::Channel* channel);
	void SetChannelPaused(FMOD::Channel* channel, bool paused);
	void SetChannelVolume(FMOD::Channel* channel, float volume);

	FMOD::System* GetFMODSystem() { return m_pFMODSystem;  }

private:
	SoundSystem();
	~SoundSystem();

	static SoundSystem* sm_pInstance;

	FMOD::System* m_pFMODSystem;
	std::map<std::string, FMOD::Sound*> m_sounds;
};

#endif // __SOUNDSYSTEM_H
