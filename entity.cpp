// This include
#include "Entity.h"

// Local includes
#include "Sprite.h"
#include "Renderer.h"

// Lib includes
#include "imgui/imgui.h"
#include <cmath>

Entity::Entity()
	: m_pSprite(nullptr)
	, m_position(0.0f, 0.0f)
	, m_velocity(0.0f, 0.0f)
	, m_bAlive(true)
	, m_currentHealth(100)
	, m_maxHealth(100)
	, m_radius(10.0f)
{
}

Entity::~Entity()
{
	if (m_pSprite)
	{
		delete m_pSprite;
		m_pSprite = nullptr;
	}
}

// Initialsie the entity
bool Entity::Initialise(Renderer& renderer)
{
	m_bAlive = true;
	m_currentHealth = m_maxHealth;
	return true;
}

void Entity::Process(float deltaTime)
{
	m_position += m_velocity * deltaTime;
	if (!m_bAlive) return;
	if (m_pSprite)
	{
		m_pSprite->SetX(static_cast<int>(m_position.x));
		m_pSprite->SetY(static_cast<int>(m_position.y));
	}
}

// draw the entity using the renderer
void Entity::Draw(Renderer& renderer)
{
	if (m_pSprite && m_bAlive)
	{
		m_pSprite->SetScale(2.0f, 2.0f);
		m_pSprite->Draw(renderer);
	}
}

void Entity::DebugDraw()
{
	if (ImGui::TreeNode((void*)this, "Entity Data (Base)"))
	{
		ImGui::Text("Position: (%.2f, %.2f)", m_position.x, m_position.y);
		ImGui::Text("Velocity: (%.2f, %.2f)", m_velocity.x, m_velocity.y);
		ImGui::Checkbox("Is Alive", &m_bAlive);
		ImGui::SliderInt("Health", &m_currentHealth, 0, m_maxHealth);
		ImGui::SameLine();
		if (ImGui::InputInt("Max HP", &m_maxHealth)) {
			if (m_maxHealth < 1) m_maxHealth = 1;
			if (m_currentHealth > m_maxHealth) m_currentHealth = m_maxHealth;
		}
		ImGui::DragFloat("Collision Radius", &m_radius, 0.1f, 0.0f, 500.0f);
		ImGui::TreePop();
	}
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
	return m_radius; // Default value
}

void
Entity::SetRadius(float radius)
{
	m_radius = radius > 0 ? radius : 0.0f;
}

Vector2& Entity::GetPosition()
{
	return m_position;
}

Vector2& Entity::GetVelocity()
{
	return m_velocity;
}

bool Entity::IsCollidingWith(Entity& toCheck)
{
	if (!this->IsAlive() || !toCheck.IsAlive())
	{
		return false; // No collision if one is dead
	}

	Vector2 diff = this->m_position - toCheck.GetPosition();
	float distSq = diff.LengthSquared();
	float sumRadii = this->GetRadius() + toCheck.GetRadius();

	return distSq < (sumRadii * sumRadii);
}

void Entity::TakeDamage(int amount)
{
	if (!m_bAlive) return; // Cannot damage a dead entity

	m_currentHealth -= amount;
	if (m_currentHealth <= 0)
	{
		m_currentHealth = 0;
		SetDead(); // Mark as dead if health drops to 0 or below
	}
	else if (m_currentHealth > m_maxHealth)
	{
		m_currentHealth = m_maxHealth;
	}
}

int Entity::GetCurrentHealth() const
{
	return m_currentHealth;
}

int Entity::GetMaxHealth() const
{
	return m_maxHealth;
}

void Entity::SetMaxHealth(int maxHealth, bool setCurrentToMax)
{
	m_maxHealth = maxHealth > 0 ? maxHealth : 1;
	if (setCurrentToMax)
	{
		m_currentHealth = m_maxHealth;
	}
	else
	{
		if (m_currentHealth > m_maxHealth)
		{
			m_currentHealth = m_maxHealth;
		}
	}
}