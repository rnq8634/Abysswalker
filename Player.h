#ifndef __PLAYER_H
#define __PLAYER_H

// Local includes
#include "vector2.h"

// Forward Declaration
class Renderer;
class AnimatedSprite;
class Sprite;

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

	Vector2& Position();

protected:

private:
	Player(const Player& player);
	Player& operator=(const Player& player);

	// Member data
public:

protected:
	Vector2 m_position;

	AnimatedSprite* m_pSprite;
	Sprite* m_pStaticSprite;
	bool m_bAlive;

private:

};

#endif // __PLAYER_H