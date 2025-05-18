// This includes
#include "UpgradeMenu.h"

// Local includes
#include "Renderer.h"
#include "Player.h"     
#include "InputSystem.h"
#include "Sprite.h"
#include "Texture.h"
#include "LogManager.h"
#include "SoundSystem.h"
#include "SceneAbyssWalker.h"
#include "XboxController.h" 

// Lib includes
#include <cstdio>          

UpgradeMenu::UpgradeMenu(Renderer* renderer, Player* player, SceneAbyssWalker* scene)
    : m_pRenderer(renderer),
    m_pPlayer(player),
    m_pScene(scene),
    m_pUpgradeMenuTitleSprite(nullptr),
    m_pUpgradeMenuTitleTexture(nullptr),
    m_pEssenceTextSprite(nullptr),
    m_pEssenceTextTexture(nullptr),
    m_selectedUpgradeButtonIndex(-1),
    m_bIsActive(false)
{
    if (!m_pRenderer || !m_pPlayer || !m_pScene) 
    {
        LogManager::GetInstance().Log("UpgradeMenu critical error: Null pointer passed in constructor.");
    }
    // Initialize layout constants
    m_panelWidth = 300.0f;
    
    m_panelY = 50.0f;
    m_buttonWidth = m_panelWidth - 40.0f;
    m_buttonHeight = 30.0f;
    m_spacing = 8.0f;
    m_textPaddingX = 10.0f;
}

UpgradeMenu::~UpgradeMenu()
{
    ClearUI(); 
}

void UpgradeMenu::ClearUI() 
{
    for (auto& btn : m_upgradeButtons) 
    {
        delete btn.textSprite; btn.textSprite = nullptr;
        delete btn.textTexture; btn.textTexture = nullptr;
    }

    m_upgradeButtons.clear();

    delete m_pUpgradeMenuTitleSprite; m_pUpgradeMenuTitleSprite = nullptr;
    delete m_pUpgradeMenuTitleTexture; m_pUpgradeMenuTitleTexture = nullptr;
    delete m_pEssenceTextSprite; m_pEssenceTextSprite = nullptr;
    delete m_pEssenceTextTexture; m_pEssenceTextTexture = nullptr;
    m_selectedUpgradeButtonIndex = -1;
}

void UpgradeMenu::SetupUI() 
{
    if (!m_pRenderer || !m_pPlayer) return;
    ClearUI();

    m_panelX = m_pRenderer->GetWidth() - (m_panelWidth + 20.0f);
    float currentY = m_panelY;

    // Title
    m_pUpgradeMenuTitleTexture = new Texture();
    m_pUpgradeMenuTitleSprite = new Sprite();
    if (m_pUpgradeMenuTitleSprite->InitialiseWithText(*m_pUpgradeMenuTitleTexture, "Upgrade Stats", m_uiFontPath, m_uiTitleFontSize)) 
    {
        m_pUpgradeMenuTitleSprite->SetX(static_cast<int>(m_panelX + m_panelWidth / 2.0f));
        m_pUpgradeMenuTitleSprite->SetY(static_cast<int>(currentY + m_buttonHeight / 2.0f));
    }
    currentY += m_buttonHeight + m_spacing * 2;

    // Essence Text
    PlayerStats& stats = m_pPlayer->GetPlayerStats();
    AbyssalEssence& essence = m_pPlayer->GetAbyssalEssence();
    int cost = stats.GetCurrentUpgradeCost();
    std::string essenceStr = "Essence: " + std::to_string(essence.GetCurrentAmount()) + " | Cost: " + std::to_string(cost);
    m_pEssenceTextTexture = new Texture();
    m_pEssenceTextSprite = new Sprite();
    if (m_pEssenceTextSprite->InitialiseWithText(*m_pEssenceTextTexture, essenceStr.c_str(), m_uiFontPath, m_uiFontSize)) 
    {
        m_pEssenceTextSprite->SetX(static_cast<int>(m_panelX + m_panelWidth / 2.0f));
        m_pEssenceTextSprite->SetY(static_cast<int>(currentY + m_buttonHeight / 2.0f));
    }
    currentY += m_buttonHeight + m_spacing;

    // Create Buttons (using a helper to reduce redundancy)
    CreateUpgradeButton("Attack Damage", StatType::ATTACK_DAMAGE, "upgrade_atk", stats.GetAttackDamage());
    CreateUpgradeButton("Max Health", StatType::MAX_HEALTH, "upgrade_hp", stats.GetMaxHealth());
    CreateUpgradeButton("Health Regen", StatType::HEALTH_REGEN, "upgrade_hpreg", -1, stats.GetHealthRegenRate());
    CreateUpgradeButton("Max Stamina", StatType::MAX_STAMINA, "upgrade_stam", -1, stats.GetMaxStamina());
    CreateUpgradeButton("Stamina Regen", StatType::STAMINA_REGEN, "upgrade_stamreg", -1, stats.GetStaminaRegenRate());

    // Position buttons based on currentY dynamically
    float buttonStartX = m_panelX + 20.0f;
    for (auto& btn : m_upgradeButtons) 
    {
        btn.rect = { buttonStartX, currentY, m_buttonWidth, m_buttonHeight };
        if (btn.textSprite) 
        {
            btn.textSprite->SetX(static_cast<int>(btn.rect.x + m_textPaddingX + btn.textTexture->GetWidth() / 2.0f));
            btn.textSprite->SetY(static_cast<int>(btn.rect.y + btn.rect.height / 2.0f));
        }
        currentY += m_buttonHeight + m_spacing;
    }

    // "Done" button - created last so it's at the bottom of the list
    UIButton doneBtn;
    doneBtn.textTexture = new Texture();
    doneBtn.textSprite = new Sprite();
    if (doneBtn.textSprite->InitialiseWithText(*doneBtn.textTexture, "Done Upgrading", m_uiFontPath, m_uiFontSize)) 
    {
        doneBtn.rect = { buttonStartX, currentY + m_spacing, m_buttonWidth, m_buttonHeight };
        doneBtn.identifier = "done_upgrading";
        doneBtn.textSprite->SetX(static_cast<int>(doneBtn.rect.x + m_textPaddingX + doneBtn.textTexture->GetWidth() / 2.0f));
        doneBtn.textSprite->SetY(static_cast<int>(doneBtn.rect.y + doneBtn.rect.height / 2.0f));
        m_upgradeButtons.push_back(std::move(doneBtn)); // Use move if UIButton has move constructor/assignment
    }
    else 
    {
        delete doneBtn.textTexture; delete doneBtn.textSprite;
    }

    m_selectedUpgradeButtonIndex = m_upgradeButtons.empty() ? -1 : 0;

    // Calculate m_panelHeight
    if (!m_upgradeButtons.empty()) 
    {
        m_panelHeight = (m_upgradeButtons.back().rect.y + m_upgradeButtons.back().rect.height + 20.0f) - m_panelY;
    }
    else 
    {
        m_panelHeight = currentY - m_panelY + 20.0f; // Fallback if only title/essence text
    }
}

