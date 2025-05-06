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

// PLAYER CLASS NEEDS TO BE CLEANED UP!!!
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
	, m_pAttackSprite(0)
	, m_pFallSprite(0)
	, m_pRollSprite(0)
	, m_pJumpSprite(0)
	, m_pTurnAroundSprite(0)
	, m_targetFacingRight(0)
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

	// ----------------------------------Fall Animation--------------------------------
	m_pFallSprite = renderer.CreateAnimatedSprite("assets/player/_Fall.png");
	if (!m_pFallSprite)
	{
		LogManager::GetInstance().Log("Failed to create fall animation!");
		return false;
	}
	// Fall Config
	m_pFallSprite->SetupFrames(120, 80);
	m_pFallSprite->SetFrameDuration(0.2f);
	m_pFallSprite->SetLooping(false);

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
	m_pTurnAroundSprite->Animate();

	// Callback for when the animation completes
	m_pTurnAroundSprite->SetAnimationCompleteCallback([this]()
		{
			this->OnTurnAnimationComplete();
		});

	// ---------------------------------Roll Animation---------------------------------------
	m_pRollSprite = renderer.CreateAnimatedSprite("assets/player/_Roll.png");
	if (!m_pRollSprite)
	{
		LogManager::GetInstance().Log("Failed to create roll animation!");
		return false;
	}
	// Roll config
	m_pRollSprite->SetupFrames(120, 80);
	m_pRollSprite->SetFrameDuration(0.1f);
	m_pRollSprite->SetLooping(false);

	m_pRollSprite->SetAnimationCompleteCallback([this]()
	{
		this->RollAnimationComplete();
	});

	// ---------------------------------Death Animation---------------------------------------

	// ---------------------------------Hit Animation---------------------------------------

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
	m_position.x = renderer.GetHeight() / 2;
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

	// apply gravity if jumping or falling
	if (m_currentState == PlayerState::JUMPING || m_currentState == PlayerState::FALLING)
	{
		const float gravity = 100.0f;
		m_velocity.y += gravity * deltaTime;

		// check if the player has reached the peak of the jump height
		if (m_currentState == PlayerState::JUMPING && m_velocity.y > 0)
		{
			m_currentState = PlayerState::FALLING;
			if (m_pFallSprite)
			{
				m_pFallSprite->Restart();
				m_pFallSprite->Animate();
			}
		}

		// ground check
		const float groundLevel = 360.0f; 

		if (m_position.y + m_velocity.y * deltaTime >= groundLevel)
		{
			m_position.y = groundLevel;
			m_velocity.y = 0;

			// return to idle or to running
			if (m_velocity.x != 0)
			{
				m_currentState = PlayerState::RUNNING;
				if (m_pRunSprite)
				{
					m_pRunSprite->Restart();
					m_pRunSprite->Animate();
				}
			}
			else
			{
				m_currentState = PlayerState::IDLE;
				if (m_pIdleSprite)
				{
					m_pIdleSprite->Restart();
					m_pIdleSprite->Animate();
				}
			}
		}
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

	case PlayerState::ROLLING:
		if (m_pRollSprite)
		{
			m_pRollSprite->Process(deltaTime);
			m_pRollSprite->SetX(m_position.x);
			m_pRollSprite->SetY(m_position.y);
			m_pRollSprite->SetFlipHorizontal(!m_bFacingRight);
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

	// Load static image (THIS IS NEEDED OTHERWISE NOTHING WILL BE ON SCREEN)
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

	case PlayerState::JUMPING:
		if (m_pJumpSprite) m_pJumpSprite->Draw(renderer);
		break;

	case PlayerState::ATTACKING:
		if (m_pAttackSprite) m_pAttackSprite->Draw(renderer);
		break;

	case PlayerState::FALLING:
		if (m_pFallSprite) m_pFallSprite->Draw(renderer);
		break;

	case PlayerState::ROLLING:
		if (m_pRollSprite) m_pRollSprite->Draw(renderer);
		break;
	}
}

//-----------------------------------------------Player States---------------------------------------------------------
void
Player::MoveLeft(float amount)
{
	// player cant move if turning or attacking
	if (m_currentState == PlayerState::TURNING || 
		m_currentState == PlayerState::ATTACKING)
	{
		return;
	}

	// set velocity regardless of state
	if (!m_bFacingRight)
	{
		m_velocity.x = -amount;
	}

	// only change state if not already in a special state
	if (m_currentState != PlayerState::JUMPING &&
		m_currentState != PlayerState::FALLING &&
		m_currentState != PlayerState::ROLLING &&
		m_currentState != PlayerState::ATTACKING)
	{
		if (!m_bFacingRight)
		{
			m_currentState = PlayerState::RUNNING;
		}
		else
		{
			StartTurnAnimation(-amount, false);
		}
	}
}

