// This include
#include "Player.h"

// Local Includes
#include "Renderer.h"
#include "AnimatedSprite.h"
#include "LogManager.h"
#include "Texture.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <cassert>
#include <cstdlib>
#include <string>
#include <algorithm>

// Constants
const float PLAYER_VISUAL_SCALE = 2.0f;

Player::Player()
	: Entity()
	, m_pStaticSprite(nullptr)
	, m_currentState(PlayerState::IDLE)
	, m_bFacingRight(true)
	, m_desiredMoveSpeed(0.0f)
	, m_isTurning(false)
	, m_targetFacingRight(true)
	, m_rollVelocityBeforeRoll(0.0f)
	, m_maxStamina(100.0f)
	, m_currentStamina(100.0f)
	, m_staminaRegenRate(10.0f)
	, m_justRevived(false)
	, m_bAlive(false)
{
	SetMaxHealth(100, true);
	SetRadius((static_cast<float>(PLAYER_SPRITE_WIDTH) * PLAYER_VISUAL_SCALE) / 2.5f);
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
	if (!Entity::Initialise(renderer))
	{
		return false;
	}

	SetMaxHealth(100, true);
	m_bAlive = true;

	// Load static image
	m_pStaticSprite = renderer.CreateSprite("assets/player/_Idle.png");
	m_pStaticSprite->SetScale(PLAYER_VISUAL_SCALE, PLAYER_VISUAL_SCALE);

	// ---IDLE---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::IDLE, "assets/player/_Idle.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.15f, true))  return false;
	// ---RUNNING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::RUNNING, "assets/player/_Run.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.10f, true))  return false;
	// ---JUMPING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::JUMPING, "assets/player/_Jump.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.15f, false, [this]() { this->JumpAnimationComplete(); })) return false;
	// ---FALLING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::FALLING, "assets/player/_Fall.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.20f, false)) return false; // Typically looping or single very long frame
	// ---ATTACKING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::ATTACKING, "assets/player/_AttackComboNoMovement.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.08f, false, [this]() { this->AttackAnimationComplete(); })) return false;
	// ---TURNING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::TURNING, "assets/player/_TurnAround.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.1f, false, [this]() { this->TurnAnimationComplete(); })) return false;
	// ---ROLLING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::ROLLING, "assets/player/_Roll.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.1f, false, [this]() { this->RollAnimationComplete(); })) return false;
	// ---HURTING---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::HURT, "assets/player/_Hit.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.3f, false, [this]() { this->HurtAnimationComplete(); })) return false;
	// ---DEATH---
	if (!InitialiseAnimatedSprite(renderer, PlayerState::DEATH, "assets/player/_Death.png", PLAYER_SPRITE_WIDTH, PLAYER_SPRITE_HEIGHT, 0.15f, false, [this]() { this->DeathAnimationComplete(); })) return false;

	// Position to be center of screen
	m_position.x = static_cast<float>(renderer.GetWidth() / 2);
	m_position.y = kGroundLevel;

	// reset the stats
	m_currentStamina = m_maxStamina;

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

	sprite->SetScale(PLAYER_VISUAL_SCALE, PLAYER_VISUAL_SCALE);
	// Store the sprite in the map
	m_animatedSprites[state] = sprite;
	
	return true;
}

