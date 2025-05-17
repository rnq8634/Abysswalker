#ifndef __ABYSSALESSENCE_H__
#define __ABYSSALESSENCE_H__

class AbyssalEssence
{
    // Member methods
public:
    AbyssalEssence();
    ~AbyssalEssence();

    void AddEssence(int amount);
    bool SpendEssence(int amount); // Generic spending
    int GetCurrentAmount() const;
    bool CanAfford(int amount) const;

    // Specific for Revival
    bool CanRevive() const;
    bool SpendForRevive(); // Uses the defined revive cost

    void DebugDraw();

    static const int DEFAULT_REVIVE_COST = 100;

    // Member data
private:
    int m_currentAmount;
    // If REVIVE_COST needs to be dynamic, it could be a member variable
    // initialized in the constructor or set via a method.
    // For now, a static const is fine.
};

#endif // __ABYSSALESSENCE_H__