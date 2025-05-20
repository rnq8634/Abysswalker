// COMP710 GP Framework
// This include:
#include "Game.h"

// Local includes:
#include "Renderer.h"
#include "LogManager.h"
#include "Sprite.h"
#include "Scene.h"
#include "InputSystem.h"
#include "XboxController.h"
#include "fmod.hpp"
#include "SoundSystem.h"

// Lib icnludes
#include <SDL_ttf.h>
#include <cstdlib>
#include <crtdbg.h>

//IMGUI INCLUDES
#include "imgui/imgui_impl_sdl2.h"

// SCENE INCLUDES
#include "SceneAbyssWalker.h"
#include "SceneTitleScreen.h"
#include "SceneSplashScreenFMOD.h"
#include "SceneSplashScreenAUT.h"

// Static Members:
Game* Game::sm_pInstance = 0;

bool Game::s_bGodMode = false;
bool Game::s_bOneShotMode = false;
bool Game::s_bInfiniteStaminaMode = false;

Game& Game::GetInstance()
{
	if (sm_pInstance == 0)
	{
		sm_pInstance = new Game();
	}
	return (*sm_pInstance);
}

void Game::DestroyInstance()
{
	delete sm_pInstance;
	sm_pInstance = 0;
}

Game::Game()
	: m_pRenderer(0)
	, m_bLooping(true)
	, m_pInputSystem(0)
	, m_bShowDebugWindow(0)
	, m_iCurrentScene(0)
	, m_fElaspedSeconds(0.0f)
	, m_fExecutionTime(0.0f)
	, m_iFPS(0)
	, m_iFrameCount(0)
	, m_iLastTime(0)
	, m_pCurrentScenePtr(nullptr)
{
}

Game::~Game()
{
	m_pCurrentScenePtr = nullptr;

	delete m_pRenderer;
	delete m_pInputSystem;
	m_pInputSystem = 0;
	m_pRenderer = 0;

	// Delete Scenes
	for (Scene* scene : m_scenes)
	{
		delete scene;
	}
	m_scenes.clear();

	// Delete Game systems
	delete m_pRenderer;
	m_pRenderer = nullptr;

	delete m_pInputSystem;
	m_pInputSystem = nullptr;

	// Libraries and subsystems
	SoundSystem::DestroyInstance();
	TTF_Quit();

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
}

void Game::Quit()
{
	m_bLooping = false;
}

// Where scenes will be added
bool Game::Initialise()
{
	// WIndow screen
	int bbWidth = 1280;
	int bbHeight = 720;

	m_pRenderer = new Renderer();
	if (!m_pRenderer->Initialize(false, bbWidth, bbHeight))
	{
		LogManager::GetInstance().Log("Renderer failed to initialise!");
		return false;
	}

	bbWidth = m_pRenderer->GetWidth();
	bbHeight = m_pRenderer->GetHeight();

	m_iLastTime = SDL_GetPerformanceCounter();

	// CHange backgroudn color
	m_pRenderer->SetClearColor(50, 50, 50);

	if (TTF_Init() == -1)
	{
		LogManager::GetInstance().Log("SDL_ttf has failed!");
		return false;
	}

	if (!SoundSystem::GetInstance().Initialise())
	{
		LogManager::GetInstance().Log("SoundSystem failed to initialise!");
		return false;
	}

	// INPUT SYSTEM
	m_pInputSystem = new InputSystem();
	bool inputInitialised = m_pInputSystem->Initialise();
	if (!inputInitialised)
	{
		LogManager::GetInstance().Log("InputSystem failed to initialise!");
		return false;
	}

	// Scene Test
	m_scenes.clear();
	m_scenes.push_back(new SceneSplashScreenFMOD());
	m_scenes.push_back(new SceneSplashScreenAUT());
	m_scenes.push_back(new SceneTitleScreen());
	m_scenes.push_back(new SceneAbyssWalker());

	return SetCurrentScene(SCENE_INDEX_FMODSPLASH, true);

	return true;
}

bool Game::SetCurrentScene(int index, bool forceInitialise)
{
	if (index >= 0 && static_cast<size_t>(index) < m_scenes.size())
	{
		m_iCurrentScene = index;
		m_pCurrentScenePtr = m_scenes[m_iCurrentScene];

		if (!m_pCurrentScenePtr->Initialise(*m_pRenderer))
		{
			LogManager::GetInstance().Log(("Scene " + std::to_string(m_iCurrentScene) + " failed to init!!").c_str());
			Quit();
			return false;
		}
		return true;
	}
	else
	{
		LogManager::GetInstance().Log(("Error: Attempted to set invalid scene index: " + std::to_string(index)).c_str());
		return false;
	}
}

int Game::GetCurrentSceneIndex() const
{
	return m_iCurrentScene;
}

InputSystem* Game::GetInputSystem()
{
	return m_pInputSystem;
}

