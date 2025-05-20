#ifndef __ABYSSALESSENCE_H__
#define __ABYSSALESSENCE_H__

class AbyssalEssence
{
    // Member methods
public:
    AbyssalEssence();
    ~AbyssalEssence();

    void AddEssence(int amount);
    bool SpendEssence(int amount);
    int GetCurrentAmount() const;
    bool CanAfford(int amount) const;

    // Specific for Revival
    bool CanRevive() const;
    bool SpendForRevive();

    void DebugDraw();

    static const int DEFAULT_REVIVE_COST = 100;
    static const int MAX_ESSENCE_CAP = 99999;

    // Member data
private:
    int m_currentAmount;
};

#endif // __ABYSSALESSENCE_H__