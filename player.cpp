// This include
#include "Player.h"

// Local Includes
#include "Renderer.h"
#include "AnimatedSprite.h"
#include "InlineHelpers.h"
#include "LogManager.h"
#include "Texture.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <cassert>
#include <cstdlib>

// Consttants
const int PLAYER_SPRITE_WIDTH = 120;
const int PLAYER_SPRITE_HEIGHT = 80;

Player::Player()
	: m_bAlive(true)
	, m_pStaticSprite(nullptr)
	, m_currentState(PlayerState::IDLE)
	, m_bFacingRight(true)
	, m_desiredMoveSpeed(0.0f)
	, m_isTurning(false)
	, m_targetFacingRight(true)
	, m_rollVelocityBeforeRoll(0.0f)
{
	m_velocity.Set(0.0f, 0.0f);
}

Player::~Player()
{
	delete m_pStaticSprite;
	m_pStaticSprite = nullptr;

	for (const auto& pair : m_animatedSprites)
	{
		delete pair.second;
	}

	m_animatedSprites.clear();
}

bool Player::Initialise(Renderer& renderer)
{
	// Load static image
	m_pStaticSprite = renderer.CreateSprite("assets/player/_Idle.png");

	// Animated Sprites
	// ---IDLE---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::IDLE, "assets/player/_Idle.png", 120, 80, 0.15f, true))  return false;
	// ---RUNNING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::RUNNING, "assets/player/_Run.png", 120, 80, 0.10f, true))  return false;
	// ---JUMPING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::JUMPING, "assets/player/_Jump.png", 120, 80, 0.15f, false, [this]() { this->OnJumpAnimationComplete(); })) return false;
	// ---FALLING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::FALLING, "assets/player/_Fall.png", 120, 80, 0.20f, false)) return false;
	// ---ATTACKING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::ATTACKING, "assets/player/_AttackComboNoMovement.png", 120, 80, 0.08f, false, [this]() { this->OnAttackAnimationComplete(); })) return false;
	// ---TURNING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::TURNING, "assets/player/_TurnAround.png", 120, 80, 0.1f, false, [this]() { this->OnTurnAnimationComplete(); })) return false;
	// ---ROLLING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::ROLLING, "assets/player/_Roll.png", 120, 80, 0.1f, false, [this]() { this->OnRollAnimationComplete(); })) return false;
	// ---HURTING---

	// ---DEATH---

	// Initial position
	// Position to be center of screen
	m_position.x = renderer.GetHeight() / 2;
	m_position.y = kGroundLevel;

	// Set the initial state
	TransitionToState(PlayerState::IDLE);

	return true;
}

// Helper method
bool Player::InitialiseAnimatedSprite(Renderer& renderer, PlayerState state, const char* pcFilename, 
	int frameWidth, int frameHeight, float frameDuration, bool loop, AnimationCallBack onComplete)
{
	AnimatedSprite* sprite = renderer.CreateAnimatedSprite(pcFilename);
	if (!sprite)
	{
		LogManager::GetInstance().Log(("Failed to create animated sprite: " + std::string(pcFilename)).c_str());
		return false;
	}

	sprite->SetupFrames(frameWidth, frameHeight);
	sprite->SetFrameDuration(frameDuration);
	sprite->SetLooping(loop);
	if (onComplete)
	{
		sprite->SetAnimationCompleteCallback(onComplete);
	}

	// Store the sprite in the map
	m_animatedSprites[state] = sprite;
	LogManager::GetInstance().Log(("Initialised sprite for state " + std::to_string((int)state) + ": " + std::string(pcFilename)).c_str());

	return true;
}

void Player::Process(float deltaTime)
{
	if (!m_bAlive)
	{
		return;
	}

	// Physics and state logic
	bool wasOnGround = (m_position.y >= kGroundLevel);

	if (m_currentState == PlayerState::JUMPING || m_currentState == PlayerState::FALLING || !wasOnGround)
	{
		m_velocity.y += kGravity * deltaTime;

		// transition from jumping to falling
		if (m_currentState == PlayerState::JUMPING && m_velocity.y > 0)
		{
			TransitionToState(PlayerState::FALLING);
		}
	}

	// Slodw donw movement if turning
	float currentSpeedFactor = m_isTurning ? kTurnSpeedFactor : 1.0f;
	m_position.x += m_velocity.x * currentSpeedFactor * deltaTime;
	
	m_position.y += m_velocity.y * deltaTime;

	// ground collision and landing
	bool isOnGround = (m_position.y >= kGroundLevel);

	if (isOnGround && !wasOnGround) // just landed
	{
		m_position.y = kGroundLevel;
		m_velocity.y = 0;
		OnFallLand();
	}
	else if (isOnGround && (m_currentState == PlayerState::JUMPING || m_currentState == PlayerState::FALLING))
	{
		m_position.y = kGroundLevel;
		m_velocity.y = 0;
		OnFallLand();
	}
	else if (!isOnGround && wasOnGround && m_currentState != PlayerState::JUMPING)
	{
		TransitionToState(PlayerState::FALLING);
	}

	// Process the current animation
	AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
	if (currentSprite)
	{
		UpdateSprite(currentSprite, deltaTime);
	}
	else
	{
		LogManager::GetInstance().Log(("Error: No Sprite found for state: " + std::to_string((int)m_currentState)).c_str());
	}
}

