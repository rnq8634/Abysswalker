// PlayerHUD.cpp
#include "PlayerHUD.h"
#include "Renderer.h"       
#include "Player.h"         
#include "PlayerStats.h"   
#include <algorithm>      

PlayerHUD::PlayerHUD(Renderer* renderer, Player* player)
    : m_pRenderer(renderer), m_pPlayer(player)
{
}

PlayerHUD::~PlayerHUD()
{
}

void PlayerHUD::Draw()
{
    if (!m_pPlayer || !m_pRenderer) return;

    int screenWidth = m_pRenderer->GetWidth();
    int screenHeight = m_pRenderer->GetHeight();

    // Calculate total width of the UI group (Health + Space + Stamina) to center it
    float totalUIGroupWidth = HEALTH_BAR_WIDTH + BAR_SPACING + STAMINA_BAR_WIDTH;
    float groupStartX = (static_cast<float>(screenWidth) - totalUIGroupWidth) / 2.0f;

    // --- Health Bar ---
    float healthBarX = groupStartX;
    float healthBarY = static_cast<float>(screenHeight) - BAR_Y_OFFSET_FROM_BOTTOM - BAR_HEIGHT;

    int currentHealth = m_pPlayer->GetCurrentHealth();
    int maxHealth = m_pPlayer->GetPlayerStats().GetMaxHealth();
    float healthRatio = (maxHealth > 0) ? static_cast<float>(currentHealth) / static_cast<float>(maxHealth) : 0.0f;
    healthRatio = std::max(0.0f, std::min(1.0f, healthRatio)); // Clamp

    // Health Bar Border
    m_pRenderer->DrawDebugRect(healthBarX - BORDER_THICKNESS, healthBarY - BORDER_THICKNESS,
        healthBarX + HEALTH_BAR_WIDTH + BORDER_THICKNESS,
        healthBarY + BAR_HEIGHT + BORDER_THICKNESS,
        BAR_BORDER_R, BAR_BORDER_G, BAR_BORDER_B, BAR_BORDER_A);

    // Health Bar Background
    m_pRenderer->DrawDebugRect(healthBarX, healthBarY,
        healthBarX + HEALTH_BAR_WIDTH,
        healthBarY + BAR_HEIGHT,
        BAR_BG_R, BAR_BG_G, BAR_BG_B, BAR_BG_A);

    // Health Bar Fill
    if (healthRatio > 0)
    {
        m_pRenderer->DrawDebugRect(healthBarX, healthBarY,
            healthBarX + (HEALTH_BAR_WIDTH * healthRatio),
            healthBarY + BAR_HEIGHT,
            HEALTH_FILL_R, HEALTH_FILL_G, HEALTH_FILL_B, BAR_FILL_A);
    }

    // --- Stamina Bar ---
    float staminaBarX = healthBarX + HEALTH_BAR_WIDTH + BAR_SPACING;
    float staminaBarY = healthBarY;

    float currentStamina = m_pPlayer->GetCurrentStamina();
    float maxStamina = m_pPlayer->GetPlayerStats().GetMaxStamina();
    float staminaRatio = (maxStamina > 0.0f) ? currentStamina / maxStamina : 0.0f;
    staminaRatio = std::max(0.0f, std::min(1.0f, staminaRatio));

    // Stamina Bar Border
    m_pRenderer->DrawDebugRect(staminaBarX - BORDER_THICKNESS, staminaBarY - BORDER_THICKNESS,
        staminaBarX + STAMINA_BAR_WIDTH + BORDER_THICKNESS,
        staminaBarY + BAR_HEIGHT + BORDER_THICKNESS,
        BAR_BORDER_R, BAR_BORDER_G, BAR_BORDER_B, BAR_BORDER_A);

    // Stamina Bar Background
    m_pRenderer->DrawDebugRect(staminaBarX, staminaBarY,
        staminaBarX + STAMINA_BAR_WIDTH,
        staminaBarY + BAR_HEIGHT,
        BAR_BG_R, BAR_BG_G, BAR_BG_B, BAR_BG_A);

    // Stamina Bar Fill
    if (staminaRatio > 0)
    {
        m_pRenderer->DrawDebugRect(staminaBarX, staminaBarY,
            staminaBarX + (STAMINA_BAR_WIDTH * staminaRatio),
            staminaBarY + BAR_HEIGHT,
            STAMINA_FILL_R, STAMINA_FILL_G, STAMINA_FILL_B, BAR_FILL_A);
    }
}