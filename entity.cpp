// This include
#include "Entity.h"
#include "Sprite.h"
#include "Renderer.h"

// COnstructor
Entity::Entity()
	: m_pSprite(0)
	, m_position(0.0f, 0.0f)
	, m_velocity(0.0f, 0.0f)
	, m_bAlive(true)
{

}

// Destructor
Entity::~Entity()
{
	delete m_pSprite;
	m_pSprite = 0;
}

// Initialsie the entity
bool 
Entity::Initialise(Renderer& renderer)
{
	m_pSprite = new Sprite();

	if (!m_pSprite)
	{
		return false;
	}

	return true;
}

// Process entity logic based on time passed
void 
Entity::Process(float deltaTime)
{
	m_position += m_velocity * deltaTime;
}

// draw the entity using the renderer
void 
Entity::Draw(Renderer& renderer)
{
	if (m_pSprite && m_bAlive)
	{

	}
}

// Rotate the entity in a given direction
void 
Entity::Rotate(float direction)
{

}

// Check if the entity is alive
bool
Entity::IsAlive() const
{
	return m_bAlive;
}

// Mark the entity as dead
void
Entity::SetDead()
{
	m_bAlive = false;
}

// Get the collision radious of the entity
float
Entity::GetRadius()
{
	return 1.0f; // Default value
}

Vector2
Entity::GetFacingDirection()
{
	// Can be calculated from velocity or stored separately
	if (m_velocity.Length() > 0)
	{
		
	}

	return Vector2(1.0f, 0.0f);
}

// Get position reference
Vector2& 
Entity::GetPosition()
{
	return m_position;
}

// Get velocity reference
Vector2& 
Entity::GetVelocity()
{
	return m_velocity;
}

// Check collision with another entity
bool
Entity::IsCollidingWith(Entity& toCheck)
{
	// Simple circle collision
	float totalRadius = GetRadius() + toCheck.GetRadius();
	Vector2 difference = m_position - toCheck.GetPosition();
	float distance = difference.Length();

	// Collision occurs when the distance is less than the sum of radii
	return distance < totalRadius;
}