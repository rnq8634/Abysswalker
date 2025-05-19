// This include
#include "GameEndPrompt.h"

// Local include
#include "Renderer.h"
#include "Player.h"
#include "InputSystem.h"
#include "Sprite.h"      
#include "Texture.h"    
#include "LogManager.h"
#include "SoundSystem.h"
#include "SceneAbyssWalker.h"
#include "XboxController.h"
#include "Game.h"    

// Lib includes
#include <cstdio>

GameEndPrompt::GameEndPrompt(Renderer* renderer, Player* player, SceneAbyssWalker* scene)
    : m_pRenderer(renderer),
    m_pPlayer(player),
    m_pScene(scene),
    m_pGameEndTitleSprite(nullptr),
    m_pGameEndTitleTexture(nullptr),
    m_pGameEndReviveCostSprite(nullptr),
    m_pGameEndReviveCostTexture(nullptr),
    m_selectedGameEndButtonIndex(-1),
    m_bIsActive(false),
    m_currentTitleMessage("")
{
    if (!m_pRenderer || !m_pPlayer || !m_pScene) 
    {
        LogManager::GetInstance().Log("GameEndPrompt critical error: Null pointer passed in constructor.");
    }
}

GameEndPrompt::~GameEndPrompt()
{
    ClearUI();
}

void GameEndPrompt::SetActive(bool active, const std::string& titleMessage) 
{
    m_bIsActive = active;
    if (m_bIsActive) 
    {
        m_currentTitleMessage = titleMessage; // Store the title
        SetupUI(m_currentTitleMessage);
    }
    else 
    {
        ClearUI();
    }
}

void GameEndPrompt::ClearUI() 
{
    for (auto& btn : m_gameEndButtons) 
    {
        delete btn.textSprite; btn.textSprite = nullptr;
        delete btn.textTexture; btn.textTexture = nullptr;
    }
    m_gameEndButtons.clear();

    delete m_pGameEndTitleSprite; m_pGameEndTitleSprite = nullptr;
    delete m_pGameEndTitleTexture; m_pGameEndTitleTexture = nullptr;
    delete m_pGameEndReviveCostSprite; m_pGameEndReviveCostSprite = nullptr;
    delete m_pGameEndReviveCostTexture; m_pGameEndReviveCostTexture = nullptr;
    m_selectedGameEndButtonIndex = -1;
}