void UpgradeMenu::CreateUpgradeButton(const std::string& baseLabel, StatType type, const std::string& identifier, int currentVal, float currentValF)
{
    UIButton btn;
    std::string fullText = baseLabel + " (";
    char buffer[32];
    if (currentValF >= 0) {
        snprintf(buffer, sizeof(buffer), "%.1f", currentValF);
        fullText += buffer;
    }
    else 
    {
        fullText += std::to_string(currentVal);
    }
    fullText += ")";

    btn.textTexture = new Texture();
    btn.textSprite = new Sprite();
    if (!btn.textSprite->InitialiseWithText(*btn.textTexture, fullText.c_str(), m_uiFontPath, m_uiFontSize)) 
    {
        LogManager::GetInstance().Log(("Failed to create text for button: " + fullText).c_str());
        delete btn.textTexture; btn.textTexture = nullptr;
        delete btn.textSprite; btn.textSprite = nullptr;
        return; // Don't add a malformed button
    }
    
    btn.statToUpgrade = type;
    btn.identifier = identifier;
    m_upgradeButtons.push_back(std::move(btn));
}


void UpgradeMenu::UpdateUI(InputSystem& inputSystem)
{
    if (!m_bIsActive || !m_pPlayer || m_upgradeButtons.empty()) 
    {
        if (m_upgradeButtons.empty()) m_selectedUpgradeButtonIndex = -1;
        return;
    }

    XboxController* pController = nullptr;
    if (inputSystem.GetNumberOfControllersAttached() > 0) 
    {
        pController = inputSystem.GetController(0);
    }

    if (pController) {
        if (pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_DOWN) == BS_PRESSED) 
        {
            SoundSystem::GetInstance().PlaySound("ui_hover"); // Example sound
            if (m_selectedUpgradeButtonIndex < static_cast<int>(m_upgradeButtons.size()) - 1) 
            {
                m_selectedUpgradeButtonIndex++;
            }
            else 
            {
                m_selectedUpgradeButtonIndex = 0;
            }
        }
        if (pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_UP) == BS_PRESSED) 
        {
            SoundSystem::GetInstance().PlaySound("ui_hover"); // Example sound
            if (m_selectedUpgradeButtonIndex > 0) 
            {
                m_selectedUpgradeButtonIndex--;
            }
            else 
            {
                m_selectedUpgradeButtonIndex = static_cast<int>(m_upgradeButtons.size()) - 1;
            }
        }
    }

    Vector2 mousePos = inputSystem.GetMousePosition();
    int lastSelectedByMouse = -1;

    for (size_t i = 0; i < m_upgradeButtons.size(); ++i) 
    {
        auto& btn = m_upgradeButtons[i];
        if (btn.IsMouseOver(mousePos.x, mousePos.y)) 
        {
            btn.isHovered = true;
            lastSelectedByMouse = static_cast<int>(i);
            if (m_selectedUpgradeButtonIndex != lastSelectedByMouse)
            {
                // SoundSystem::GetInstance().PlaySound("ui_hover"); // Optional: sound on mouse hover changing selection
                m_selectedUpgradeButtonIndex = lastSelectedByMouse;
            }
        }
        else 
        {
            btn.isHovered = false;
        }

        bool isActiveButton = btn.isHovered || (pController && static_cast<int>(i) == m_selectedUpgradeButtonIndex);

        for (size_t i = 0; i < m_upgradeButtons.size(); ++i)
        {
            auto& btn = m_upgradeButtons[i];
            if (btn.textSprite)
            {
                if (isActiveButton)
                {
                    // Orange 
                    btn.textSprite->SetRedTint(1.0f);
                    btn.textSprite->SetGreenTint(0.647f);
                    btn.textSprite->SetBlueTint(0.0f);
                }
                else
                {
                    // White
                    btn.textSprite->SetRedTint(1.0f);
                    btn.textSprite->SetGreenTint(1.0f);
                    btn.textSprite->SetBlueTint(1.0f);
                }
            }
        }
    }
   
    if (lastSelectedByMouse == -1 && pController && m_selectedUpgradeButtonIndex >= 0 && m_selectedUpgradeButtonIndex < static_cast<int>(m_upgradeButtons.size()))
    {
        auto& selectedBtn = m_upgradeButtons[m_selectedUpgradeButtonIndex];
        if (selectedBtn.textSprite) 
        { 
            selectedBtn.textSprite->SetRedTint(1.0f); selectedBtn.textSprite->SetGreenTint(0.647f); selectedBtn.textSprite->SetBlueTint(0.0f);
        }
    }

    bool actionTriggered = false;
    std::string actionIdentifier = "";

    if (inputSystem.GetMouseButtonState(SDL_BUTTON_LEFT) == BS_PRESSED) 
    {
        for (const auto& btn : m_upgradeButtons) 
        {
            if (btn.isHovered) 
            {
                actionIdentifier = btn.identifier;
                actionTriggered = true;
                break;
            }
        }
    }

    if (!actionTriggered && pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_A) == BS_PRESSED) 
    {
        if (m_selectedUpgradeButtonIndex >= 0 && m_selectedUpgradeButtonIndex < static_cast<int>(m_upgradeButtons.size())) 
        {
            actionIdentifier = m_upgradeButtons[m_selectedUpgradeButtonIndex].identifier;
            actionTriggered = true;
        }
    }

    if (actionTriggered && !actionIdentifier.empty()) 
    {
        SoundSystem::GetInstance().PlaySound("titleButton");
        ActivateButtonAction(actionIdentifier);
    }
}

