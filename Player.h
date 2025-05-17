#ifndef __PLAYER_H
#define __PLAYER_H

// Local includes
#include "Entity.h"
#include "vector2.h"
#include "InputSystem.h"

// Lib includes
#include <map>
#include <functional>
#include <string>
#include <set>

// Forward Declaration
class Renderer;
class AnimatedSprite;
class Sprite;

const int PLAYER_SPRITE_WIDTH = 120;
const int PLAYER_SPRITE_HEIGHT = 80;

// enum for player states
enum class PlayerState
{
	IDLE,
	RUNNING,
	JUMPING,
	FALLING,
	ATTACKING,
	TURNING,
	ROLLING,
	HURT,
	DEATH
};

class Player : public Entity
{
	// Typedef for cleaner callback
	using AnimationCallBack = std::function<void()>;

	// Member methods
public:
	Player();
	~Player();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);
	void DebugDraw();

	// Getters
	PlayerState GetCurrentState() const { return m_currentState;  }
	bool IsFacingRight() const { return m_bFacingRight; }

	// stat getters
	float GetCurrentStamina() const { return m_currentStamina;  }
	float GetMaxStamina() const { return m_maxStamina;  }

	// Methods for player movement
	void MoveLeft(float amount);
	void MoveRight(float amount);
	void Jump();
	void Attack();
	void Roll(float speedBoost);
	void StopMoving();

	// stat modifiers
	void TakeDamage(int amount);
	bool UseStamina(float amount); // will return false if not enough stamina
	void Revive();
	bool WasJustRevived() const { return m_justRevived; }
	void ClearReviveFlag() { m_justRevived = false; }

	// For tracking enemies hit
	bool DamageDoneToTarget(Entity* target);
	void ClearHitEntitiesList();

	AnimatedSprite* GetCurrentAnimatedSprite();

	bool CheckCollision(const Entity& other) const;
	bool IsInvincible() const { return m_invincibilityTimer > 0.0f; }

protected:
	// Methods for turn animation
	void TurnAnimationComplete();
	void RollAnimationComplete();
	void AttackAnimationComplete();
	void JumpAnimationComplete();
	void FallOnLand(); // added called when landing on ground
	void HurtAnimationComplete();
	void DeathAnimationComplete();

private:
	// helper methods
	bool InitialiseAnimatedSprite(
		Renderer& renderer,
		PlayerState state,
		const char* pcFilename,
		int frameWidth,
		int frameHeight,
		float frameDuration,
		bool loop,
		AnimationCallBack onComplete = nullptr
	);
	void TransitionToState(PlayerState newState);
	std::set<Entity*> m_hitEntitiesThisAttack;

	void UpdateSprite(AnimatedSprite* sprite, float deltaTime);
	void StartTurn(float desiredSpeed, bool turnToRight);

	Player(const Player& player);
	Player& operator=(const Player& player);

	// Member data
public:
	static const int PLAYER_SPRITE_WIDTH = 120;
	static const int PLAYER_SPRITE_HEIGHT = 80;

	const float kGroundLevel = 850.0f;

protected:
	PlayerState m_currentState;
	bool m_bFacingRight;

	// Static (NEEDS STATIC SPRITE TO LOAD SPRITES)
	Sprite* m_pStaticSprite;
	std::map<PlayerState, AnimatedSprite*> m_animatedSprites;

	// Action States
	float m_desiredMoveSpeed;
	bool m_isTurning;
	bool m_targetFacingRight;
	float m_rollVelocityBeforeRoll;
	bool m_justRevived;
	bool m_bIsInvincible;

	// Player I-Frames
	float m_invincibilityTimer;
	const float m_invincibilityDuration = 1.0f;

	// Stats
	float m_maxStamina;
	float m_currentStamina;
	float m_staminaRegenRate;

	// Physics constants
	const float kGravity = 150.0f; // gravity strengtth
	const float kJumpForce = 100.0f; // initial jump strength
	const float kTurnSpeedFactor = 0.2f; // speed multiplier when turning

private:

};

#endif // __PLAYER_H