void Player::Process(float deltaTime)
{
	if (!m_bAlive)
	{
		if (m_currentState != PlayerState::DEATH) 
		{
			TransitionToState(PlayerState::DEATH);
		}
		// Process death animation if applicable
		AnimatedSprite* deathSprite = GetCurrentAnimatedSprite();
		if (deathSprite) 
		{
			UpdateSprite(deathSprite, deltaTime);
		}
		return; // Don't process movement/physics if dead
	}

	// Regen stam
	bool canRegenStamina = (m_currentState != PlayerState::ROLLING && m_currentState != PlayerState::ATTACKING && m_currentState != PlayerState::JUMPING);

	if (canRegenStamina && m_currentStamina < m_maxStamina)
	{
		m_currentStamina += m_staminaRegenRate * deltaTime;
		m_currentStamina = std::min(m_currentStamina, m_maxStamina);
	}

	// ---------------------------------Physics and state logic-----------------------------
	bool wasOnGround = (m_position.y >= kGroundLevel);

	// gravity will be applied if not on the ground or jumping
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
	
	// update the position based on player velocity
	bool canMoveHorizontally = (m_currentState != PlayerState::ATTACKING); // prevent moving while attacking

	if (canMoveHorizontally)
	{
		m_position.x += m_velocity.x * currentSpeedFactor * deltaTime;
	}

	m_position.y += m_velocity.y * deltaTime;

	// ground collision and landing
	bool isOnGround = (m_position.y >= kGroundLevel);

	if (isOnGround && !wasOnGround) // just landed
	{
		m_position.y = kGroundLevel;
		m_velocity.y = 0;
		//OnFallLand();
		if (!wasOnGround || m_currentState == PlayerState::FALLING || m_currentState == PlayerState::JUMPING)
		{
			FallOnLand();
		}
	}
	// if they player rolls off the edge
	else if (!isOnGround && wasOnGround && m_currentState != PlayerState::JUMPING && m_currentState != PlayerState::ROLLING)
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
	AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
	if (!m_bAlive && GetCurrentAnimatedSprite() && GetCurrentAnimatedSprite()->IsAnimationComplete()) return;

	// Load static image (THIS IS NEEDED OTHERWISE NOTHING WILL BE ON SCREEN)
	m_pStaticSprite->Draw(renderer);

	// draw the animated sprite for the current state
	if (currentSprite)
	{
		currentSprite->Draw(renderer);
	}
}

// Helper to get the sprite for the current state from the map
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
	if (m_currentState == PlayerState::HURT && !GetCurrentAnimatedSprite()->IsAnimationComplete()) return;

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
	if (m_currentState == PlayerState::ATTACKING || m_currentState == PlayerState::ROLLING || m_isTurning || m_currentState == PlayerState::HURT || m_currentState == PlayerState::DEATH)
	{
		return;
	}

	bool isOnGround = (m_position.y >= kGroundLevel);

	if (m_bFacingRight)
	{
		if (isOnGround) // Can only turn on the grouynd
		{
			// Need to turn first
			StartTurn(-amount, false); // Can obly turn when on the ground
		}
	}
	else
	{
		// Already facing left, start or continue running
		m_velocity.x = -amount;
		// Only transition to RUNNING if on the ground and not already jumping or falling
		if (isOnGround && m_currentState != PlayerState::JUMPING && m_currentState != PlayerState::FALLING)
		{
			TransitionToState(PlayerState::RUNNING);
		}
	}
}

void Player::MoveRight(float amount)
{
	// Cannot change direction or start moving while attacking, rolling, (or maybe jumping/falling depending on design)
	if (m_currentState == PlayerState::ATTACKING || m_currentState == PlayerState::ROLLING || m_isTurning || m_currentState == PlayerState::HURT || m_currentState == PlayerState::DEATH)
	{
		return;
	}

	bool isOnGround = (m_position.y >= kGroundLevel);

	if (!m_bFacingRight)
	{
		if (isOnGround)
		{
			StartTurn(amount, true); // Start turning right only on ground
		}
	}
	else
	{
		// Already facing right, start or continue running
		m_velocity.x = amount;
		// Only transition to RUNNING if on the ground
		if (isOnGround && m_currentState != PlayerState::JUMPING && m_currentState != PlayerState::FALLING)
		{
			TransitionToState(PlayerState::RUNNING);
		}
	}
}

void Player::StopMoving()
{
	if (m_currentState == PlayerState::RUNNING || m_currentState == PlayerState::IDLE)
	{
		if (abs(m_velocity.x) > 0.01f) // Only stop if actually moving
		{
			m_velocity.x = 0;
			TransitionToState(PlayerState::IDLE);
		}
	}
}

// Helper to initiate the turning sequence
void Player::StartTurn(float desiredSpeed, bool turnToRight)
{
	if (m_isTurning || m_currentState != PlayerState::RUNNING && m_currentState != PlayerState::IDLE) return; // Cant turn if already turning or not in movable ground state

	m_desiredMoveSpeed = desiredSpeed;
	m_targetFacingRight = turnToRight;
	// Velocity during turn is handled by process checking m_isTurning flag
	TransitionToState(PlayerState::TURNING);
}

void Player::Jump()
{
	// Can only jump from ground states (Idle, Running)
	if (m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING)
	{
		bool isOnGround = (m_position.y >= kGroundLevel - 0.1f);
		float staminaCost = 15.0f;

		if (isOnGround && UseStamina(staminaCost)) // Double check we are on ground
		{
			m_velocity.y = -kJumpForce;
			TransitionToState(PlayerState::JUMPING);
		}
		else if (isOnGround)
		{
			LogManager::GetInstance().Log("Not enought stamina!");
			// Show on the player bar no stamina
		}
	}
}

