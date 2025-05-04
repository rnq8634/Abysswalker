#ifndef __PLAYER_H
#define __PLAYER_H

// Local includes
#include "vector2.h"
#include "InputSystem.h"

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
	ROLLING
};

class Player
{
	// Member methods
public:
	Player();
	~Player();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);
	void DebugDraw();

	// Get player position
	Vector2& Position();

	// Methods for player movement
	void MoveLeft(float amount);
	void MoveRight(float amount);
	void Jump(float amount);
	void Fall(float amount);
	void Attack(float amount);
	void TurnAround(float amount);
	void Roll(float amount);
	void StopMoving();

protected:
	// Methods for turn animation
	void StartTurnAnimation(float desiredSpeed, bool turnToRight);
	void OnTurnAnimationComplete();

private:
	Player(const Player& player);
	Player& operator=(const Player& player);

	// Member data
public:

protected:
	// Position and movement
	Vector2 m_position; // Position of the player
	Vector2 m_velocity; // Speed of the player

	// Animated Sprite related
	AnimatedSprite* m_pIdleSprite;
	AnimatedSprite* m_pRunSprite;
	AnimatedSprite* m_pJumpSprite;
	AnimatedSprite* m_pFallSprite;
	AnimatedSprite* m_pAttackSprite;
	AnimatedSprite* m_pTurnAroundSprite;
	AnimatedSprite* m_pRollSprite;

	// Static 
	Sprite* m_pStaticSprite;

	// State tracking
	PlayerState m_currentState; // Checks the players current state
	bool m_bAlive;
	bool m_bFacingRight; // To track the direction of the player

	// Turn animation variables
	typedef void (*AnimationCompleteCallback)();
	AnimationCompleteCallback m_turnCompleteCallback;
	float m_desiredMoveSpeed;
	bool m_isTurning;
	bool m_targetFacingRight;

private:

};

#endif // __PLAYER_H