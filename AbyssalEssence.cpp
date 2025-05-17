// This include
#include "AbyssalEssence.h"

// Local Includes
#include "LogManager.h" // For logging, if you want to log essence changes

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <algorithm> // For std::max

AbyssalEssence::AbyssalEssence()
    : m_currentAmount(0) // Player starts with 0 essence
{
}

AbyssalEssence::~AbyssalEssence()
{
}

void AbyssalEssence::AddEssence(int amount)
{
    if (amount > 0)
    {
        m_currentAmount += amount;
    }
}

bool AbyssalEssence::SpendEssence(int amount)
{
    if (amount <= 0)
    {
        // LogManager::GetInstance().Log("Cannot spend zero or negative essence.");
        return false;
    }
    if (m_currentAmount >= amount)
    {
        m_currentAmount -= amount;
        // Optional: Log essence spent
        // LogManager::GetInstance().Log(("Spent " + std::to_string(amount) + " Abyssal Essence. Remaining: " + std::to_string(m_currentAmount)).c_str());
        return true;
    }
    // LogManager::GetInstance().Log("Not enough Abyssal Essence to spend.");
    return false;
}

int AbyssalEssence::GetCurrentAmount() const
{
    return m_currentAmount;
}

bool AbyssalEssence::CanAfford(int amount) const
{
    return m_currentAmount >= amount;
}

bool AbyssalEssence::CanRevive() const
{
    return CanAfford(DEFAULT_REVIVE_COST);
}

bool AbyssalEssence::SpendForRevive()
{
    if (SpendEssence(DEFAULT_REVIVE_COST))
    {
        LogManager::GetInstance().Log("Spent essence for revival.");
        return true;
    }
    // LogManager::GetInstance().Log("Could not spend essence for revival (not enough).");
    return false; // SpendEssence would have already logged "not enough" if that was the case
}

void AbyssalEssence::DebugDraw()
{
    if (ImGui::TreeNode("Abyssal Essence"))
    {
    }
}