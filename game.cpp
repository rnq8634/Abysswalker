// COMP710 GP Framework

// This include:
#include "game.h"

// Library includes:
#include "renderer.h"
#include "logmanager.h"
#include "scene.h"
#include "inputsystem.h"

//IMGUI INCLUDES
#include "imgui/imgui_impl_sdl2.h"

// SCENE INCLUDES
#include "SceneAbyssWalker.h"

// Static Members:
Game* Game::sm_pInstance = 0;

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

{

}

Game::~Game()
{
	delete m_pRenderer;
	delete m_pInputSystem;
	m_pInputSystem = 0;
	m_pRenderer = 0;
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

	// INPUT SYSTEM
	m_pInputSystem = new InputSystem();
	bool inputInitialised = m_pInputSystem->Initialise();
	if (!inputInitialised)
	{
		return false;
	}

	// Scene Test
	Scene* pScene = 0;
	pScene = new SceneAbyssWalker();
	if (!pScene->Initialise(*m_pRenderer))
	{
		LogManager::GetInstance().Log("Abysswalker failed to launch!");
		return false;
	}

	m_scenes.push_back(pScene);
	m_iCurrentScene = 0; // Only one scene available

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

	m_scenes[m_iCurrentScene]->Process(deltaTime, *m_pInputSystem);

	// TODO: Add game objects to process here!
	
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