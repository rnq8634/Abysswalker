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
	: m_pIdleSprite(0)
	, m_pRunSprite(0)
	, m_bAlive(true)
	, m_pStaticSprite(0)
	, m_currentState(PlayerState::IDLE)
	, m_bFacingRight(true)
	, m_turnCompleteCallback(nullptr)
	, m_desiredMoveSpeed(0)
	, m_isTurning(false)
{
	m_velocity.Set(0, 0);
}

Player::~Player()
{
	delete m_pStaticSprite;
	delete m_pIdleSprite;
	delete m_pRunSprite;
	delete m_pJumpSprite;
	delete m_pFallSprite;
	delete m_pAttackSprite;
	delete m_pTurnAroundSprite;
	m_pIdleSprite = 0;
	m_pStaticSprite = 0;
	m_pRunSprite = 0;
	m_pJumpSprite = 0;
	m_pFallSprite = 0;
	m_pAttackSprite = 0;
	m_pTurnAroundSprite = 0;
}

bool
Player::Initialise(Renderer& renderer)
{
	// Load static image
	m_pStaticSprite = renderer.CreateSprite("assets/player/_Idle.png");

	// -------------------------------------Idle Animation-------------------------------
	m_pIdleSprite = renderer.CreateAnimatedSprite("assets/player/_Idle.png");
	if (!m_pIdleSprite)
	{
		LogManager::GetInstance().Log("Failed to create idle animation!");
		return false;
	}
	// Idle config
	m_pIdleSprite->SetupFrames(120, 80);
	m_pIdleSprite->SetFrameDuration(0.15f);
	m_pIdleSprite->SetLooping(true);
	m_pIdleSprite->Animate();

	// --------------------------------Jump Animation-----------------------------
	m_pJumpSprite = renderer.CreateAnimatedSprite("assets/player/_Jump.png");
	if (!m_pJumpSprite)
	{
		LogManager::GetInstance().Log("Failed to create jump animation!");
		return false;
	}
	// Jump Config
	m_pJumpSprite->SetupFrames(120, 80);
	m_pJumpSprite->SetFrameDuration(0.2f); // How long the frame is shown
	m_pJumpSprite->SetLooping (false);
	m_pJumpSprite->Animate();

	// ----------------------------------Jump Fall Animation--------------------------------
	m_pFallSprite = renderer.CreateAnimatedSprite("assets/player/_Fall.png");
	if (!m_pFallSprite)
	{
		LogManager::GetInstance().Log("Failed to create fall animation!");
		return false;
	}
	// Jump Config
	m_pFallSprite->SetupFrames(120, 80);
	m_pFallSprite->SetFrameDuration(0.2f);
	m_pFallSprite->SetLooping(false);
	m_pFallSprite->Animate();

	// ----------------------------------Attack Animation------------------------------------
	m_pAttackSprite = renderer.CreateAnimatedSprite("assets/player/_AttackComboNoMovement.png");
	if (!m_pAttackSprite)
	{
		LogManager::GetInstance().Log("Failed to create attack combo animation!");
		return false;
	}
	// Attack config
	m_pAttackSprite->SetupFrames(120, 80);
	m_pAttackSprite->SetFrameDuration(0.1f);
	m_pAttackSprite->SetLooping(true);
	m_pAttackSprite->Animate();

	// ---------------------------------TurnAround Animation-------------------------------------
	m_pTurnAroundSprite = renderer.CreateAnimatedSprite("assets/player/_TurnAround.png");
	if (!m_pTurnAroundSprite)
	{
		LogManager::GetInstance().Log("Failed to create turn around animation!");
		return false;
	}
	// Attack config
	m_pTurnAroundSprite->SetupFrames(120, 80);
	m_pTurnAroundSprite->SetFrameDuration(0.1f);
	m_pTurnAroundSprite->SetLooping(false);

	// Callback for when the animation completes
	m_pTurnAroundSprite->SetAnimationCompleteCallback([this]()
		{
			this->OnTurnAnimationComplete();
		});

	m_pTurnAroundSprite->Animate();

	// ---------------------------------Roll Animation---------------------------------------

	// -----------------------------------Running Animation----------------------------------
	m_pRunSprite = renderer.CreateAnimatedSprite("assets/player/_Run.png");
	if (!m_pRunSprite)
	{
		LogManager::GetInstance().Log("Failed to create run animation!");
		return false;
	}

	// Run config
	m_pRunSprite->SetupFrames(120, 80);
	m_pRunSprite->SetFrameDuration(0.1f);
	m_pRunSprite->SetLooping(true);
	m_pRunSprite->Animate();

	// Position to be center of screen
	m_position.x = renderer.GetWidth() / 2;
	m_position.y = renderer.GetHeight() / 2;

	return true;
}

