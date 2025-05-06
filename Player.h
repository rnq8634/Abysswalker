#ifndef __PLAYER_H
#define __PLAYER_H

// Local includes
#include "vector2.h"
#include "InputSystem.h"

// Lib includes
#include <map>
#include <functional>
#include <string>

// Forward Declaration
class Renderer;
class AnimatedSprite;
class Sprite;
class Vector2;

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

class Player
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
	Vector2& Position() { return m_position; }
	PlayerState GetCurrentState() const { return m_currentState;  }
	bool IsFacingRight() const { return m_bFacingRight; }
	bool IsAlive() const { return m_bAlive;  }

	// Methods for player movement
	void MoveLeft(float amount);
	void MoveRight(float amount);
	void Jump(float amount);
	void Attack();
	void Roll(float speedBoost);
	void StopMoving();

protected:
	// Methods for turn animation
	void OnTurnAnimationComplete();
	void OnRollAnimationComplete();
	void OnAttackAnimationComplete();
	void OnJumpAnimationComplete();
	void OnFallLand(); // added called when landing on ground

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
	AnimatedSprite* GetCurrentAnimatedSprite();

	void UpdateSprite(AnimatedSprite* sprite, float deltaTime);

	void StartTurn(float desiredSpeed, bool turnToRight);

	Player(const Player& player);
	Player& operator=(const Player& player);

	// Member data
public:

protected:
	// Position and movement
	Vector2 m_position; // Position of the player
	Vector2 m_velocity; // Speed of the player
	PlayerState m_currentState;
	bool m_bAlive;
	bool m_bFacingRight;

	// Static (NEEDS STATIC SPRITE TO LOAD SPRITES)
	Sprite* m_pStaticSprite;

	std::map<PlayerState, AnimatedSprite*> m_animatedSprites;

	// State specific data
	// Turn animation
	float m_desiredMoveSpeed;
	bool m_isTurning;

	bool m_targetFacingRight;

	float m_rollVelocityBeforeRoll;

	// Physics constants
	const float kGroundLevel = 360.0f;
	const float kGravity = 150.0f; // gravity strengtth
	const float kJumpForce = 100.0f; // initial jump strength
	const float kTurnSpeedFactor = 0.2f; // speed multiplier when turning

private:

};

#endif // __PLAYER_H