bool Game::DoGameLoop()
{
	const float stepSize = 1.0f / 60.0f;

	// TODO: process input here:
	
	m_pInputSystem->ProcessInput();

	ProcessCheats();
	
	if (m_bLooping)
	{
		Uint64 current = SDL_GetPerformanceCounter();
		float deltaTime = (current - m_iLastTime) / static_cast<float>(SDL_GetPerformanceFrequency());

		m_iLastTime = current;

		m_fExecutionTime += deltaTime;
		
		Process(deltaTime);

#ifdef USE_LAG
		m_fLag += deltaTime;
		
		int innerLag = 0;

		while (m_fLag >= stepSize)
		{
			Process(stepSize);

			m_fLag -= stepSize;

			++m_iUpdateCount;
			++innerLag;
		}
#endif // USE_LAG

		Draw(*m_pRenderer);
	}

	return m_bLooping;
}

void Game::ProcessCheats()
{
	if (!m_pInputSystem) return;

	// GOD MODE! Key: F1
	if (m_pInputSystem->GetKeyState(SDL_SCANCODE_F1) == BS_PRESSED)
	{
		s_bGodMode = !s_bGodMode;
		LogManager::GetInstance().Log(s_bGodMode ? "CHEAT: God Mode ENABLED" : "CHEAT: God Mode DISABLED");
	}

	// ONE SHOT MODE! Key: F2
	if (m_pInputSystem->GetKeyState(SDL_SCANCODE_F2) == BS_PRESSED)
	{
		s_bOneShotMode = !s_bOneShotMode;
		LogManager::GetInstance().Log(s_bOneShotMode ? "CHEAT: One Shot Mode ENABLED" : "CHEAT: One Shot Mode DISABLED");
	}

	// INFINITE STAMINA! Key: F3
	if (m_pInputSystem->GetKeyState(SDL_SCANCODE_F3) == BS_PRESSED)
	{
		s_bInfiniteStaminaMode = !s_bInfiniteStaminaMode;
		LogManager::GetInstance().Log(s_bInfiniteStaminaMode ? "CHEAT: Inf Stam Mode ENABLED" : "CHEAT: Inf Stam Mode DISABLED");
	}

	// SKIP TO LAST WAVE! Key: F4
	if (m_pInputSystem->GetKeyState(SDL_SCANCODE_F4) == BS_PRESSED)
	{
		if (m_iCurrentScene == SCENE_INDEX_ABYSSWALKER && m_pCurrentScenePtr)
		{
			SceneAbyssWalker* pAbyssScene = static_cast<SceneAbyssWalker*>(m_pCurrentScenePtr);
			if (pAbyssScene)
			{
				LogManager::GetInstance().Log("CHEAT: F4 Pressed - Skipping to last wave!!");
				pAbyssScene->DebugSkipToLastWave();
			}
		}
	}
}

void Game::Process(float deltaTime)
{
	ProcessFrameCounting(deltaTime);

	int sceneIndexBeforeAutoTransition = m_iCurrentScene;

	if (m_iCurrentScene == SCENE_INDEX_FMODSPLASH)
	{
		SceneSplashScreenFMOD* fmodSplash = dynamic_cast<SceneSplashScreenFMOD*>(m_scenes[SCENE_INDEX_FMODSPLASH]);
		if (fmodSplash && fmodSplash->IsFinished())
		{
			SetCurrentScene(SCENE_INDEX_AUTSPLASH);
		}
	}

	else if (m_iCurrentScene == SCENE_INDEX_AUTSPLASH)
	{
		SceneSplashScreenAUT* autSplash = dynamic_cast<SceneSplashScreenAUT*>(m_scenes[SCENE_INDEX_AUTSPLASH]);
		if (autSplash && autSplash->IsFinished())
		{
			SetCurrentScene(SCENE_INDEX_TITLE);
		}
	}

	if (m_pCurrentScenePtr)
	{
		m_pCurrentScenePtr->Process(deltaTime, *m_pInputSystem);
	}
	else
	{
		LogManager::GetInstance().Log("Error: m_pCurrentScenePtr is null in Game::Process. Quitting.");
		Quit();
		return;
	}

	// Update FMOD system
	SoundSystem::GetInstance().Update();
}

void Game::Draw(Renderer& renderer)
{
	++m_iFrameCount;

	renderer.Clear();

	if (m_pCurrentScenePtr)
	{
		m_pCurrentScenePtr->Draw(renderer);
	}

	// TODO: Add game objects to draw here!

	DebugDraw();

	renderer.Present();
}

void Game::ProcessFrameCounting(float deltaTime)
{
	// Count total simulation time elapsed:
	m_fElaspedSeconds += deltaTime;

	// Frame Counter:
	if (m_fElaspedSeconds > 1.0f)
	{
		m_fElaspedSeconds -= 1.0f;
		m_iFPS = m_iFrameCount;
		m_iFrameCount = 0;
	}
}

void
Game::DebugDraw()
{
	if (m_bShowDebugWindow)
	{
		bool open = true;

		ImGui::Begin("Debug Window", &open, ImGuiWindowFlags_MenuBar);

		ImGui::Text("COMP710 GP FRAMEWORK (%s)", "2025, S1");

		if (ImGui::Button("Quit"))
		{
			Quit();
		}

		ImGui::SliderInt("Active scene", &m_iCurrentScene, 0, m_scenes.size() - 1, "%d");
		m_scenes[m_iCurrentScene]->DebugDraw();

		ImGui::End();
	}
}

void
Game::ToggleDebugWindow()
{
	m_bShowDebugWindow = !m_bShowDebugWindow;

	m_pInputSystem->ShowMouseCursor(m_bShowDebugWindow);
}