void UpgradeMenu::ActivateButtonAction(const std::string& identifier)
{
    if (!m_pPlayer || !m_pScene || !m_pScene->GetWaveSystem()) return; // Guard

    if (identifier == "done_upgrading") 
    {
        m_pScene->GetWaveSystem()->EndIntermission();
        SetActive(false); // Hide the menu
    }
    else 
    {
        StatType statToUpgrade = StatType::MAX_HEALTH; // Default
        bool found = false;
        for (const auto& btn : m_upgradeButtons) 
        {
            if (btn.identifier == identifier) 
            {
                statToUpgrade = btn.statToUpgrade;
                found = true;
                break;
            }
        }

        if (found && m_pPlayer->GetPlayerStats().AttemptUpgrade(statToUpgrade, m_pPlayer->GetAbyssalEssence())) 
        {
            if (statToUpgrade == StatType::MAX_HEALTH || statToUpgrade == StatType::MAX_STAMINA) 
            {
                m_pPlayer->UpdateStatsFromPlayerStats();
            }
            SetupUI();
        }
        else if (found) 
        {
            //SoundSystem::GetInstance().PlaySound("ui_fail_sound"); // Placeholder
        }
    }
}

void UpgradeMenu::Draw(Renderer& renderer)
{
    if (!m_bIsActive || !m_pRenderer) return;

    // Draw panel background
    renderer.DrawDebugRect(m_panelX, m_panelY, m_panelX + m_panelWidth, m_panelY + m_panelHeight, 10, 10, 10, 230);

    if (m_pUpgradeMenuTitleSprite) m_pUpgradeMenuTitleSprite->Draw(renderer);
    if (m_pEssenceTextSprite) m_pEssenceTextSprite->Draw(renderer);

    for (const auto& btn : m_upgradeButtons) 
    {
        unsigned char r = 40, g = 40, b = 45, a = 240;
        // Highlight if it's the actively selected button (either by hover or controller)
        bool isActiveButton = btn.isHovered || (&btn == &m_upgradeButtons[m_selectedUpgradeButtonIndex] && m_selectedUpgradeButtonIndex != -1);
        if (isActiveButton) { r = 70; g = 70; b = 75; }

        renderer.DrawDebugRect(btn.rect.x, btn.rect.y,
            btn.rect.x + btn.rect.width, btn.rect.y + btn.rect.height,
            r, g, b, a);
        if (btn.textSprite) 
        {
            btn.textSprite->Draw(renderer);
        }
    }
}