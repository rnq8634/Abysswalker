// This includes
#include "SceneSplashScreenAUT.h"

// Local Includes
#include "Renderer.h"
#include "InputSystem.h"
#include "Sprite.h"
#include "Texture.h"
#include "LogManager.h"
#include "Game.h"

// IMGUI
#include "imgui/imgui.h"

const char* AUT_LOGO_FILEPATH = "assets/logos/AUT.png";
const float AUT_DISPLAY_DURATION = 3.0f;
const float AUT_FADE_DURATION = 1.0f;

SceneSplashScreenAUT::SceneSplashScreenAUT()
	: m_pAUTLogoSprite(nullptr)
	, m_pAUTLogoTexture(nullptr)
	, m_fDisplayTime(AUT_DISPLAY_DURATION)
	, m_fFadeTime(AUT_FADE_DURATION)
	, m_fTimer(0.0f)
	, m_fAlpha(0.0f)
	, m_eCurrentState(AUTSplashState::FADING_IN)
{
}

SceneSplashScreenAUT::~SceneSplashScreenAUT()
{
	delete m_pAUTLogoSprite;
	m_pAUTLogoSprite = nullptr;
	delete m_pAUTLogoTexture;
	m_pAUTLogoTexture = nullptr;
}

bool SceneSplashScreenAUT::Initialise(Renderer& renderer)
{
	renderer.SetClearColor(0, 0, 0);

	m_pAUTLogoTexture = new Texture();
	if (!m_pAUTLogoTexture)
	{
		LogManager::GetInstance().Log("Failed to allocate AUT Logo texture.");
		return false;
	}

	if (!m_pAUTLogoTexture->Initialise(AUT_LOGO_FILEPATH))
	{
		LogManager::GetInstance().Log("Failed to initialise AUT Logo texture from file!!");
		delete m_pAUTLogoTexture;
		m_pAUTLogoTexture = nullptr;
		return false;
	}

	m_pAUTLogoSprite = new Sprite();
	if (!m_pAUTLogoSprite)
	{
		LogManager::GetInstance().Log("Failed to allocate AUT Logo Sprite!");
		delete m_pAUTLogoTexture;
		m_pAUTLogoTexture = nullptr;
		return false;
	}

	if (!m_pAUTLogoSprite->Initialise(*m_pAUTLogoTexture))
	{
		LogManager::GetInstance().Log("Failed to Init AUT Logo Sprite with texture!");
		delete m_pAUTLogoSprite;
		m_pAUTLogoSprite = nullptr;
		delete m_pAUTLogoTexture;
		m_pAUTLogoTexture = nullptr;
		return false;
	}

	// Position for the Logos (Centered)
	int screenWidth = renderer.GetWidth();
	int screenHeight = renderer.GetHeight();

	m_pAUTLogoSprite->SetX(screenWidth / 2);
	m_pAUTLogoSprite->SetY(screenHeight / 2);
	m_pAUTLogoSprite->SetAlpha(0.0f);

	m_fTimer = 0.0f;
	m_fAlpha = 0.0f;
	m_eCurrentState = AUTSplashState::FADING_IN;

	return true;
}

void SceneSplashScreenAUT::Process(float deltaTime, InputSystem& inputSystem)
{
	m_fTimer += deltaTime;

	switch (m_eCurrentState)
	{
	case AUTSplashState::FADING_IN:
		m_fAlpha = m_fTimer / m_fFadeTime;
		if (m_fAlpha >= 1.0f)
		{
			m_fAlpha = 1.0f;
			m_eCurrentState = AUTSplashState::DISPLAYING;
			m_fTimer = 0.0f;
		}
		break;

	case AUTSplashState::DISPLAYING:
		if (m_fTimer >= m_fDisplayTime)
		{
			m_eCurrentState = AUTSplashState::FADING_OUT;
			m_fTimer = 0.0f;
		}
		break;

	case AUTSplashState::FADING_OUT:
		m_fAlpha = 1.0f - (m_fTimer / m_fFadeTime);
		if (m_fAlpha <= 0.0f)
		{
			m_fAlpha = 0.0f;
			m_eCurrentState = AUTSplashState::FINISHED;
		}
		break;

	case AUTSplashState::FINISHED:
		break;
	}

	if (m_pAUTLogoSprite)
	{
		m_pAUTLogoSprite->SetAlpha(m_fAlpha);
	}

	// NOTE: Optional, can add a skipping button if have enough time
}

void SceneSplashScreenAUT::Draw(Renderer& renderer)
{
	if (m_pAUTLogoSprite && m_eCurrentState != AUTSplashState::FINISHED)
	{
		m_pAUTLogoSprite->SetAngle(180.0f);
		m_pAUTLogoSprite->SetFlipHorizontal(true);
		m_pAUTLogoSprite->Draw(renderer);
	}
}

void SceneSplashScreenAUT::DebugDraw()
{
	ImGui::Text("AUT Splash Screen");
	ImGui::Text("State: %d", static_cast<int>(m_eCurrentState));
	ImGui::Text("Timer: %.2f", m_fTimer);
	ImGui::Text("Alpha: %.2f", m_fAlpha);
}

bool SceneSplashScreenAUT::IsFinished() const
{
	return m_eCurrentState == AUTSplashState::FINISHED;
}