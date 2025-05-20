// This include
#include "AbyssalEssence.h"

// Local Includes
#include "LogManager.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <algorithm> 

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
        return false;
    }
    if (m_currentAmount >= amount)
    {
        m_currentAmount -= amount;
        return true;
    }
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
    return false;
}

void AbyssalEssence::DebugDraw()
{
    if (ImGui::TreeNode("Abyssal Essence"))
    {
        ImGui::Text("Current Amount: %d / %d", m_currentAmount, MAX_ESSENCE_CAP);

        // Add essence
        if (ImGui::Button("Add 500 Essence")) 
        { 
            AddEssence(500); 
        }

        ImGui::TreePop();
    }
}