// This includes
#include "PlayerStats.h"

// Local includes
#include "AbyssalEssence.h"
#include "LogManager.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <string>

PlayerStats::PlayerStats()
{
    InitialiseDefault();
}

PlayerStats::~PlayerStats()
{
}

void PlayerStats::InitialiseDefault()
{
    m_baseAttackDamage = 25; // Default player attack
    m_baseMaxHealth = 125;
    m_baseMaxStamina = 115.0f;
    m_baseStaminaRegenRate = 10.0f;
    m_baseHealthRegenRate = 0.0f; // Starts with no health regen

    m_attackDamageLevel = 0;
    m_maxHealthLevel = 0;
    m_healthRegenLevel = 0;
    m_maxStaminaLevel = 0;
    m_staminaRegenLevel = 0;

    m_totalUpgradesPurchased = 0;
    m_currentUpgradeCost = m_initialUpgradeCost;
}

void PlayerStats::ResetStats()
{
    InitialiseDefault();
}

int PlayerStats::GetAttackDamage() const
{
    return m_baseAttackDamage + (m_attackDamageLevel * m_attackDamagePerLevel);
}

int PlayerStats::GetMaxHealth() const
{
    return m_baseMaxHealth + (m_maxHealthLevel * m_maxHealthPerLevel);
}

float PlayerStats::GetHealthRegenRate() const
{
    // Health regen is purely from upgrades
    return m_baseHealthRegenRate + (m_healthRegenLevel * m_healthRegenPerLevel);
}

float PlayerStats::GetMaxStamina() const
{
    return m_baseMaxStamina + (m_maxStaminaLevel * m_maxStaminaPerLevel);
}

float PlayerStats::GetStaminaRegenRate() const
{
    return m_baseStaminaRegenRate + (m_staminaRegenLevel * m_staminaRegenPerLevel);
}

int PlayerStats::GetCurrentUpgradeCost() const
{
    return m_currentUpgradeCost;
}

bool PlayerStats::CanAffordUpgrade(const AbyssalEssence& essence) const
{
    return essence.CanAfford(m_currentUpgradeCost);
}

bool PlayerStats::AttemptUpgrade(StatType type, AbyssalEssence& essence)
{
    if (!CanAffordUpgrade(essence))
    {
        LogManager::GetInstance().Log("Not enough essence for upgrade.");
        return false;
    }

    if (essence.SpendEssence(m_currentUpgradeCost))
    {
        switch (type)
        {
        case StatType::ATTACK_DAMAGE:
            m_attackDamageLevel++;
            LogManager::GetInstance().Log("Upgraded Attack Damage.");
            break;
        case StatType::MAX_HEALTH:
            m_maxHealthLevel++;
            LogManager::GetInstance().Log("Upgraded Max Health.");
            break;
        case StatType::HEALTH_REGEN:
            m_healthRegenLevel++;
            LogManager::GetInstance().Log("Upgraded Health Regen.");
            break;
        case StatType::MAX_STAMINA:
            m_maxStaminaLevel++;
            LogManager::GetInstance().Log("Upgraded Max Stamina.");
            break;
        case StatType::STAMINA_REGEN:
            m_staminaRegenLevel++;
            LogManager::GetInstance().Log("Upgraded Stamina Regen.");
            break;
        default:
            LogManager::GetInstance().Log("Unknown stat type for upgrade.");
            essence.AddEssence(m_currentUpgradeCost);
            return false;
        }
        m_totalUpgradesPurchased++;
        CalculateNextUpgradeCost();
        return true;
    }
    return false;
}

void PlayerStats::CalculateNextUpgradeCost()
{
    m_currentUpgradeCost = m_initialUpgradeCost + (m_totalUpgradesPurchased * m_costIncreasePerUpgrade);
}

void PlayerStats::ModifyBaseAttack(int amount) 
{ 
    m_baseAttackDamage += amount; 
}
void PlayerStats::ModifyBaseMaxHealth(int amount) 
{ 
    m_baseMaxHealth += amount; 
}

void PlayerStats::DebugDraw()
{
    if (ImGui::TreeNode("Player Stats"))
    {
        ImGui::Text("Attack Damage: %d (Lvl %d)", GetAttackDamage(), m_attackDamageLevel);
        ImGui::Text("Max Health: %d (Lvl %d)", GetMaxHealth(), m_maxHealthLevel);
        ImGui::Text("Health Regen: %.2f/s (Lvl %d)", GetHealthRegenRate(), m_healthRegenLevel);
        ImGui::Text("Max Stamina: %.2f (Lvl %d)", GetMaxStamina(), m_maxStaminaLevel);
        ImGui::Text("Stamina Regen: %.2f/s (Lvl %d)", GetStaminaRegenRate(), m_staminaRegenLevel);
        ImGui::Separator();
        ImGui::Text("Next Upgrade Cost: %d Essence", m_currentUpgradeCost);
        ImGui::Text("Total Upgrades Purchased: %d", m_totalUpgradesPurchased);
        ImGui::TreePop();
    }
}