void
Player::MoveRight(float amount)
{
	// player cant move if attacking or turning
	if (m_currentState == PlayerState::TURNING || 
		m_currentState == PlayerState::ATTACKING)
	{
		return;
	}

	// set velocity regardless of state
	if (m_bFacingRight)
	{
		m_velocity.x = amount;
	}

	// only change state if not already in a special state
	if (m_currentState != PlayerState::JUMPING &&
		m_currentState != PlayerState::FALLING &&
		m_currentState != PlayerState::ROLLING &&
		m_currentState != PlayerState::ATTACKING)
	{
		if (m_bFacingRight)
		{
			m_currentState = PlayerState::RUNNING;
		}
		else
		{
			StartTurnAnimation(amount, true);
		}
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
Player::RollAnimationComplete()
{
	if (m_currentState != PlayerState::ROLLING)
	{
		return;
	}

	// Transition
	if (m_rollVelocityBeforeRoll != 0)
	{
		m_currentState = PlayerState::RUNNING;

		// restore previous velocity
		m_velocity.x = m_rollVelocityBeforeRoll;

		// Start running animation
		if (m_pRunSprite)
		{
			m_pRunSprite->Restart();
			m_pRunSprite->Animate();
		}
	}
	else
	{
		m_currentState = PlayerState::IDLE;
		m_velocity.x = 0;

		// Start idle anim
		if (m_pIdleSprite)
		{
			m_pIdleSprite->Restart();
			m_pIdleSprite->Animate();
		}
	}

	m_rollVelocityBeforeRoll = 0;
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
	// player cant jump if already jumping, falling or turning
	if (m_currentState == PlayerState::JUMPING ||
		m_currentState == PlayerState::FALLING ||
		m_currentState == PlayerState::ROLLING ||
		m_currentState == PlayerState::TURNING)
	{
		return;
	}

	const float groundLevel = 360.0f;

	// only allow jumping if player is on the ground
	if (m_position.y >= groundLevel - 0.1f)
	{
		m_velocity.y = -amount;

		// state will then be changed to jumping
		m_currentState = PlayerState::JUMPING;

		if (m_pJumpSprite)
		{
			m_pJumpSprite->Restart();
			m_pJumpSprite->Animate();

			// set callback when the jump anim completes
			m_pJumpSprite->SetAnimationCompleteCallback([this]()
			{
				// if player still in jump anim
				if (this->m_currentState == PlayerState::JUMPING)
				{
					this->m_currentState = PlayerState::FALLING;
					if (this->m_pFallSprite)
					{
						this->m_pFallSprite->Restart();
						this->m_pFallSprite->Animate();
					}
				}
			});
		}
	}
}

void
Player::Fall(float amount)
{
	// Dont interrupt if already falling or turning
	if (m_currentState == PlayerState::FALLING ||
		m_currentState == PlayerState::TURNING)
	{
		return;
	}

	// Set velocity downward
	m_velocity.y = amount;

	// Change state to falling
	m_currentState = PlayerState::FALLING;

	// Reset and start the fall animation
	if (m_pFallSprite)
	{
		m_pFallSprite->Restart();
		m_pFallSprite->Animate();

		// Set a callbakc for when the anim completes
		m_pFallSprite->SetAnimationCompleteCallback([this]()
		{
			if (this->m_currentState == PlayerState::FALLING)
			{
				if (this->m_pFallSprite)
				{
						this->m_pFallSprite->Restart();
						this->m_pFallSprite->Animate();
				}
			}
		});
	}
}

void
Player::Attack(float amount)
{
	// player cant attack if already attacking, jumping, turning, running or falling
	if (m_currentState == PlayerState::ATTACKING ||
		m_currentState == PlayerState::TURNING ||
		m_currentState == PlayerState::JUMPING ||
		m_currentState == PlayerState::RUNNING ||
		m_currentState == PlayerState::ROLLING ||
		m_currentState == PlayerState::FALLING)
	{
		return;
	}

	// state will be changed to attacking
	m_currentState = PlayerState::ATTACKING;

	// reset and start the attack anim
	if (m_pAttackSprite)
	{
		m_pAttackSprite->Restart();
		m_pAttackSprite->SetLooping(false);
		m_pAttackSprite->Animate();

		// Set a callback when the animation completes
		m_pAttackSprite->SetAnimationCompleteCallback([this]()
		{
			// when anim ends, return to idle or running
			if (this->m_velocity.x != 0)
			{
				this->m_currentState = PlayerState::RUNNING;
			}
			else
			{
				this->m_currentState = PlayerState::IDLE;
			}

		});
	}
}

void
Player::TurnAround(float amount)
{
	m_currentState = PlayerState::TURNING;
}

void
Player::Roll(float amount)
{
	// if rolling these cant interuppt
	if (m_currentState == PlayerState::ROLLING ||
		m_currentState == PlayerState::TURNING ||
		m_currentState == PlayerState::JUMPING ||
		m_currentState == PlayerState::FALLING ||
		m_currentState == PlayerState::ATTACKING)
	{
		return;
	}

	// Speed burst
	m_rollVelocityBeforeRoll = m_velocity.x;

	// Apply boost in the facing direction
	float rollSpeedBoost = amount;

	// Determine direction based on facing or previous movement
	if (m_rollVelocityBeforeRoll != 0) 
	{
		// If already moving, use that direction
		rollSpeedBoost = m_bFacingRight ? amount : -amount;
	}
	else 
	{
		// If standing still, roll in the direction player is facing
		rollSpeedBoost = m_bFacingRight ? amount : -amount;
	}

	m_velocity.x = rollSpeedBoost;

	m_currentState = PlayerState::ROLLING;

	if (m_pRollSprite)
	{
		m_pRollSprite->Restart();
		m_pRollSprite->Animate();
	}
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