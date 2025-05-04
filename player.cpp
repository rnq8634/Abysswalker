// This include
#include "Player.h"

// Local Includes
#include "Renderer.h"
#include "AnimatedSprite.h"
#include "InlineHelpers.h"
#include "LogManager.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <cassert>
#include <cstdlib>

Player::Player()
	: m_pSprite(0)
	, m_bAlive(true)
	, m_pStaticSprite(0)
{

}

Player::~Player()
{
	delete m_pSprite;
	m_pSprite = 0;
}

bool
Player::Initialise(Renderer& renderer)
{
	// Needs static image for the animation to play
	m_pStaticSprite = renderer.CreateSprite("assets/player/_Idle.png");
	m_pSprite = renderer.CreateAnimatedSprite("assets/player/_Idle.png");

	if (!m_pSprite)
	{
		LogManager::GetInstance().Log("Failed to create player sprite!");
		return false;
	}

	// Set up the frames
	m_pSprite->SetupFrames(120, 80);

	// Config
	m_pSprite->SetFrameDuration(0.1f);
	m_pSprite->SetLooping(true);

	m_pSprite->SetAlpha(1.0f);

	m_position.x = renderer.GetWidth() / 2;
	m_position.y = renderer.GetHeight() / 2;

	m_pSprite->SetX(m_position.x);
	m_pSprite->SetY(m_position.y);

	// Start with the animation playing
	m_pSprite->Animate();

	return true;
}

void
Player::Process(float deltaTime)
{
	if (!m_bAlive)
	{
		return;
	}

	// Update the sprite animation
	if (m_pSprite)
	{
		m_pSprite->Process(deltaTime);
	}
}

void
Player::Draw(Renderer& renderer)
{
	if (!m_bAlive || !m_pSprite)
	{
		return;
	}
	m_pStaticSprite->Draw(renderer);
	m_pSprite->Draw(renderer);
}

void
Player::DebugDraw()
{
	if (ImGui::CollapsingHeader("Player"))
	{
		ImGui::Text("Position: (%.1f, %.1f)", m_position.x, m_position.y);
		ImGui::Checkbox("Alive", &m_bAlive);

		if (m_pSprite)
		{
			ImGui::Text("Sprite dimensions: %d x %d", m_pSprite->GetWidth(), m_pSprite->GetHeight());
			ImGui::Text("Animation playing: %s", m_pSprite->IsAnimating() ? "Yes" : "No");

			// Allow toggling animation
			bool isAnimating = m_pSprite->IsAnimating();
			if (ImGui::Checkbox("Animate", &isAnimating))
			{
				if (isAnimating)
				{
					m_pSprite->Animate();
				}
				else
				{
					// Note: AnimatedSprite doesn't have a Stop() method
					// This resets to first frame but doesn't actually stop animating
					m_pSprite->Restart();
				}
			}

			// Show the sprite's debug UI
			m_pSprite->DebugDraw();
		}
	}
}

Vector2&
Player::Position()
{
	return m_position;
}