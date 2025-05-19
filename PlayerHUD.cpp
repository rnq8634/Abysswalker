// This include
#include "PlayerHUD.h"

// Local include
#include "Renderer.h"
#include "Player.h"
#include "Sprite.h"    
#include "Texture.h"   
#include "LogManager.h" 
#include "AbyssalEssence.h"   

// Lib includes
#include <algorithm>  
#include <string>

PlayerHUD::PlayerHUD(Renderer* renderer, Player* player)
    : m_pRenderer(renderer)
    , m_pPlayer(player)
    , m_pEssenceTextSprite(nullptr)
    , m_pEssenceTextTexture(nullptr)
    , m_lastEssenceStr("")
{
    if (!m_pRenderer || !m_pPlayer) 
    {
        LogManager::GetInstance().Log("PlayerHUD critical error: Null pointer in constructor.");
    }
}

PlayerHUD::~PlayerHUD()
{
    delete m_pEssenceTextSprite; m_pEssenceTextSprite = nullptr;
    delete m_pEssenceTextTexture; m_pEssenceTextTexture = nullptr;
}

void PlayerHUD::Draw()
{
    if (!m_pPlayer || !m_pRenderer) return;

    int screenWidth = m_pRenderer->GetWidth();
    int screenHeight = m_pRenderer->GetHeight();

    // Calculate total width of the UI group (Health + Space + Stamina) to center it
    float groupStartX = (static_cast<float>(screenWidth) - totalBarGroupWidth) / 2.0f;

    // --- Health Bar ---
    float healthBarX = groupStartX;
    float healthBarY = static_cast<float>(screenHeight) - BAR_Y_OFFSET_FROM_BOTTOM - BAR_HEIGHT;

    int currentHealth = m_pPlayer->GetCurrentHealth();
    int maxHealth = m_pPlayer->GetPlayerStats().GetMaxHealth();
    float healthRatio = (maxHealth > 0) ? static_cast<float>(currentHealth) / static_cast<float>(maxHealth) : 0.0f;
    healthRatio = std::max(0.0f, std::min(1.0f, healthRatio));

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

    // --- Abyssal Essence Display ---
    std::string currentEssenceStr = "Essence: " + std::to_string(m_pPlayer->GetAbyssalEssence().GetCurrentAmount());
    if (m_lastEssenceStr != currentEssenceStr || !m_pEssenceTextSprite) 
    {
        delete m_pEssenceTextSprite; m_pEssenceTextSprite = nullptr;
        delete m_pEssenceTextTexture; m_pEssenceTextTexture = nullptr;

        m_pEssenceTextTexture = new Texture();
        m_pEssenceTextSprite = new Sprite();
        if (m_pEssenceTextSprite->InitialiseWithText(*m_pEssenceTextTexture, currentEssenceStr.c_str(), m_uiFontPath, m_uiFontSize)) 
        {
            m_lastEssenceStr = currentEssenceStr;
            m_pEssenceTextSprite->SetRedTint(ESSENCE_TEXT_COLOR_R / 255.0f);
            m_pEssenceTextSprite->SetGreenTint(ESSENCE_TEXT_COLOR_G / 255.0f);
            m_pEssenceTextSprite->SetBlueTint(ESSENCE_TEXT_COLOR_B / 255.0f);
        }
        else 
        {
            LogManager::GetInstance().Log("Failed to create essence text sprite for HUD.");
            delete m_pEssenceTextSprite; m_pEssenceTextSprite = nullptr;
            delete m_pEssenceTextTexture; m_pEssenceTextTexture = nullptr;
            m_lastEssenceStr = "";
        }
    }

    if (m_pEssenceTextSprite) 
    {
        float essenceTextWidth = static_cast<float>(m_pEssenceTextTexture->GetWidth());
        float essencePanelWidth = essenceTextWidth + 20.0f;
        float essencePanelHeight = BAR_HEIGHT;
        // Positioned next to stam bar
        float essencePanelX = staminaBarX + STAMINA_BAR_WIDTH + BAR_SPACING;
        float essencePanelY = healthBarY;

        // Essence Border
        m_pRenderer->DrawDebugRect(essencePanelX, essencePanelY, essencePanelX + essencePanelWidth, essencePanelY + essencePanelHeight,
            BAR_BG_R, BAR_BG_G, BAR_BG_B, BAR_BG_A);

        // Essence Background
        m_pRenderer->DrawDebugRect(essencePanelX - BORDER_THICKNESS, essencePanelY - BORDER_THICKNESS, essencePanelX + essencePanelWidth + BORDER_THICKNESS, essencePanelY + essencePanelHeight + BORDER_THICKNESS,
            BAR_BORDER_R, BAR_BORDER_G, BAR_BORDER_B, BAR_BORDER_A);

        m_pEssenceTextSprite->SetX(static_cast<int>(essencePanelX + essencePanelWidth / 2));
        m_pEssenceTextSprite->SetY(static_cast<int>(essencePanelY + essencePanelHeight / 2));
        m_pEssenceTextSprite->Draw(*m_pRenderer);
    }
}

float PlayerHUD::GetHealthBarStartX(int screenWidth) const
{
    return (static_cast<float>(screenWidth) - this->totalBarGroupWidth) / 2.0f;
}