void
Player::Process(float deltaTime)
{
	if (!m_bAlive)
	{
		return;
	}

	// Update position based on velocity (only if not turning)
	if (m_currentState != PlayerState::TURNING)
	{
		m_position += m_velocity * deltaTime;
	}

	// Update the sprite animation based on current state
	switch (m_currentState)
	{
	case PlayerState::IDLE:
		if (m_pIdleSprite)
		{
			m_pIdleSprite->Process(deltaTime);
			m_pIdleSprite->SetX(m_position.x);
			m_pIdleSprite->SetY(m_position.y);
			m_pIdleSprite->SetFlipHorizontal(!m_bFacingRight);
		}
		break;

	case PlayerState::RUNNING:
		if (m_pRunSprite)
		{
			m_pRunSprite->Process(deltaTime);
			m_pRunSprite->SetX(m_position.x);
			m_pRunSprite->SetY(m_position.y);
			m_pRunSprite->SetFlipHorizontal(!m_bFacingRight);
		}
		break;

	case PlayerState::TURNING:
		if (m_pTurnAroundSprite)
		{
			m_pTurnAroundSprite->Process(deltaTime);
			m_pTurnAroundSprite->SetX(m_position.x);
			m_pTurnAroundSprite->SetY(m_position.y);

			// When turning, we flip the sprite based on the direction we're coming from
			// If currently facing right and turning left, don't flip
			// If currently facing left and turning right, flip
			m_pTurnAroundSprite->SetFlipHorizontal(!m_bFacingRight);
		}
		break;

	case PlayerState::JUMPING:
		if (m_pJumpSprite)
		{
			m_pJumpSprite->Process(deltaTime);
			m_pJumpSprite->SetX(m_position.x);
			m_pJumpSprite->SetY(m_position.y);
			m_pJumpSprite->SetFlipHorizontal(!m_bFacingRight);
		}
		break;

	case PlayerState::FALLING:
		if (m_pFallSprite)
		{
			m_pFallSprite->Process(deltaTime);
			m_pFallSprite->SetX(m_position.x);
			m_pFallSprite->SetY(m_position.y);
			m_pFallSprite->SetFlipHorizontal(!m_bFacingRight);
		}
		break;

	case PlayerState::ATTACKING:
		if (m_pAttackSprite)
		{
			m_pAttackSprite->Process(deltaTime);
			m_pAttackSprite->SetX(m_position.x);
			m_pAttackSprite->SetY(m_position.y);
			m_pAttackSprite->SetFlipHorizontal(!m_bFacingRight);
		}
		break;
	}
}

void
Player::Draw(Renderer& renderer)
{
	if (!m_bAlive)
	{
		return;
	}

	// Load static image
	m_pStaticSprite->Draw(renderer);

	// Draw based on current state of player
	switch (m_currentState)
	{
	case PlayerState::IDLE:
		if (m_pIdleSprite) m_pIdleSprite->Draw(renderer);
		break;

	case PlayerState::RUNNING:
		if (m_pRunSprite) m_pRunSprite->Draw(renderer);
		break;
	
	case PlayerState::TURNING:
		if (m_pTurnAroundSprite) m_pTurnAroundSprite->Draw(renderer);
		break;

	}
}

//-----------------------------------------------Player States---------------------------------------------------------
void
Player::MoveLeft(float amount)
{
	// If turning, dont interrupt
	if (m_currentState == PlayerState::TURNING)
	{
		return;
	}

	// If already moving left, then just keep running
	if (m_velocity.x < 0 && !m_bFacingRight)
	{
		m_velocity.x = -amount;
		m_currentState = PlayerState::RUNNING;
		return;
	}

	// If currently idle or running AND already facing right, start/continue running
	if ((m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING) && !m_bFacingRight)
	{
		m_velocity.x = -amount;
		m_currentState = PlayerState::RUNNING;
		return;
	}

	// If facing left and need to turn right, start turn animation
	if (m_bFacingRight)
	{
		StartTurnAnimation(-amount, false);
	}
}

