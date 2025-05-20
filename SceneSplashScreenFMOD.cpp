// This includes
#include "SceneSplashScreenFMOD.h"

// Local Includes
#include "Renderer.h"
#include "InputSystem.h"
#include "Sprite.h"
#include "Texture.h"
#include "LogManager.h"
#include "Game.h"

// IMGUI
#include "imgui/imgui.h"

const char* FMOD_LOGO_FILEPATH = "assets/logos/FMOD_BLACK_BACKGROUND.png";
const float FMOD_DISPLAY_DURATION = 3.0f;
const float FMOD_FADE_DURATION = 1.0f;

SceneSplashScreenFMOD::SceneSplashScreenFMOD()
	: m_pFMODLogoSprite(nullptr)
	, m_pFMODLogoTexture(nullptr)
	, m_fDisplayTime(FMOD_DISPLAY_DURATION)
	, m_fFadeTime(FMOD_FADE_DURATION)
	, m_fTimer(0.0f)
	, m_fAlpha(0.0f)
	, m_eCurrentState(FMODSplashState::FADING_IN)
{
}

SceneSplashScreenFMOD::~SceneSplashScreenFMOD()
{
	delete m_pFMODLogoSprite;
	m_pFMODLogoSprite = nullptr;
	delete m_pFMODLogoTexture;
	m_pFMODLogoTexture = nullptr;
}

bool SceneSplashScreenFMOD::Initialise(Renderer& renderer)
{
	renderer.SetClearColor(0, 0, 0);

	m_pFMODLogoTexture = new Texture();
	if (!m_pFMODLogoTexture)
	{
		LogManager::GetInstance().Log("Failed to allocate FMOD Logo texture.");
		return false;
	}

	if (!m_pFMODLogoTexture->Initialise(FMOD_LOGO_FILEPATH))
	{
		LogManager::GetInstance().Log("Failed to initialise FMOD Logo texture from file!!");
		delete m_pFMODLogoTexture;
		m_pFMODLogoTexture = nullptr;
		return false;
	}

	m_pFMODLogoSprite = new Sprite();
	if (!m_pFMODLogoSprite) 
	{
		LogManager::GetInstance().Log("Failed to allocate FMOD Logo Sprite!");
		delete m_pFMODLogoTexture;
		m_pFMODLogoTexture = nullptr;
		return false;
	}

	if (!m_pFMODLogoSprite->Initialise(*m_pFMODLogoTexture))
	{
		LogManager::GetInstance().Log("Failed to Init FMOD Logo Sprite with texture!");
		delete m_pFMODLogoSprite;
		m_pFMODLogoSprite = nullptr;
		delete m_pFMODLogoTexture;
		m_pFMODLogoTexture = nullptr;
		return false;
	}

	// Position for the Logos (Centered)
	int screenWidth = renderer.GetWidth();
	int screenHeight = renderer.GetHeight();

	m_pFMODLogoSprite->SetX(screenWidth / 2);
	m_pFMODLogoSprite->SetY(screenHeight / 2);
	m_pFMODLogoSprite->SetAlpha(0.0f);

	m_fTimer = 0.0f;
	m_fAlpha = 0.0f;
	m_eCurrentState = FMODSplashState::FADING_IN;

	return true;
}

void SceneSplashScreenFMOD::Process(float deltaTime, InputSystem& inputSystem)
{
	m_fTimer += deltaTime;

	switch (m_eCurrentState)
	{
	case FMODSplashState::FADING_IN:
		m_fAlpha = m_fTimer / m_fFadeTime;
		if (m_fAlpha >= 1.0f)
		{
			m_fAlpha = 1.0f;
			m_eCurrentState = FMODSplashState::DISPLAYING;
			m_fTimer = 0.0f;
		}
		break;

	case FMODSplashState::DISPLAYING:
		if (m_fTimer >= m_fDisplayTime)
		{
			m_eCurrentState = FMODSplashState::FADING_OUT;
			m_fTimer = 0.0f;
		}
		break;

	case FMODSplashState::FADING_OUT:
		m_fAlpha = 1.0f - (m_fTimer / m_fFadeTime);
		if (m_fAlpha <= 0.0f)
		{
			m_fAlpha = 0.0f;
			m_eCurrentState = FMODSplashState::FINISHED;
		}
		break;

	case FMODSplashState::FINISHED:
		break;
	}

	if (m_pFMODLogoSprite)
	{
		m_pFMODLogoSprite->SetAlpha(m_fAlpha);
	}

	// NOTE: Optional, can add a skipping button if have enough time
}

void SceneSplashScreenFMOD::Draw(Renderer& renderer)
{
	if (m_pFMODLogoSprite && m_eCurrentState != FMODSplashState::FINISHED)
	{
		m_pFMODLogoSprite->SetAngle(180.0f);
		m_pFMODLogoSprite->SetFlipHorizontal(true);
		m_pFMODLogoSprite->Draw(renderer);
	}
}

void SceneSplashScreenFMOD::DebugDraw()
{
	ImGui::Text("FMOD Splash Screen");
	ImGui::Text("State: %d", static_cast<int>(m_eCurrentState));
	ImGui::Text("Timer: %.2f", m_fTimer);
	ImGui::Text("Alpha: %.2f", m_fAlpha);
}

bool SceneSplashScreenFMOD::IsFinished() const
{
	return m_eCurrentState == FMODSplashState::FINISHED;
}