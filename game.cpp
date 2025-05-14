// COMP710 GP Framework

// This include:
#include "Game.h"

// Library includes:
#include "Renderer.h"
#include "LogManager.h"
#include "Sprite.h"
#include "Scene.h"
#include "InputSystem.h"
#include "XboxController.h"
#include "fmod.hpp"
#include <SDL_ttf.h>

//IMGUI INCLUDES
#include "imgui/imgui_impl_sdl2.h"

// SCENE INCLUDES
#include "SceneAbyssWalker.h"
#include "SceneTitleScreen.h"

// Static Members:
Game* Game::sm_pInstance = 0;
FMOD::System* m_pFMODSystem = nullptr;

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
	, m_pCheckerboard(0)
	, m_pChannel(0)
	, m_pFMODSystem(0)
	, m_pSound(0)
{
}

Game::~Game()
{
	delete m_pRenderer;
	delete m_pInputSystem;
	m_pInputSystem = 0;
	m_pRenderer = 0;

	for (Scene* scene : m_scenes)
	{
		delete scene;
	}
	m_scenes.clear();

	TTF_Quit();
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

	// Initialise FMOD
	FMOD::System_Create(&m_pFMODSystem);
	m_pFMODSystem->init(512, FMOD_INIT_NORMAL, 0);

	// INPUT SYSTEM
	m_pInputSystem = new InputSystem();
	bool inputInitialised = m_pInputSystem->Initialise();
	if (!inputInitialised)
	{
		LogManager::GetInstance().Log("InputSystem failed to initialise!");
		return false;
	}

	// Scene Test
	m_scenes.push_back(new SceneTitleScreen());
	m_scenes.push_back(new SceneAbyssWalker(m_pFMODSystem));

	// Initialize title screen
	if (!m_scenes[0]->Initialise(*m_pRenderer)) 
	{
		LogManager::GetInstance().Log("Title screen failed to initialise!");
		return false;
	}

	// Initialize game scene
	if (!m_scenes[1]->Initialise(*m_pRenderer)) 
	{
		LogManager::GetInstance().Log("Game scene failed to initialise!");
		return false;
	}


	// Start with the title screen
	m_iCurrentScene = 0; // Only one scene available

	if (!m_scenes[0]->Initialise(*m_pRenderer))
	{
		LogManager::GetInstance().Log("Title screen failed to initialise!");
		return false;
	}

	return true;
}

bool Game::DoGameLoop()
{
	const float stepSize = 1.0f / 60.0f;

	// TODO: process input here:
	
	m_pInputSystem->ProcessInput();
	
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

void Game::Process(float deltaTime)
{
	ProcessFrameCounting(deltaTime);

	// Check if scene changed via IMGUI
	static int previousScene = m_iCurrentScene;
	if (m_iCurrentScene != previousScene)
	{
		// Re-initialize the new scene
		if (!m_scenes[m_iCurrentScene]->Initialise(*m_pRenderer))
		{
			LogManager::GetInstance().Log("Failed to initialise scene!");
			m_iCurrentScene = previousScene;
		}
		previousScene = m_iCurrentScene;
	}

	m_scenes[m_iCurrentScene]->Process(deltaTime, *m_pInputSystem);

	// Update FMOD system
	if (m_pFMODSystem)
	{
		m_pFMODSystem->update();
	}
}

void Game::Draw(Renderer& renderer)
{
	++m_iFrameCount;

	renderer.Clear();

	m_scenes[m_iCurrentScene]->Draw(renderer);

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