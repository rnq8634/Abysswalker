#ifndef __ENTITY_H
#define __ENTITY_H

// Local inc
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
	void DebugDraw();

	bool IsAlive() const;
	void SetDead();

	float GetRadius();
	void SetRadius(float radius);
	Vector2 GetFacingDirection();

	Vector2& GetPosition();
	Vector2& GetVelocity();

	bool IsCollidingWith(Entity& toCheck); // Collision

	// Stats
	void TakeDamage(int amount);
	int GetCurrentHealth() const;
	int GetMaxHealth() const;
	void SetMaxHealth(int maxHealth, bool setCurrentToMax = true);

protected:

private:

	// Member data
public:

protected:
	Sprite* m_pSprite;
	Vector2 m_position;
	Vector2 m_velocity;
	bool m_bAlive;

	// Stats
	int m_currentHealth;
	int m_maxHealth;

	float m_radius;

private:
	Entity(const Entity& entity);
	Entity& operator=(const Entity& entity);
};

#endif // __ENTITY_H