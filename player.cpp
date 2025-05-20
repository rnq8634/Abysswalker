// This include
#include "Player.h"

// Local Includes
#include "Renderer.h"
#include "AnimatedSprite.h"
#include "LogManager.h"
#include "Texture.h"
#include "Game.h"

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
	, m_currentStamina(0.0f)
	, m_justRevived(false)
	, m_invincibilityTimer(0.0f)
	, m_bIsInvincible(false)
	, m_healthRegenFractionAccumulator(0.0f)
{
	m_velocity.Set(0.0f, 0.0f);
	ResetForNewGame();
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

void Player::ResetForNewGame()
{
	LogManager::GetInstance().Log("Game has resetted!!");

	m_bAlive = true;
	m_currentState = PlayerState::IDLE;
	m_playerStats.ResetStats();
	m_abyssalEssence = AbyssalEssence(); 
	UpdateStatsFromPlayerStats();

	m_healthRegenFractionAccumulator = 0.0f;

	m_currentStamina = m_playerStats.GetMaxStamina();
	m_currentHealth = m_playerStats.GetMaxHealth();

	m_justRevived = false;
	m_velocity.Set(0.0f, 0.0f);                   
	m_invincibilityTimer = 0.0f;
	m_bIsInvincible = false;
	
	ClearHitEntitiesList();
}

bool Player::Initialise(Renderer& renderer)
{
	if (!Entity::Initialise(renderer))
	{
		return false;
	}

	SetMaxHealth(m_playerStats.GetMaxHealth(), true);
	m_currentStamina = m_playerStats.GetMaxStamina();
	m_bAlive = true;

	SetRadius((static_cast<float>(PLAYER_SPRITE_WIDTH) * PLAYER_VISUAL_SCALE) / 2.5f);

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

void Player::UpdateStatsFromPlayerStats()
{
	SetMaxHealth(m_playerStats.GetMaxHealth(), false);
}

void Player::Process(float deltaTime)
{
	if (!m_bAlive && m_currentState == PlayerState::DEATH)
	{
		// Process death animation if applicable
		AnimatedSprite* deathSprite = GetCurrentAnimatedSprite();
		if (deathSprite) 
		{
			UpdateSprite(deathSprite, deltaTime);
		}
		return; // Don't process movement/physics if dead
	}
	else if (!m_bAlive)
	{
		TransitionToState(PlayerState::DEATH);
		return;
	}

	// Handle the I-Frame duration
	if (m_invincibilityTimer > 0.0f)
	{
		m_invincibilityTimer -= deltaTime;
		if (m_invincibilityTimer < 0.0f)
		{
			m_invincibilityTimer = 0.0f;
			m_bIsInvincible = false;
		}
	}

	ApplyHealthRegen(deltaTime);

	// Regen stam
	bool canRegenStamina = (m_currentState != PlayerState::ROLLING && m_currentState != PlayerState::ATTACKING && m_currentState != PlayerState::JUMPING);

	if (canRegenStamina && m_currentStamina < m_playerStats.GetMaxStamina())
	{
		m_currentStamina += m_playerStats.GetStaminaRegenRate() * deltaTime;
		m_currentStamina = std::min(m_currentStamina, m_playerStats.GetMaxStamina());
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

	if (!m_bAlive && m_currentState == PlayerState::DEATH && GetCurrentAnimatedSprite() && GetCurrentAnimatedSprite()->IsAnimationComplete())
	{
		if (m_abyssalEssence.CanRevive())
		{
			// Have a UI that asks if player wants to be revived
		}
	}
}

void Player::ApplyHealthRegen(float deltaTime)
{
	// Early exit if not alive or already at max health
	if (!m_bAlive || m_currentHealth >= m_playerStats.GetMaxHealth()) 
	{
		m_healthRegenFractionAccumulator = 0.0f;
		return;
	}

	float currentRegenRate = m_playerStats.GetHealthRegenRate();

	// Early exit if there's no regen rate
	if (currentRegenRate <= 0.0f) 
	{
		m_healthRegenFractionAccumulator = 0.0f;
		return;
	}

	m_healthRegenFractionAccumulator += currentRegenRate * deltaTime;

	if (m_healthRegenFractionAccumulator >= 1.0f) {
		int wholePointsToRegen = static_cast<int>(m_healthRegenFractionAccumulator);

		m_currentHealth += wholePointsToRegen;
		m_currentHealth = std::min(m_currentHealth, m_playerStats.GetMaxHealth()); // Caps at max health

		m_healthRegenFractionAccumulator -= static_cast<float>(wholePointsToRegen);
	}
}

void Player::Revive()
{
	// Revival will only be allowed if player is dead, in state of dead and also have enough essence
	if (!m_bAlive && m_currentState == PlayerState::DEATH)
	{
		if (m_abyssalEssence.SpendForRevive())
		{
			m_bAlive = true;
			UpdateStatsFromPlayerStats();
			SetMaxHealth(m_playerStats.GetMaxHealth(), true);
			m_currentStamina = m_playerStats.GetMaxStamina();
			m_justRevived = true;
			m_position.y = kGroundLevel;
			m_velocity.Set(0.0f, 0.0f);
			TransitionToState(PlayerState::IDLE);
			LogManager::GetInstance().Log("Player Revived using Abyssal Essence!");

			// Grant invincibility upon revival
			m_bIsInvincible = true;
			m_invincibilityTimer = m_invincibilityDuration * 5.0f; // 5 seconds of invincibility
		}
		else
		{
			LogManager::GetInstance().Log("Player tried to revive but not enough Abyssal Essence.");
		}
	}
	else if (m_bAlive)
	{
		LogManager::GetInstance().Log("Player is already alive, cannot revive.");
	}
	else if (m_currentState != PlayerState::DEATH)
	{
		LogManager::GetInstance().Log("Player is not in DEATH state, cannot revive yet.");
	}
}

void Player::GainEssence(int amount)
{
	m_abyssalEssence.AddEssence(amount);
	LogManager::GetInstance().Log(("Player gained " + std::to_string(amount) + " essence. Total: " + std::to_string(m_abyssalEssence.GetCurrentAmount())).c_str());
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

	if (m_bIsInvincible)
	{
		float flashFrequency = 8.0f;
		if (static_cast<int>(m_invincibilityTimer * flashFrequency) % 2 == 0)
		{
			m_pStaticSprite->SetAlpha(128);
			m_pStaticSprite->Draw(renderer);

			if (currentSprite)
			{
				currentSprite->SetAlpha(128);
				currentSprite->Draw(renderer);
			}

			// Reset the alpha for the next frame
			m_pStaticSprite->SetAlpha(255);
			if (currentSprite)
			{
				currentSprite->SetAlpha(255);
			}
		}
		else
		{
			m_pStaticSprite->Draw(renderer);
			if (currentSprite)
			{
				currentSprite->Draw(renderer);
			}
		}
	}
	else
	{
		m_pStaticSprite->Draw(renderer);
		if (currentSprite)
		{
			currentSprite->Draw(renderer);
		}
	}
}

// Helper to get the sprite for the current state from the map
AnimatedSprite* Player::GetCurrentAnimatedSprite()
{
	auto it = m_animatedSprites.find(m_currentState);
	if (it != m_animatedSprites.end())
	{
		return it->second;
	}
	return nullptr;
}

// State transitions
void Player::TransitionToState(PlayerState newState)
{
	if (m_currentState == PlayerState::DEATH && newState != PlayerState::DEATH)
	{
		if (!m_bAlive) return;
	}

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
		float staminaCost = 10.0f;

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
		float staminaCost = 5.0f;
		if (UseStamina(staminaCost))
		{
			m_velocity.x = 0;

			ClearHitEntitiesList();

			TransitionToState(PlayerState::ATTACKING);
		}
		else
		{
			LogManager::GetInstance().Log("Not enough stamina!");
			// Stamina bar decreases
		}
	}
}

void Player::Roll(float speedBoost)
{
	// Can only roll from ground states (Idle, Running)
	if (m_currentState == PlayerState::IDLE || m_currentState == PlayerState::RUNNING)
	{
		float staminaCost = 15.0f;
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
	if (Game::s_bGodMode)
	{
		LogManager::GetInstance().Log("Player took no dmg since in god mode");
		return;
	}

	if (!m_bAlive) return;
	if (m_currentState == PlayerState::ROLLING || m_bIsInvincible) return;

	Entity::TakeDamage(amount);

	if (!m_bAlive)
	{
		TransitionToState(PlayerState::DEATH);
	}
	else
	{
		TransitionToState(PlayerState::HURT);
		// knockback for the player
		m_velocity.x = m_bFacingRight ? -50.0f : 50.0f;
		m_velocity.y = -50.0f;
		m_bIsInvincible = true;
		m_invincibilityTimer = m_invincibilityDuration;

	}
}

bool Player::UseStamina(float amount)
{
	if (Game::s_bInfiniteStaminaMode)
	{
		return true;
	}

	if (m_currentStamina >= amount)
	{
		m_currentStamina -= amount;
		m_currentStamina = std::max(0.0f, m_currentStamina);
		return true;
	}
	return false;
}

int Player::GetAttackDamage() const
{
	if (Game::s_bOneShotMode)
	{
		return 99999;
	}

	return m_playerStats.GetAttackDamage();
}

// Animation Completion Handlers
void Player::TurnAnimationComplete()
{
	if (m_currentState == PlayerState::TURNING)
	{
		m_bFacingRight = m_targetFacingRight;
		m_velocity.x = m_desiredMoveSpeed;

		if (abs(m_desiredMoveSpeed) > 0.01f)
		{
			TransitionToState(PlayerState::RUNNING);
		}
		else
		{
			TransitionToState(PlayerState::IDLE);
		}
	}
	m_isTurning = false;
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
		ClearHitEntitiesList();
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
	if (m_currentState == PlayerState::FALLING || m_currentState == PlayerState::JUMPING)
	{
		// Short I-frames on landing from a high fall or after being hit.
		if (m_velocity.y > kJumpForce * 0.5f)
		{
			m_bIsInvincible = true;
			m_invincibilityTimer = m_invincibilityDuration * 0.5f;
		}

		if (abs(m_velocity.x) > 0.1f) TransitionToState(PlayerState::RUNNING);
		else
		{
			m_velocity.x = 0;
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
		} 
		else 
		{
			TransitionToState(PlayerState::FALLING);
		}
	}
}

void Player::DeathAnimationComplete() 
{
	LogManager::GetInstance().Log("Death animation complete.");
	if (m_abyssalEssence.CanRevive())
	{
		LogManager::GetInstance().Log(("Player can revive. Cost: " + std::to_string(AbyssalEssence::DEFAULT_REVIVE_COST) + " essence.").c_str());
		// Add this prompt here,maybe above the player, the camera zooms in abit and blurs all sprites around the playuer
	}
	else
	{
		LogManager::GetInstance().Log("Player cannot revive (not enough essence). Game Over or return to menu.");
	}
}

bool Player::DamageDoneToTarget(Entity* target)
{
	if (m_currentState != PlayerState::ATTACKING || !target) return false;

	return m_hitEntitiesThisAttack.insert(target).second;
}

void Player::ClearHitEntitiesList()
{
	m_hitEntitiesThisAttack.clear();
}

// ------------------------------------------Debugging-------------------------------------------------------
// Need to add cheat features for debug
// God Mode
// One Shot
// Inf Stamina
void Player::DebugDraw()
{
	if (ImGui::CollapsingHeader("Player Debug##Player"))
	{
		ImGui::Checkbox("Alive", &m_bAlive);
		ImGui::Text("Health: %d / %d", m_currentHealth, m_playerStats.GetMaxHealth());
		ImGui::Text("Stamina: %.1f / %.1f", m_currentStamina, m_playerStats.GetMaxStamina());
		ImGui::Text("Position: (%.1f, %.1f)", m_position.x, m_position.y);
		ImGui::Text("Velocity: (%.1f, %.1f)", m_velocity.x, m_velocity.y);
		ImGui::Text("Invincible: %s (%.1f seconds)", m_bIsInvincible ? "Yes" : "No", m_invincibilityTimer);

		ImGui::Separator();
		m_abyssalEssence.DebugDraw();
		m_playerStats.DebugDraw();

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
		ImGui::Text("On Ground: %s", (m_position.y >= kGroundLevel) ? "Yes" : "No");

		if (ImGui::Button("Damage Player (15)")) { TakeDamage(15); }
		ImGui::SameLine();
		if (ImGui::Button("Kill Player")) { TakeDamage(m_playerStats.GetMaxHealth() * 2); }
		if (ImGui::Button("Revive Player")) { Revive(); }

		AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
		if (currentSprite)
		{
			ImGui::Separator();
			ImGui::Text("Current Sprite Details:");
			currentSprite->DebugDraw();
		}
		else ImGui::Text("Current Sprite: None");
	}
}

bool Player::CheckCollision(const Entity& other) const
{
	if (!IsAlive() || !other.IsAlive()) return false;

	Vector2 playerPos = GetPosition();
	Vector2 otherPos = other.GetPosition();

	// box collision using sprite dimensions
	float playerHalfWidth = PLAYER_SPRITE_WIDTH / 2.0f;
	float playerHalfHeight = PLAYER_SPRITE_HEIGHT / 2.0f;
	float otherRadius = other.GetRadius();

	float playerLeft = playerPos.x - playerHalfWidth;
	float playerRight = playerPos.x + playerHalfWidth;
	float playerTop = playerPos.y - playerHalfHeight;
	float playerBottom = playerPos.y + playerHalfHeight;

	float otherLeft = otherPos.x - otherRadius;
	float otherRight = otherPos.x + otherRadius;
	float otherTop = otherPos.y - otherRadius;
	float otherBottom = otherPos.y + otherRadius;

	return (playerLeft < otherRight &&
		playerRight > otherLeft &&
		playerTop < otherBottom &&
		playerBottom > otherTop);
}