void GameEndPrompt::SetupUI(const std::string& titleMessage) 
{
    if (!m_pRenderer || !m_pPlayer || !m_pScene || !m_pScene->GetWaveSystem()) return;
    ClearUI(); // Clear previous elements

    WaveState currentWaveState = m_pScene->GetWaveSystem()->GetCurrentState(); 

    float centerX = m_pRenderer->GetWidth() / 2.0f;
    float menuY = m_pRenderer->GetHeight() / 2.0f - 100.0f;
    float buttonWidth = 250.0f;
    float buttonHeight = 40.0f;
    float spacing = 15.0f;

    // Title Message
    m_pGameEndTitleTexture = new Texture();
    m_pGameEndTitleSprite = new Sprite();
    if (m_pGameEndTitleSprite->InitialiseWithText(*m_pGameEndTitleTexture, titleMessage.c_str(), m_uiFontPath, m_uiTitleFontSize * 1.2f)) 
    {
        m_pGameEndTitleSprite->SetX(static_cast<int>(centerX));
        m_pGameEndTitleSprite->SetY(static_cast<int>(menuY));
    }
    menuY += m_pGameEndTitleTexture->GetHeight() + spacing * 1.5f;

    // Revive Button (if applicable)
    if (currentWaveState == WaveState::GAME_END_PROMPT && m_pPlayer->GetAbyssalEssence().CanRevive()) 
    {
        UIButton reviveBtn;
        std::string reviveText = "Revive (" + std::to_string(AbyssalEssence::DEFAULT_REVIVE_COST) + " Essence)";
        reviveBtn.textTexture = new Texture();
        reviveBtn.textSprite = new Sprite();
        if (reviveBtn.textSprite->InitialiseWithText(*reviveBtn.textTexture, reviveText.c_str(), m_uiFontPath, m_uiFontSize)) 
        {
            reviveBtn.rect = { centerX - buttonWidth / 2.0f, menuY, buttonWidth, buttonHeight };
            reviveBtn.identifier = "revive_player";
            reviveBtn.textSprite->SetX(static_cast<int>(centerX)); // Center text in button
            reviveBtn.textSprite->SetY(static_cast<int>(menuY + buttonHeight / 2.0f));
            m_gameEndButtons.push_back(std::move(reviveBtn));
        }
        else { delete reviveBtn.textSprite; delete reviveBtn.textTexture; }
        menuY += buttonHeight + spacing;
    }
    else if (currentWaveState == WaveState::GAME_END_PROMPT) { // Cannot revive
        m_pGameEndReviveCostTexture = new Texture();
        m_pGameEndReviveCostSprite = new Sprite();
        if (m_pGameEndReviveCostSprite->InitialiseWithText(*m_pGameEndReviveCostTexture, "Not enough essence to revive.", m_uiFontPath, m_uiFontSize)) 
        {
            m_pGameEndReviveCostSprite->SetX(static_cast<int>(centerX));
            m_pGameEndReviveCostSprite->SetY(static_cast<int>(menuY));
        }
        menuY += m_pGameEndReviveCostTexture->GetHeight() + spacing;
    }

    // Restart Button
    UIButton restartBtn;
    restartBtn.textTexture = new Texture();
    restartBtn.textSprite = new Sprite();
    if (restartBtn.textSprite->InitialiseWithText(*restartBtn.textTexture, "Restart Game", m_uiFontPath, m_uiFontSize)) 
    {
        restartBtn.rect = { centerX - buttonWidth / 2.0f, menuY, buttonWidth, buttonHeight };
        restartBtn.identifier = "restart_game";
        restartBtn.textSprite->SetX(static_cast<int>(centerX));
        restartBtn.textSprite->SetY(static_cast<int>(menuY + buttonHeight / 2.0f));
        m_gameEndButtons.push_back(std::move(restartBtn));
    }
    else { delete restartBtn.textSprite; delete restartBtn.textTexture; }
    menuY += buttonHeight + spacing;

    // Quit to Title Button
    UIButton quitTitleBtn;
    quitTitleBtn.textTexture = new Texture();
    quitTitleBtn.textSprite = new Sprite();
    if (quitTitleBtn.textSprite->InitialiseWithText(*quitTitleBtn.textTexture, "Quit to Title", m_uiFontPath, m_uiFontSize)) 
    {
        quitTitleBtn.rect = { centerX - buttonWidth / 2.0f, menuY, buttonWidth, buttonHeight };
        quitTitleBtn.identifier = "quit_title";
        quitTitleBtn.textSprite->SetX(static_cast<int>(centerX));
        quitTitleBtn.textSprite->SetY(static_cast<int>(menuY + buttonHeight / 2.0f));
        m_gameEndButtons.push_back(std::move(quitTitleBtn));
    }
    else { delete quitTitleBtn.textSprite; delete quitTitleBtn.textTexture; }
    menuY += buttonHeight + spacing;

    // Quit Game Button
    UIButton quitGameBtn;
    quitGameBtn.textTexture = new Texture();
    quitGameBtn.textSprite = new Sprite();
    if (quitGameBtn.textSprite->InitialiseWithText(*quitGameBtn.textTexture, "Quit Game", m_uiFontPath, m_uiFontSize)) 
    {
        quitGameBtn.rect = { centerX - buttonWidth / 2.0f, menuY, buttonWidth, buttonHeight };
        quitGameBtn.identifier = "quit_game";
        quitGameBtn.textSprite->SetX(static_cast<int>(centerX));
        quitGameBtn.textSprite->SetY(static_cast<int>(menuY + buttonHeight / 2.0f));
        m_gameEndButtons.push_back(std::move(quitGameBtn));
    }
    else { delete quitGameBtn.textSprite; delete quitGameBtn.textTexture; }

    m_selectedGameEndButtonIndex = m_gameEndButtons.empty() ? -1 : 0;
}

void GameEndPrompt::UpdateUI(InputSystem& inputSystem) 
{
    if (!m_bIsActive || !m_pPlayer || m_gameEndButtons.empty()) 
    {
        if (m_gameEndButtons.empty()) m_selectedGameEndButtonIndex = -1;
        return;
    }

    XboxController* pController = nullptr;
    if (inputSystem.GetNumberOfControllersAttached() > 0) 
    {
        pController = inputSystem.GetController(0);
    }

    // Controller Navigation
    if (pController) 
    {
        if (pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_DOWN) == BS_PRESSED)
        {
            SoundSystem::GetInstance().PlaySound("ui_hover");
            if (m_selectedGameEndButtonIndex < static_cast<int>(m_gameEndButtons.size()) - 1) 
            {
                m_selectedGameEndButtonIndex++;
            }
            else 
            {
                m_selectedGameEndButtonIndex = 0;
            }
        }
        if (pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_UP) == BS_PRESSED) 
        {
            SoundSystem::GetInstance().PlaySound("ui_hover");
            if (m_selectedGameEndButtonIndex > 0) {
                m_selectedGameEndButtonIndex--;
            }
            else 
            {
                m_selectedGameEndButtonIndex = static_cast<int>(m_gameEndButtons.size()) - 1;
            }
        }
    }

    Vector2 mousePos = inputSystem.GetMousePosition();
    int lastSelectedByMouse = -1;

    for (size_t i = 0; i < m_gameEndButtons.size(); ++i) 
    {
        auto& btn = m_gameEndButtons[i];
        if (btn.IsMouseOver(mousePos.x, mousePos.y)) 
        {
            btn.isHovered = true;
            lastSelectedByMouse = static_cast<int>(i);
            if (m_selectedGameEndButtonIndex != static_cast<int>(i)) 
            {
                m_selectedGameEndButtonIndex = static_cast<int>(i);
            }
        }
        else 
        {
            btn.isHovered = false;
        }
    }

    for (size_t i = 0; i < m_gameEndButtons.size(); ++i) 
    {
        auto& btn = m_gameEndButtons[i];
        if (btn.textSprite) 
        {
            if (static_cast<int>(i) == m_selectedGameEndButtonIndex) 
            {
                btn.textSprite->SetRedTint(1.0f); btn.textSprite->SetGreenTint(0.647f); btn.textSprite->SetBlueTint(0.0f);
            }
            else 
            {
                btn.textSprite->SetRedTint(1.0f); btn.textSprite->SetGreenTint(1.0f); btn.textSprite->SetBlueTint(1.0f);
            }
        }
    }

    bool actionTriggered = false;
    std::string actionIdentifier = "";

    if (inputSystem.GetMouseButtonState(SDL_BUTTON_LEFT) == BS_PRESSED) 
    {
        if (m_selectedGameEndButtonIndex != -1 && m_gameEndButtons[m_selectedGameEndButtonIndex].isHovered) 
        {
            actionIdentifier = m_gameEndButtons[m_selectedGameEndButtonIndex].identifier;
            actionTriggered = true;
        }
    }

    if (!actionTriggered && pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_A) == BS_PRESSED) 
    {
        if (m_selectedGameEndButtonIndex >= 0 && m_selectedGameEndButtonIndex < static_cast<int>(m_gameEndButtons.size())) 
        {
            actionIdentifier = m_gameEndButtons[m_selectedGameEndButtonIndex].identifier;
            actionTriggered = true;
        }
    }

    if (actionTriggered && !actionIdentifier.empty()) 
    {
        SoundSystem::GetInstance().PlaySound("titleButton");
        ActivateButtonAction(actionIdentifier);
    }
}