void Player::UpdateSprite(AnimatedSprite* sprite, float deltaTime)
{
	sprite->Process(deltaTime);
	sprite->SetX(static_cast<int>(m_position.x));
	sprite->SetY(static_cast<int>(m_position.y));
	sprite->SetFlipHorizontal(!m_bFacingRight);
}

// ---Drawing---

void Player::Draw(Renderer& renderer)
{
	if (!m_bAlive)
	{
		return;
	}

	// Load static image (THIS IS NEEDED OTHERWISE NOTHING WILL BE ON SCREEN)
	m_pStaticSprite->Draw(renderer);

	// draw the animated sprite for the current state
	AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
	if (currentSprite)
	{
		currentSprite->Draw(renderer);
	}
}

// helper to get the sprite for the current state from the map
AnimatedSprite* Player::GetCurrentAnimatedSprite()
{
	auto it = m_animatedSprites.find(m_currentState);
	if (it != m_animatedSprites.end())
	{
		return it->second; // return the found sprite pointer
	}
	return nullptr;
}

// State transitions
void Player::TransitionToState(PlayerState newState)
{
	if (m_currentState == newState && newState != PlayerState::FALLING && newState != PlayerState::JUMPING)
	{
		AnimatedSprite* sprite = GetCurrentAnimatedSprite();
		if (sprite && !sprite->IsAnimating() && sprite->IsLooping())
		{
			sprite->Animate();
		}

		return;
	}

	m_currentState = newState;
	m_isTurning = (newState == PlayerState::TURNING);

	AnimatedSprite* newSprite = GetCurrentAnimatedSprite();
	if (newSprite)
	{
		LogManager::GetInstance().Log(("Transitioning to state: " + std::to_string((int)newState)).c_str());
		newSprite->Restart();
		newSprite->Animate();
	}
	else
	{
		LogManager::GetInstance().Log(("Error: No sprite found for state transitioning: " + std::to_string((int)newState)).c_str());
	}
}

//-----------------------------------------------Player Action---------------------------------------------------------
void Player::MoveLeft(float amount)
{
	// Cannot change direction or start moving while attacking, rolling, (or maybe jumping/falling depending on design)
	if (m_currentState == PlayerState::ATTACKING || m_currentState == PlayerState::ROLLING || m_isTurning )
	{
		return;
	}

	if (m_bFacingRight)
	{
		if (m_position.y >= kGroundLevel)
		{
			// Need to turn first
			StartTurn(-amount, false); // can obly turn when on the ground
		}
	}
	else
	{
		// Already facing left, start or continue running
		m_velocity.x = -amount;
		// Only transition to RUNNING if on the ground
		if (m_position.y >= kGroundLevel)
		{
			TransitionToState(PlayerState::RUNNING);
		}
	}
}

void Player::MoveRight(float amount)
{
	// Cannot change direction or start moving while attacking, rolling, (or maybe jumping/falling depending on design)
	if (m_currentState == PlayerState::ATTACKING || m_currentState == PlayerState::ROLLING || m_isTurning)
	{
		return;
	}

	if (!m_bFacingRight)
	{
		if (m_position.y >= kGroundLevel)
		{
			StartTurn(amount, true); // Start turning right only on ground
		}
	}
	else
	{
		// Already facing right, start or continue running
		m_velocity.x = amount;
		// Only transition to RUNNING if on the ground
		if (m_position.y >= kGroundLevel)
		{
			TransitionToState(PlayerState::RUNNING);
		}
	}
}

void Player::StopMoving()
{
	// Can stop moving if running or idle (or maybe after roll/attack completes)
	// Don't interrupt actions like attack, roll, turn, jump, fall
	if (m_currentState == PlayerState::RUNNING || m_currentState == PlayerState::IDLE)
	{
		if (m_velocity.x != 0) // Only transition if actually moving
		{
			m_velocity.x = 0;
			TransitionToState(PlayerState::IDLE);
		}
	}
	// If stopping after a roll/attack, that logic is handled in their respective OnComplete handlers
}

// Helper to initiate the turning sequence
void Player::StartTurn(float desiredSpeed, bool turnToRight)
{
	if (m_isTurning) return; // Already turning

	m_desiredMoveSpeed = desiredSpeed;
	m_targetFacingRight = turnToRight;
	// Velocity during turn is handled by process checking m_isTurning flag
	TransitionToState(PlayerState::TURNING);
}

void Player::Jump(float amount)
{
	// Can only jump from ground states (Idle, Running)
	if (m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING)
	{
		bool isOnGround = (m_position.y >= kGroundLevel);
		if (isOnGround) // Double check we are on ground
		{
			m_velocity.y = -kJumpForce; // Apply upward force
			TransitionToState(PlayerState::JUMPING);
		}
	}
}