void
Player::MoveRight(float amount)
{
	// If turning, dont interrupt
	if (m_currentState == PlayerState::TURNING)
	{
		return;
	}

	// If already moving right, then just keep running
	if (m_velocity.x > 0 && m_bFacingRight)
	{
		m_velocity.x = amount;
		m_currentState = PlayerState::RUNNING;
		return;
	}

	// If currently idle or running AND already facing right, start/continue running
	if ((m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING) && m_bFacingRight)
	{
		m_velocity.x = amount;
		m_currentState = PlayerState::RUNNING;
		return;
	}

	// If facing left and need to turn right, start turn animation
	if (!m_bFacingRight)
	{
		StartTurnAnimation(amount, true);
	}
}

void
Player::StartTurnAnimation(float desiredSpeed, bool turnToRight)
{
	// Save the desired speed to apply after turning completes
	m_desiredMoveSpeed = desiredSpeed;

	// Set turning state
	m_currentState = PlayerState::TURNING;
	m_isTurning = true;

	// Direction we'll face after turning
	m_targetFacingRight = turnToRight;

	// Slow down while turning
	m_velocity.x = m_desiredMoveSpeed * 0.2f;

	// Restart the turn animation
	if (m_pTurnAroundSprite)
	{
		m_pTurnAroundSprite->Restart();
		m_pTurnAroundSprite->Animate();
	}
}

void
Player::OnTurnAnimationComplete()
{
	if (m_isTurning)
	{
		// Update the facing direction
		m_bFacingRight = m_targetFacingRight;

		// Apply the saved velocity once turn is complete
		m_velocity.x = m_desiredMoveSpeed;

		// Change to running state
		m_currentState = PlayerState::RUNNING;
		m_isTurning = false;
	}
}

void
Player::StopMoving()
{
	// Dont intereuppt turning animation
	if (m_currentState == PlayerState::TURNING)
	{
		return;
	}

	m_velocity.x = 0;
	m_currentState = PlayerState::IDLE;
}

void
Player::Jump(float amount) // NEED TO BE ABLE TO JUMP AND FACE LEFT AS WELL
{
	m_currentState = PlayerState::JUMPING;
}

void
Player::Fall(float amount)
{
	m_currentState = PlayerState::FALLING;
}

void
Player::Attack(float amount)
{
	m_currentState = PlayerState::ATTACKING;
}

void
Player::TurnAround(float amount)
{
	m_currentState = PlayerState::TURNING;
}

void
Player::Roll(float amount)
{
	m_currentState = PlayerState::ROLLING;
}

// ------------------------------------------Debugging-------------------------------------------------------
void
Player::DebugDraw()
{
	if (ImGui::CollapsingHeader("Player"))
	{
		ImGui::Text("Position: (%.1f, %.1f)", m_position.x, m_position.y);
		ImGui::Checkbox("Alive", &m_bAlive);
		ImGui::Text("Facing: %s", m_bFacingRight ? "Right" : "Left");

		if (m_pRunSprite)
		{
			ImGui::Text("Sprite dimensions: %d x %d", m_pRunSprite->GetWidth(), m_pRunSprite->GetHeight());
			ImGui::Text("Animation playing: %s", m_pRunSprite->IsAnimating() ? "Yes" : "No");
			ImGui::Text("Flipped: %s", m_pRunSprite->IsFlippedHorizontal() ? "Yes" : "No");

			// Allow toggling animation
			bool isAnimating = m_pRunSprite->IsAnimating();
			if (ImGui::Checkbox("Animate", &isAnimating))
			{
				if (isAnimating)
				{
					m_pRunSprite->Animate();
				}
				else
				{
					// Note: AnimatedSprite doesn't have a Stop() method
					// This resets to first frame but doesn't actually stop animating
					m_pRunSprite->Restart();
				}
			}

			// Show the sprite's debug UI
			m_pRunSprite->DebugDraw();
		}
	}
}

Vector2&
Player::Position()
{
	return m_position;
}