void GameEndPrompt::ActivateButtonAction(const std::string& identifier) 
{
    if (!m_pPlayer || !m_pScene || !m_pScene->GetWaveSystem()) return;

    if (identifier == "revive_player") 
    {
        m_pPlayer->Revive();
        if (m_pPlayer->IsAlive()) 
        {
            SetActive(false);
            
            m_pScene->GetWaveSystem()->ResetWaveTimerForPreWave();
            m_pScene->GetWaveSystem()->SetEnemiesKilledThisWave(0);
            m_pScene->GetWaveSystem()->TransitionToState(WaveState::PRE_WAVE_DELAY);
        }
        else 
        {
            SetActive(true, "YOU DIED!");
        }
    }
    else if (identifier == "restart_game") 
    {
        Game::GetInstance().SetCurrentScene(SCENE_INDEX_ABYSSWALKER);
        SetActive(false);
    }
    else if (identifier == "quit_title") 
    {
        Game::GetInstance().SetCurrentScene(SCENE_INDEX_TITLE);
    }
    else if (identifier == "quit_game") 
    {
        m_pScene->PlayerRequestsQuit();
    }
}

void GameEndPrompt::Draw(Renderer& renderer) 
{
    if (!m_bIsActive || !m_pRenderer) return;

    // Draw panel background
    float panelWidth = 400.0f;
    // Calculate panel height dynamically or set fixed
    float panelHeight = 0.0f;
    if (m_pGameEndTitleSprite) panelHeight += m_pGameEndTitleTexture->GetHeight() + 15.0f * 1.5f;
    if (m_pGameEndReviveCostSprite) panelHeight += m_pGameEndReviveCostTexture->GetHeight() + 15.0f;
    else if (m_pPlayer && m_pPlayer->GetAbyssalEssence().CanRevive() && m_gameEndButtons.size() > 0 && m_gameEndButtons[0].identifier == "revive_player") panelHeight += m_gameEndButtons[0].rect.height + 15.0f; // approx revive button

    panelHeight += (m_gameEndButtons.size() * (40.0f + 15.0f)); // For other buttons
    if (panelHeight < 250.0f) panelHeight = 250.0f; // Minimum height


    float panelX = m_pRenderer->GetWidth() / 2.0f - panelWidth / 2.0f;
    float panelY = m_pRenderer->GetHeight() / 2.0f - panelHeight / 2.0f; // Center panel better

    renderer.DrawDebugRect(panelX, panelY, panelX + panelWidth, panelY + panelHeight, 15, 15, 15, 235);

    if (m_pGameEndTitleSprite) m_pGameEndTitleSprite->Draw(renderer);
    if (m_pGameEndReviveCostSprite) m_pGameEndReviveCostSprite->Draw(renderer);

    for (size_t i = 0; i < m_gameEndButtons.size(); ++i) 
    {
        const auto& btn = m_gameEndButtons[i];
        unsigned char r = 40, g = 40, b = 45, a = 240;
        if (static_cast<int>(i) == m_selectedGameEndButtonIndex) 
        { // Use selected index for highlight
            r = 70; g = 70; b = 75;
        }
        renderer.DrawDebugRect(btn.rect.x, btn.rect.y,
            btn.rect.x + btn.rect.width, btn.rect.y + btn.rect.height,
            r, g, b, a);
        if (btn.textSprite) 
        {
            btn.textSprite->Draw(renderer);
        }
    }
}