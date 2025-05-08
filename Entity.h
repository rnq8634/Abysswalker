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
	virtual ~Entity();

	virtual bool Initialise(Renderer& renderer);
	virtual void Process(float deltaTime);
	virtual void Draw(Renderer& renderer);
	virtual void DebugDraw();

	bool IsAlive() const;
	void SetDead();

	virtual float GetRadius();
	void SetRadius(float radius);
	Vector2 GetFacingDirection();

	Vector2& GetPosition();
	const Vector2& GetPosition() const;
	Vector2& GetVelocity();
	const Vector2& GetVelocity() const;

	bool IsCollidingWith(Entity& toCheck); // Collision

	// Stats
	virtual void TakeDamage(int amount);
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