void Player::Attack()
{
	// Can only attack from ground states (Idle, Running)
	if (m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING)
	{
		float staminaCost = 10.0f;
		if (UseStamina(staminaCost))
		{
			m_velocity.x = 0;
			TransitionToState(PlayerState::ATTACKING);
		}
		else
		{
			LogManager::GetInstance().Log("Not enough stamina!");
			// Show on the stamina bar
		}
	}
}

void Player::Roll(float speedBoost)
{
	// Can only roll from ground states (Idle, Running)
	if (m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING)
	{
		float staminaCost = 30.0f;
		if (UseStamina(staminaCost))
		{
			m_rollVelocityBeforeRoll = m_velocity.x; // Store the current speed

			// Apply boost in the facing direction
			float rollSpeed = m_bFacingRight ? speedBoost : -speedBoost;
			m_velocity.x = rollSpeed;

			TransitionToState(PlayerState::ROLLING);
			// Add a invincibility during roll
		}
		else
		{
			LogManager::GetInstance().Log("Not enough stamina to roll!");
		}
	}
}

// --- Stat Modifiers ---
void Player::TakeDamage(int amount)
{
	if (!m_bAlive && amount < 0)
	{
		//Revive();
		// if have enough abyssal essence
	}
	else if (!m_bAlive) return;
	if (m_currentState == PlayerState::ROLLING) return;

	m_currentHealth -= amount;
	m_currentHealth = std::max(0, m_currentHealth);

	if (m_currentHealth <= 0)
	{
		m_bAlive = false;
		TransitionToState(PlayerState::DEATH); // add a revive feature as well. 
		LogManager::GetInstance().Log("Player Died!");

	}
	else
	{
		TransitionToState(PlayerState::HURT);
		// knockback for the player
		m_velocity.x = m_bFacingRight ? -50.0f : 50.0f;
		m_velocity.y = -50.0f; // Small pop-up
	}
}

void Player::Revive()
{
	if (!m_bAlive && m_currentState == PlayerState::DEATH)
	{
		m_bAlive = true;
		SetMaxHealth(GetMaxHealth(), true); 
		m_currentStamina = m_maxStamina;
		m_justRevived = true;
		m_position.y = kGroundLevel; // makes sure the player is on ground
		m_velocity.Set(0.0f, 0.0f);
		TransitionToState(PlayerState::IDLE);
		LogManager::GetInstance().Log("Player Revived!");
	}
}

bool Player::UseStamina(float amount)
{
	if (m_currentStamina >= amount)
	{
		m_currentStamina -= amount;
		m_currentStamina = std::max(0.0f, m_currentStamina);
		return true;
	}
	return false;
}


// Animation Completion Handlers
void Player::TurnAnimationComplete()
{
	if (m_currentState == PlayerState::TURNING) // Ensure we were actually turning
	{
		m_bFacingRight = m_targetFacingRight; // Update facing direction
		m_velocity.x = m_desiredMoveSpeed;    // Apply desired speed

		// Decide next state based on whether the player intended to move
		if (abs(m_desiredMoveSpeed) > 0.01f)
		{
			TransitionToState(PlayerState::RUNNING);
		}
		else
		{
			TransitionToState(PlayerState::IDLE); // Should ideally not happen if turn is only triggered by move keys
		}
	}
	m_isTurning = false; // Ensure flag is reset even if state was wrong
	m_desiredMoveSpeed = 0;
}

void Player::RollAnimationComplete()
{
	if (m_currentState == PlayerState::ROLLING)
	{
		// Transition back to Idle or Running based on pre-roll speed
		m_velocity.x = m_rollVelocityBeforeRoll;

		if (abs(m_rollVelocityBeforeRoll) > 0.01f)
		{
			TransitionToState(PlayerState::RUNNING);
		}
		else
		{
			TransitionToState(PlayerState::IDLE);
		}
		m_rollVelocityBeforeRoll = 0; // Reset temp variable
	}
}

void Player::AttackAnimationComplete()
{
	if (m_currentState == PlayerState::ATTACKING)
	{
		m_velocity.x = 0; // Ensure stopped if attack stops movement
		TransitionToState(PlayerState::IDLE);
	}
}

