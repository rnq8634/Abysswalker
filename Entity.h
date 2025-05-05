#ifndef __ENTITY_H
#define __ENTITY_H

// Local inc
#include "entity.h"
#include "vector2.h"

// Foward dec
class Renderer;
class Sprite;

// Class Dec
class Entity
{
	// Member methods
public:
	Entity();
	~Entity();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);

	bool IsAlive() const;
	void SetDead();

	float GetRadius();
	Vector2 GetFacingDirection();

	Vector2& GetPosition();
	Vector2& GetVelocity();

	bool IsCollidingWith(Entity& toCheck); // Collision

protected:

private:
	Entity(const Entity& entity);
	Entity& operator=(const Entity& entity);

	// Member data
public:

protected:
	Sprite* m_pSprite;
	Vector2 m_position;
	Vector2 m_velocity;
	bool m_bAlive;

private:

};

#endif // __ENTITY_H