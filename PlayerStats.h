#ifndef __PLAYERSTATS_H__
#define __PLAYERSTATS_H__

// Forward Declarations
class AbyssalEssence;

enum class StatType
{
    ATTACK_DAMAGE,
    MAX_HEALTH,
    HEALTH_REGEN,
    MAX_STAMINA,
    STAMINA_REGEN
};

class PlayerStats
{
public:
    // Member methids
    PlayerStats();
    ~PlayerStats();

    void InitialiseDefault();
    void ResetStats(); // When game resets

    // Getters for effective stat values
    int GetAttackDamage() const;
    int GetMaxHealth() const;
    float GetHealthRegenRate() const;
    float GetMaxStamina() const;
    float GetStaminaRegenRate() const;

    // Upgrade Logic
    int GetCurrentUpgradeCost() const;
    bool CanAffordUpgrade(const AbyssalEssence& essence) const;
    bool AttemptUpgrade(StatType type, AbyssalEssence& essence);

    void ModifyBaseAttack(int amount);
    void ModifyBaseMaxHealth(int amount);

    void DebugDraw();

private:
    void CalculateNextUpgradeCost();

    // Member data
private:
    // Base Values
    int m_baseAttackDamage;
    int m_baseMaxHealth;
    float m_baseMaxStamina;
    float m_baseStaminaRegenRate;
    float m_baseHealthRegenRate;

    // Upgrade Levels
    int m_attackDamageLevel;
    int m_maxHealthLevel;
    int m_healthRegenLevel;
    int m_maxStaminaLevel;
    int m_staminaRegenLevel;

    // Amount per level
    const int m_attackDamagePerLevel = 5;
    const int m_maxHealthPerLevel = 20;
    const float m_healthRegenPerLevel = 0.5f;
    const float m_maxStaminaPerLevel = 15.0f;
    const float m_staminaRegenPerLevel = 2.0f;

    // Upgrade Cost
    int m_currentUpgradeCost;
    int m_totalUpgradesPurchased;
    const int m_initialUpgradeCost = 10;
    const int m_costIncreasePerUpgrade = 4;
};

#endif //__PLAYERSTATS_H__