void Player::JumpAnimationComplete()
{
	// Jump animation finishes mid-air, transition to falling state
	if (m_currentState == PlayerState::JUMPING)
	{
		TransitionToState(PlayerState::FALLING);
	}
}

void Player::FallOnLand()
{
	// Called when player hits the ground (from Process)
	if (m_currentState == PlayerState::FALLING || m_currentState == PlayerState::JUMPING)
	{
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

void Player::HurtAnimationComplete() 
{
	if (m_currentState == PlayerState::HURT) 
	{
		// Transition back to idle or falling depending on whether still airborne
		bool isOnGround = (m_position.y >= kGroundLevel);
		if (isOnGround) 
		{
			TransitionToState(PlayerState::IDLE);
		} else 
		{
			TransitionToState(PlayerState::FALLING);
		}
	}
}

void Player::DeathAnimationComplete() 
{
	LogManager::GetInstance().Log("Death animation complete.");
	// add a option if player wants to revive
	// [Arise] / [Perish]
}

// ------------------------------------------Debugging-------------------------------------------------------
// Need to add cheat features for debug
// God Mode
// One Shot
// Inf Stamina
void Player::DebugDraw()
{
	if (ImGui::CollapsingHeader("Player Debug"))
	{
		ImGui::Checkbox("Alive", &m_bAlive);
		ImGui::Text("Position: (%.1f, %.1f)", m_position.x, m_position.y);
		ImGui::Text("Velocity: (%.1f, %.1f)", m_velocity.x, m_velocity.y);

		// Display current state as text
		std::string stateName = "UNKNOWN";
		switch (m_currentState) {
		case PlayerState::IDLE: stateName = "IDLE"; break;
		case PlayerState::RUNNING: stateName = "RUNNING"; break;
		case PlayerState::JUMPING: stateName = "JUMPING"; break;
		case PlayerState::FALLING: stateName = "FALLING"; break;
		case PlayerState::ATTACKING: stateName = "ATTACKING"; break;
		case PlayerState::TURNING: stateName = "TURNING"; break;
		case PlayerState::ROLLING: stateName = "ROLLING"; break;
		case PlayerState::HURT: stateName = "HURT"; break;
		case PlayerState::DEATH: stateName = "DEATH"; break;
		default: stateName = "(" + std::to_string((int)m_currentState) + ")"; break;
		}
		ImGui::Text("State: %s", stateName.c_str());
		ImGui::Text("Facing: %s", m_bFacingRight ? "Right" : "Left");
		ImGui::Text("Turning: %s (Target: %s, Desired Spd: %.1f)",
			m_isTurning ? "Yes" : "No",
			m_targetFacingRight ? "Right" : "Left",
			m_desiredMoveSpeed);
		ImGui::Text("On Ground: %s", (m_position.y >= kGroundLevel) ? "Yes" : "No");

		// Stats Display/Edit
		ImGui::SliderInt("Health", &m_currentHealth, 0, m_maxHealth);
		ImGui::SliderFloat("Stamina", &m_currentStamina, 0.0f, m_maxStamina);
		if (ImGui::Button("Damage Player (15)")) { TakeDamage(15); }
		ImGui::SameLine();
		if (ImGui::Button("Kill Player")) { TakeDamage(m_maxHealth * 2); }
		if (ImGui::Button("Revive Player")) { Revive(); }
		// if if (ImGui::Button("God Mode")) { GodMode(); }
		// if (ImGui::Button("One Shot")) { OneShotMode(); }
		// if (ImGui::Button("Infinite Stamina")) { InfStamina(); }

		AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
		if (currentSprite)
		{
			ImGui::Separator();
			ImGui::Text("Current Sprite: %p", currentSprite);
			ImGui::Text(" Sprite Dims: %d x %d", currentSprite->GetWidth(), currentSprite->GetHeight());
			ImGui::Text(" Animating: %s", currentSprite->IsAnimating() ? "Yes" : "No");
			ImGui::Text(" Looping: %s", currentSprite->IsLooping() ? "Yes" : "No");
			ImGui::Text(" Flipped: %s", currentSprite->IsFlippedHorizontal() ? "Yes" : "No");
			ImGui::Text(" Anim Complete Flag: %s", currentSprite->IsAnimationComplete() ? "Yes" : "No");

			currentSprite->DebugDraw(); // Includes frame slider
		}
		else
		{
			ImGui::Text("Current Sprite: None");
		}
	}
}