void Player::Attack()
{
	// Can only attack from ground states (Idle, Running)
	// Or maybe allow air attacks later? Add checks for JUMPING/FALLING if so.
	if (m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING)
	{
		// Stop horizontal movement during attack
		m_velocity.x = 0; // Or allow movement if desired, remove this line
		TransitionToState(PlayerState::ATTACKING);
	}
}

void Player::Roll(float speedBoost)
{
	// Can only roll from ground states (Idle, Running)
	if (m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING)
	{
		m_rollVelocityBeforeRoll = m_velocity.x; // Store current speed

		// Apply boost in the facing direction
		float rollSpeed = m_bFacingRight ? speedBoost : -speedBoost;
		m_velocity.x = rollSpeed;

		TransitionToState(PlayerState::ROLLING);
	}
}

// Animation Completion Handlers
void Player::OnTurnAnimationComplete()
{
	if (m_currentState == PlayerState::TURNING) // Ensure we were actually turning
	{
		m_bFacingRight = m_targetFacingRight; // Update facing direction
		m_velocity.x = m_desiredMoveSpeed;    // Apply desired speed

		// Decide next state based on whether the player intended to move
		if (m_desiredMoveSpeed != 0)
		{
			TransitionToState(PlayerState::RUNNING);
		}
		else
		{
			TransitionToState(PlayerState::IDLE); // Should ideally not happen if turn is only triggered by move keys
		}
	}
	m_isTurning = false; // Ensure flag is reset even if state was wrong
}

void Player::OnRollAnimationComplete()
{
	if (m_currentState == PlayerState::ROLLING)
	{
		// Transition back to Idle or Running based on pre-roll speed
		if (m_rollVelocityBeforeRoll != 0)
		{
			m_velocity.x = m_rollVelocityBeforeRoll; // Restore previous speed
			TransitionToState(PlayerState::RUNNING);
		}
		else
		{
			m_velocity.x = 0; // Stop
			TransitionToState(PlayerState::IDLE);
		}
		m_rollVelocityBeforeRoll = 0; // Reset temp variable
	}
}

void Player::OnAttackAnimationComplete()
{
	if (m_currentState == PlayerState::ATTACKING)
	{
		// Transition back to Idle or Running (check velocity if attack allowed movement)
		// Assuming attack stops movement for now:
		// Check if movement keys are held down? Or just go to IDLE?
		// Let's default to IDLE, input handling in Scene can override next frame
		m_velocity.x = 0; // Ensure stopped if attack stops movement
		TransitionToState(PlayerState::IDLE);
	}
}

void Player::OnJumpAnimationComplete()
{
	// Jump animation finishes mid-air, transition to falling state
	if (m_currentState == PlayerState::JUMPING)
	{
		TransitionToState(PlayerState::FALLING);
	}
}

void Player::OnFallLand()
{
	// Called when player hits the ground (from Process)
	if (m_currentState == PlayerState::FALLING || m_currentState == PlayerState::JUMPING)
	{
		// Decide whether to land in IDLE or RUNNING state based on horizontal velocity
		if (abs(m_velocity.x) > 0.1f) // Check if there's significant horizontal speed
		{
			TransitionToState(PlayerState::RUNNING);
		}
		else
		{
			m_velocity.x = 0; // Ensure stopped
			TransitionToState(PlayerState::IDLE);
		}
	}
}

// ------------------------------------------Debugging-------------------------------------------------------
void Player::DebugDraw()
{
	if (ImGui::CollapsingHeader("Player"))
	{
		ImGui::Checkbox("Alive", &m_bAlive);
		ImGui::Text("Position: (%.1f, %.1f)", m_position.x, m_position.y);
		ImGui::Text("Velocity: (%.1f, %.1f)", m_velocity.x, m_velocity.y);
		ImGui::Text("State: %d", static_cast<int>(m_currentState)); // Show state enum value
		ImGui::Text("Facing: %s", m_bFacingRight ? "Right" : "Left");
		ImGui::Text("Turning: %s", m_isTurning ? "Yes" : "No");
		ImGui::Text("On Ground: %s", (m_position.y >= kGroundLevel) ? "Yes" : "No");

		AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
		if (currentSprite)
		{
			ImGui::Text("Current Sprite Addr: %p", currentSprite);
			ImGui::Text("Sprite Dims: %d x %d", currentSprite->GetWidth(), currentSprite->GetHeight());
			ImGui::Text("Animating: %s", currentSprite->IsAnimating() ? "Yes" : "No");
			ImGui::Text("Looping: %s", currentSprite->IsLooping() ? "Yes" : "No"); // Added looping info
			ImGui::Text("Flipped: %s", currentSprite->IsFlippedHorizontal() ? "Yes" : "No");
			ImGui::Text("Anim Complete Flag: %s", currentSprite->IsAnimationComplete() ? "Yes" : "No"); // Added complete flag info

			// Show the sprite's specific debug UI
			currentSprite->DebugDraw();
		}
		else
		{
			ImGui::Text("Current Sprite: None");
		}
	}
}