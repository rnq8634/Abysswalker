// This inlcude
#include "SceneAbyssWalker.h"

// Local Include
#include "Player.h"
#include "EnemyType2.h"
#include "EnemyBat.h" 
#include "Renderer.h"
#include "InputSystem.h"
#include "Sprite.h"
#include "LogManager.h" 
#include "AnimatedSprite.h" 
#include "SoundSystem.h"
#include "Game.h"
#include "Texture.h"
#include "XboxController.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <algorithm>
#include <cstdio>

SceneAbyssWalker::SceneAbyssWalker()
    : m_pPlayer(nullptr)
    , m_pRenderer(nullptr)
    , m_pGameEndTitleSprite(nullptr)
    , m_pGameEndTitleTexture(nullptr)
    , m_pGameEndReviveCostSprite(nullptr)
    , m_pGameEndReviveCostTexture(nullptr)
    , m_batSpawnTimer(0.0f)
    , m_type2SpawnTimer(0.0f)
    , m_bShowHitboxes(false)
    , m_playerChoseToQuit(false)
    , m_selectedUpgradeButtonIndex(-1)
    , m_selectedGameEndButtonIndex(-1)
    , m_pWaveSystem(nullptr)
{
    m_pmoonBackground = nullptr;
}

SceneAbyssWalker::~SceneAbyssWalker()
{
    delete m_pWaveSystem;
    m_pWaveSystem = nullptr;

    delete m_pPlayer;
    m_pPlayer = nullptr;

    for (EnemyBat* enemyBat : m_enemyBats)
    {
        delete enemyBat;
    }
    m_enemyBats.clear();

    for (EnemyType2* enemyType2 : m_enemyType2)
    {
        delete enemyType2;
    }
    m_enemyType2.clear();

    // Background
    delete m_pmoonBackground; 
    m_pmoonBackground = nullptr;

    ClearUpgradeMenuUI();
    ClearGameEndPromptUI();
}

void SceneAbyssWalker::fullBackground(Renderer& renderer)
{
    m_pmoonBackground = renderer.CreateSprite("assets/backgrounds/main_background.png");

    const int screenWidth = renderer.GetWidth();
    const int screenHeight = renderer.GetHeight();
    const int screenCenterX = screenWidth / 2;
    const int screenBottomY = screenHeight;

    if (m_pmoonBackground)
    {
        float scaleX = static_cast<float>(screenWidth) / m_pmoonBackground->GetOriginalWidth();
        float scaleY = static_cast<float>(screenHeight) / m_pmoonBackground->GetOriginalHeight();

        m_pmoonBackground->SetX(screenCenterX);
        m_pmoonBackground->SetY(screenHeight / 2);
        m_pmoonBackground->SetScale(scaleX, scaleY);
        m_pmoonBackground->SetFlipHorizontal(true);
        m_pmoonBackground->SetAngle(180.0f);
    }
}

bool SceneAbyssWalker::Initialise(Renderer& renderer)
{
    m_pRenderer = &renderer;
    fullBackground(*m_pRenderer);

    // Load Player
    m_pPlayer = new Player();
    if (!m_pPlayer || !m_pPlayer->Initialise(*m_pRenderer))
    {
        LogManager::GetInstance().Log("Failed to initialise Player.");
        delete m_pPlayer; m_pPlayer = nullptr;
        return false;
    }
    m_pPlayer->ResetForNewGame();

    m_pUpgradeMenu = new UpgradeMenu(m_pRenderer, m_pPlayer, this);
    if (!m_pUpgradeMenu)
    {
        LogManager::GetInstance().Log("Failed to load UpgradeMenu!!");
        return false;
    }

    // Load Wave System
    m_pWaveSystem = new WaveSystem(this, m_pPlayer);
    if (!m_pWaveSystem)
    {
        LogManager::GetInstance().Log("Failed to create WaveManager.");
        return false;
    }

    m_batSpawnTimer = m_batSpawnInterval; // Start ready to spawn enemies
    m_type2SpawnTimer = m_type2SpawnInterval;

    m_playerChoseToQuit = false;

    SetupUpgradeMenuUI();

    return true;
}

void SceneAbyssWalker::RestartGame()
{
    LogManager::GetInstance().Log("Restarting game...");
    // Clear existing enemies
    for (EnemyBat* enemyBat : m_enemyBats) delete enemyBat;
    m_enemyBats.clear();
    for (EnemyType2* enemyType2 : m_enemyType2) delete enemyType2;
    m_enemyType2.clear();

    if (m_pPlayer) m_pPlayer->ResetForNewGame();

    if (m_pWaveSystem) m_pWaveSystem->ResetForNewGame();

    m_playerChoseToQuit = false;
}

void SceneAbyssWalker::Process(float deltaTime, InputSystem& inputSystem)
{
    if (!m_pPlayer || !m_pRenderer || !m_pWaveSystem) return;
    if (m_playerChoseToQuit)
    {
        Game::GetInstance().Quit();
        return;
    }

    m_pWaveSystem->Process(deltaTime, inputSystem);
    WaveState currentWaveState = m_pWaveSystem->GetCurrentState();

    if (currentWaveState == WaveState::INTERMISSION)
    {
        UpdateUpgradeMenuUI(inputSystem);
    }

    if (currentWaveState == WaveState::IN_WAVE ||
        currentWaveState == WaveState::INTERMISSION ||
        currentWaveState == WaveState::PRE_WAVE_DELAY)
    {
        // Player input processing
        const float moveSpeed = 150.0f;
        const float rollSpeed = 200.0f;
        bool isMoving = false;

        bool isPlayerAttemptingMove = false;
        PlayerState stateBeforePlayerInputAndProcess = PlayerState::IDLE;

        if (m_pPlayer)
        {
            stateBeforePlayerInputAndProcess = m_pPlayer->GetCurrentState();
        }

        XboxController* pController = nullptr;
        if (inputSystem.GetNumberOfControllersAttached() > 0)
        {
            pController = inputSystem.GetController(0);
        }

        if (m_pPlayer && m_pPlayer->IsAlive())
        {
            // Player Move Left
            if (inputSystem.GetKeyState(SDL_SCANCODE_A) == BS_HELD || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_LEFT) == BS_HELD))
            {
                m_pPlayer->MoveLeft(moveSpeed);
                isMoving = true;
            }

            // Player Move Right
            else if (inputSystem.GetKeyState(SDL_SCANCODE_D) == BS_HELD || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_RIGHT) == BS_HELD))
            {
                m_pPlayer->MoveRight(moveSpeed);
                isMoving = true;
            }

            // Player Jump
            if (inputSystem.GetKeyState(SDL_SCANCODE_SPACE) == BS_PRESSED || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_A) == BS_PRESSED))
            {
                m_pPlayer->Jump();
            }

            // Player Attack
            if (inputSystem.GetKeyState(SDL_SCANCODE_J) == BS_PRESSED || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_X) == BS_PRESSED))
            {
                m_pPlayer->Attack();
                // add sound ^^^ like jump
            }

            // Player Roll
            if (inputSystem.GetKeyState(SDL_SCANCODE_Q) == BS_PRESSED || (pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_B) == BS_PRESSED))
            {
                m_pPlayer->Roll(rollSpeed);
                // add sound here
            }

            if (!isMoving &&
                m_pPlayer->IsAlive() &&
                m_pPlayer->GetCurrentState() != PlayerState::JUMPING &&
                m_pPlayer->GetCurrentState() != PlayerState::FALLING &&
                m_pPlayer->GetCurrentState() != PlayerState::TURNING &&
                m_pPlayer->GetCurrentState() != PlayerState::ROLLING &&
                m_pPlayer->GetCurrentState() != PlayerState::HURT &&
                m_pPlayer->GetCurrentState() != PlayerState::ATTACKING)
            {
                m_pPlayer->StopMoving();
            }
        }

        m_pPlayer->Process(deltaTime);

        bool processEnemies = (currentWaveState == WaveState::IN_WAVE);

        for (EnemyBat* enemyBat : m_enemyBats) 
        {
            if (enemyBat->IsAlive() && processEnemies) 
            { // Only process AI and movement if wave active
                if (enemyBat->m_pTargetPlayer == nullptr && m_pPlayer) enemyBat->m_pTargetPlayer = m_pPlayer;
                enemyBat->Process(deltaTime);
            }
            else if (!enemyBat->IsAlive() && enemyBat->GetCurrentState() == EnemyBatState::DEATH) 
            {
                // If dead and in death state, just process animation
                AnimatedSprite* sprite = enemyBat->GetCurrentAnimatedSprite();
                if (sprite) enemyBat->UpdateSprite(sprite, deltaTime); // Assuming UpdateSprite handles animation process
            }
        }

        // Similar loop for EnemyType2
        for (EnemyType2* enemyT2 : m_enemyType2) 
        {
            if (enemyT2->IsAlive() && processEnemies) 
            {
                if (enemyT2->m_pTargetPlayer == nullptr && m_pPlayer) enemyT2->m_pTargetPlayer = m_pPlayer;
                enemyT2->Process(deltaTime);
            }
            else if (!enemyT2->IsAlive() && enemyT2->GetCurrentState() == EnemyType2State::DEATH) 
            {
                AnimatedSprite* sprite = enemyT2->GetCurrentAnimatedSprite();
                if (sprite) enemyT2->UpdateSprite(sprite, deltaTime);
            }
        }

        if (processEnemies) 
        {
            HandleCollisions();
            if (m_pPlayer->IsAlive()) UpdateSpawning(deltaTime);
        }

        CleanupDead();
    }
}

void SceneAbyssWalker::EndWaveEnemyCleanup()
{
    for (EnemyBat* bat : m_enemyBats)
    {
        if (bat->IsAlive())
        {
            bat->TransitionToState(EnemyBatState::DEATH);
            bat->SetDead();
        }
    }
    for (EnemyType2* t2 : m_enemyType2)
    {
        if (t2->IsAlive())
        {
            t2->TransitionToState(EnemyType2State::DEATH);
            t2->SetDead();
        }
    }
}

void SceneAbyssWalker::ClearUpgradeMenuUI() 
{
    if (m_pUpgradeMenu)
    {
        m_pUpgradeMenu->SetActive(false);
    }
}

void SceneAbyssWalker::SetupUpgradeMenuUI() 
{
    if (m_pUpgradeMenu)
    {
        m_pUpgradeMenu->SetActive(true);
    }
}


void SceneAbyssWalker::UpdateUpgradeMenuUI(InputSystem& inputSystem) 
{
    if (m_pUpgradeMenu && m_pUpgradeMenu->IsActive())
    {
        m_pUpgradeMenu->UpdateUI(inputSystem);
    }
}

void SceneAbyssWalker::DrawUpgradeMenu(Renderer& renderer) 
{
    if (m_pUpgradeMenu && m_pUpgradeMenu->IsActive())
    {
        m_pUpgradeMenu->Draw(renderer);
    }
}

// --- Game End Prompts (Custom UI) ---
void SceneAbyssWalker::ClearGameEndPromptUI() 
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
}

void SceneAbyssWalker::SetupGameEndPromptUI(const std::string& titleMessage) 
{
    if (!m_pRenderer || !m_pPlayer) return;
    ClearGameEndPromptUI();

    WaveState currentActualWaveState = m_pWaveSystem->GetCurrentState();

    float centerX = m_pRenderer->GetWidth() / 2.0f;
    float menuY = m_pRenderer->GetHeight() / 2.0f - 100.0f;
    float buttonWidth = 250.0f;
    float buttonHeight = 40.0f;
    float spacing = 15.0f;

    // Title Message
    m_pGameEndTitleTexture = new Texture();
    m_pGameEndTitleSprite = new Sprite();
    if (m_pGameEndTitleSprite->InitialiseWithText(*m_pGameEndTitleTexture, titleMessage.c_str(), m_uiFontPath, m_uiTitleFontSize * 1.2f)) 
    { // Larger font
        m_pGameEndTitleSprite->SetX(static_cast<int>(centerX));
        m_pGameEndTitleSprite->SetY(static_cast<int>(menuY));
    }
    menuY += buttonHeight + spacing * 2;

    // Revive Button (if applicable)
    if (currentActualWaveState == WaveState::GAME_END_PROMPT && m_pPlayer->GetAbyssalEssence().CanRevive())
    {
        UIButton reviveBtn;
        std::string reviveText = "Revive (" + std::to_string(AbyssalEssence::DEFAULT_REVIVE_COST) + " Essence)";
        reviveBtn.textTexture = new Texture();
        reviveBtn.textSprite = new Sprite();
        if (reviveBtn.textSprite->InitialiseWithText(*reviveBtn.textTexture, reviveText.c_str(), m_uiFontPath, m_uiFontSize)) 
        {
            reviveBtn.rect = { centerX - buttonWidth / 2.0f, menuY, buttonWidth, buttonHeight };
            reviveBtn.identifier = "revive_player";
            reviveBtn.textSprite->SetX(static_cast<int>(centerX));
            reviveBtn.textSprite->SetY(static_cast<int>(menuY + buttonHeight / 2.0f));
            m_gameEndButtons.push_back(reviveBtn);
        }
        else { delete reviveBtn.textSprite; delete reviveBtn.textTexture; }
        menuY += buttonHeight + spacing;
    }
    else if (currentActualWaveState == WaveState::GAME_END_PROMPT)
    {
        // "Not enough essence" text if cannot revive
        m_pGameEndReviveCostTexture = new Texture();
        m_pGameEndReviveCostSprite = new Sprite();
        if (m_pGameEndReviveCostSprite->InitialiseWithText(*m_pGameEndReviveCostTexture, "Not enough essence to revive.", m_uiFontPath, m_uiFontSize)) 
        {
            m_pGameEndReviveCostSprite->SetX(static_cast<int>(centerX));
            m_pGameEndReviveCostSprite->SetY(static_cast<int>(menuY));
        }
        menuY += buttonHeight + spacing;
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
        m_gameEndButtons.push_back(restartBtn);
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
        m_gameEndButtons.push_back(quitTitleBtn);
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
        m_gameEndButtons.push_back(quitGameBtn);
    }
    else { delete quitGameBtn.textSprite; delete quitGameBtn.textTexture; }

    m_selectedGameEndButtonIndex = m_gameEndButtons.empty() ? -1 : 0;
}

void SceneAbyssWalker::UpdateGameEndPromptUI(InputSystem& inputSystem) 
{
    if (!m_pPlayer || m_gameEndButtons.empty())
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
            if (m_selectedGameEndButtonIndex < static_cast<int>(m_gameEndButtons.size()) - 1)
            {
                m_selectedGameEndButtonIndex++;
            }
            else
            {
                m_selectedGameEndButtonIndex = 0; // Wrap around
            }
            // Sound for moving around the buttons
            //SoundSystem::GetInstance().PlaySound("ui_nav_sound"); // Placeholder
        }
        if (pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_UP) == BS_PRESSED)
        {
            if (m_selectedGameEndButtonIndex > 0)
            {
                m_selectedGameEndButtonIndex--;
            }
            else
            {
                m_selectedGameEndButtonIndex = static_cast<int>(m_gameEndButtons.size()) - 1;
            }
            // Sound for moving around the buttons
            //SoundSystem::GetInstance().PlaySound("ui_nav_sound"); // Placeholder
        }
    }

    Vector2 mousePos = inputSystem.GetMousePosition();

    for (size_t i = 0; i < m_gameEndButtons.size(); ++i)
    {
        auto& btn = m_gameEndButtons[i];
        btn.isHovered = btn.IsMouseOver(mousePos.x, mousePos.y);

        bool isActive = btn.isHovered || (pController && static_cast<int>(i) == m_selectedGameEndButtonIndex);

        if (btn.textSprite)
        {
            if (isActive)
            {
                btn.textSprite->SetRedTint(1.0f); 
                btn.textSprite->SetGreenTint(0.647f); 
                btn.textSprite->SetBlueTint(0.0f);
            }
            else
            {
                btn.textSprite->SetRedTint(1.0f); 
                btn.textSprite->SetGreenTint(1.0f);
                btn.textSprite->SetBlueTint(1.0f);
            }
        }
    }

    bool actionTriggered = false;
    std::string actionIdentifier = "";

    // Mouse clicks
    if (inputSystem.GetMouseButtonState(SDL_BUTTON_LEFT) == BS_PRESSED)
    {
        for (const auto& btn : m_gameEndButtons)
        {
            if (btn.isHovered) {
                actionIdentifier = btn.identifier;
                actionTriggered = true;
                break;
            }
        }
    }

    // Controller clicks
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
        ActivateGameEndButtonAction(actionIdentifier);
    }
}

void SceneAbyssWalker::ActivateGameEndButtonAction(const std::string& identifier)
{
    if (identifier == "revive_player")
    {
        m_pPlayer->Revive();
        if (m_pPlayer->IsAlive())
        {
            ClearGameEndPromptUI();
            m_selectedGameEndButtonIndex = -1;
            if (m_pWaveSystem)
            {
                m_pWaveSystem->ResetWaveTimerForPreWave();
                m_pWaveSystem->SetEnemiesKilledThisWave(0);
                m_pWaveSystem->TransitionToState(WaveState::PRE_WAVE_DELAY);
            }
        }
        else
        {
            SetupGameEndPromptUI("Defeated!");
        }
    }
    else if (identifier == "restart_game")
    {
        Game::GetInstance().SetCurrentScene(SCENE_INDEX_ABYSSWALKER);
    }
    else if (identifier == "quit_title")
    {
        Game::GetInstance().SetCurrentScene(SCENE_INDEX_TITLE);
    }
    else if (identifier == "quit_game")
    {
        m_playerChoseToQuit = true;
    }
}

void SceneAbyssWalker::DrawEndGamePrompts(Renderer& renderer) 
{
    if (!m_pWaveSystem || (m_pWaveSystem->GetCurrentState() != WaveState::GAME_WON && m_pWaveSystem->GetCurrentState() != WaveState::GAME_END_PROMPT))
    {
        return;
    }

    // Draw panel background (optional, could be full screen tint)
    float panelWidth = 400.0f;
    float panelHeight = 300.0f;
    float panelX = m_pRenderer->GetWidth() / 2.0f - panelWidth / 2.0f;
    float panelY = m_pRenderer->GetHeight() / 2.0f - panelHeight / 2.0f;
    renderer.DrawDebugRect(panelX, panelY, panelX + panelWidth, panelY + panelHeight, 15, 15, 15, 235);

    if (m_pGameEndTitleSprite) m_pGameEndTitleSprite->Draw(renderer);
    if (m_pGameEndReviveCostSprite) m_pGameEndReviveCostSprite->Draw(renderer);


    for (const auto& btn : m_gameEndButtons) 
    {
        unsigned char r = 40, g = 40, b = 45;
        if (btn.isHovered) { r = 70; g = 70; b = 75; }
        renderer.DrawDebugRect(btn.rect.x, btn.rect.y,
            btn.rect.x + btn.rect.width, btn.rect.y + btn.rect.height,
            r, g, b, 240);
        if (btn.textSprite) 
        {
            btn.textSprite->Draw(renderer);
        }
    }
}

void SceneAbyssWalker::NotifyEnemyKilled()
{
    if (m_pWaveSystem)
    {
        m_pWaveSystem->NotifyEnemyKilled();
    }
}

void SceneAbyssWalker::CleanupDead()
{
    // Cleaning up the enemy bats
    WaveState currentWaveState = WaveState::PRE_WAVE_DELAY;

    if (m_pWaveSystem)
    {
        currentWaveState = m_pWaveSystem->GetCurrentState();
    }

    m_enemyBats.erase(std::remove_if(m_enemyBats.begin(), m_enemyBats.end(), [&](EnemyBat* enemyBat)
        {
            if (!enemyBat->IsAlive())
            {
                AnimatedSprite* sprite = enemyBat->GetCurrentAnimatedSprite();
                bool animComplete = (sprite && !sprite->IsLooping() && sprite->IsAnimationComplete());
                bool noSprite = !sprite;
                bool forceClean = (currentWaveState != WaveState::IN_WAVE && enemyBat->GetCurrentState() == EnemyBatState::DEATH);

                if (animComplete || noSprite || forceClean)
                {
                    LogManager::GetInstance().Log("Cleaning up dead Bat.");
                    delete enemyBat;
                    return true;
                }
            }
            return false;
        }), m_enemyBats.end());

    // Cleaning up Type 2's
    m_enemyType2.erase(std::remove_if(m_enemyType2.begin(), m_enemyType2.end(),
        [&](EnemyType2* enemyT2) 
        {
            if (!enemyT2->IsAlive()) 
            {
                AnimatedSprite* sprite = enemyT2->GetCurrentAnimatedSprite();
                bool animComplete = (sprite && !sprite->IsLooping() && sprite->IsAnimationComplete());
                bool noSprite = !sprite;
                bool forceClean = (currentWaveState != WaveState::IN_WAVE && enemyT2->GetCurrentState() == EnemyType2State::DEATH);

                if (animComplete || noSprite || forceClean) 
                {
                    LogManager::GetInstance().Log("Cleaning up dead EnemyType2.");
                    delete enemyT2;
                    return true;
                }
            }
            return false;
        }), m_enemyType2.end());
}

void SceneAbyssWalker::HandleCollisions()
{
    if (!m_pPlayer || !m_pPlayer->IsAlive()) return;

    const float PLAYER_ATTACK_REACH = 25.0f;
    int playerAttackDamage = m_pPlayer->GetAttackDamage();

    for (EnemyBat* enemyBat : m_enemyBats)
    {
        if (!enemyBat->IsAlive()) continue;

        if (m_pPlayer->IsAlive())
        {
            // Check for player getting hit by enemy
            if (m_pPlayer->GetCurrentState() != PlayerState::ROLLING &&
                m_pPlayer->GetCurrentState() != PlayerState::HURT &&
                !m_pPlayer->IsInvincible() &&
                enemyBat->IsAttacking() &&
                m_pPlayer->CheckCollision(*enemyBat))
            {
                //SoundSystem::GetInstance().PlaySound(SF_PLAYER_HURT);
            }

            // Check for player attacking enemy
            if (m_pPlayer->GetCurrentState() == PlayerState::ATTACKING)
            {
                AnimatedSprite* playerSprite = m_pPlayer->GetCurrentAnimatedSprite();
                if (playerSprite)
                {
                    int currentFrame = playerSprite->GetCurrentFrame();
                    bool isHitFrame = (currentFrame >= 2 && currentFrame <= 5);

                    if (isHitFrame)
                    {
                        Vector2 playerPos = m_pPlayer->GetPosition();

                        float pAttackMinX = m_pPlayer->IsFacingRight() ?
                            playerPos.x :
                            playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                        float pAttackMaxX = m_pPlayer->IsFacingRight() ?
                            playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH) :
                            playerPos.x;
                        float pAttackMinY = playerPos.y - (Player::PLAYER_SPRITE_HEIGHT / 2.0f);
                        float pAttackMaxY = playerPos.y + (Player::PLAYER_SPRITE_HEIGHT / 2.0f);

                        Vector2 enemyBatPos = enemyBat->GetPosition();
                        float enemyRadius = enemyBat->GetRadius();
                        float eMinX = enemyBatPos.x - enemyRadius;
                        float eMaxX = enemyBatPos.x + enemyRadius;
                        float eMinY = enemyBatPos.y - enemyRadius;
                        float eMaxY = enemyBatPos.y + enemyRadius;

                        bool overlapX = pAttackMinX < eMaxX && pAttackMaxX > eMinX;
                        bool overlapY = pAttackMinY < eMaxY && pAttackMaxY > eMinY;

                        if (overlapX && overlapY)
                        {
                            if (m_pPlayer->DamageDoneToTarget(enemyBat))
                            {
                                enemyBat->TakeDamage(playerAttackDamage); // Player attack damage to enemyBat
                                // hurt sounds for bats
                            }
                        }
                    }
                }
            }
        }

        // Collisions for Player Vs EnemyType2
        for (EnemyType2* enemyT2 : m_enemyType2)
        {
            if (!enemyT2->IsAlive()) continue;

            if (m_pPlayer->GetCurrentState() != PlayerState::ROLLING &&
                m_pPlayer->GetCurrentState() != PlayerState::HURT &&
                !m_pPlayer->IsInvincible() &&
                enemyT2->IsAttacking() &&
                m_pPlayer->CheckCollision(*enemyT2))
            {
                //SoundSystem::GetInstance().PlaySound(SF_PLAYER_HURT);
            }

            // Check if player attacks Type2
            if (m_pPlayer->GetCurrentState() == PlayerState::ATTACKING)
            {
                AnimatedSprite* playerSprite = m_pPlayer->GetCurrentAnimatedSprite();
                if (playerSprite)
                {
                    Vector2 playerPos = m_pPlayer->GetPosition();
                    float pAttackMinX = m_pPlayer->IsFacingRight() ? playerPos.x : playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                    float pAttackMaxX = m_pPlayer->IsFacingRight() ? playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH) : playerPos.x;
                    float pAttackMinY = playerPos.y - (Player::PLAYER_SPRITE_HEIGHT / 2.0f);
                    float pAttackMaxY = playerPos.y + (Player::PLAYER_SPRITE_HEIGHT / 2.0f);

                    Vector2 enemyT2Position = enemyT2->GetPosition();
                    float enemyRadius = enemyT2->GetRadius();
                    float eMinX = enemyT2Position.x - enemyRadius;
                    float eMaxX = enemyT2Position.x + enemyRadius;
                    float eMinY = enemyT2Position.y - enemyRadius;
                    float eMaxY = enemyT2Position.y + enemyRadius;

                    bool overlapX = pAttackMinX < eMaxX && pAttackMaxX > eMinX;
                    bool overlapY = pAttackMinY < eMaxY && pAttackMaxY > eMinY;

                    if (overlapX && overlapY)
                    {
                        if (m_pPlayer->DamageDoneToTarget(enemyT2))
                        {
                            enemyT2->TakeDamage(playerAttackDamage);
                            // hurt sounds for T2
                        }
                    }
                }
            }
        }
    }
}

void SceneAbyssWalker::UpdateSpawning(float deltaTime)
{
    if (!m_pRenderer || !m_pWaveSystem || m_pWaveSystem->GetCurrentState() != WaveState::IN_WAVE) return;

    size_t totalCurrentEnemies = m_enemyBats.size() + m_enemyType2.size();

    // Bats
    if (totalCurrentEnemies < static_cast<size_t>(m_maxEnemies) && m_enemyBats.size() < static_cast<size_t>(m_maxBats)) {
        m_batSpawnTimer += deltaTime;
        if (m_batSpawnTimer >= m_batSpawnInterval) 
        {
            m_batSpawnTimer = 0.0f;
            // Simplified: alternate sides for now, or pick random
            SpawnEnemy(EnemySpawnType::BAT, rand() % 2 == 0);
        }
    }
    // Type2
    if (totalCurrentEnemies < static_cast<size_t>(m_maxEnemies) && m_enemyType2.size() < static_cast<size_t>(m_maxType2)) {
        m_type2SpawnTimer += deltaTime;
        if (m_type2SpawnTimer >= m_type2SpawnInterval) 
        {
            m_type2SpawnTimer = 0.0f;
            SpawnEnemy(EnemySpawnType::TYPE2, rand() % 2 == 0);
        }
    }
}

void SceneAbyssWalker::SpawnEnemy(EnemySpawnType type, bool spawnOnLeft)
{
    if (!m_pRenderer || !m_pPlayer) return;

    size_t totalCurrentEnemies = m_enemyBats.size() + m_enemyType2.size();
    if (totalCurrentEnemies >= static_cast<size_t>(m_maxEnemies)) return;

    Vector2 spawnPos;
    const float spawnXOffset = 100.0f; // Further off-screen
    spawnPos.x = spawnOnLeft ? -spawnXOffset : static_cast<float>(m_pRenderer->GetWidth()) + spawnXOffset;

    if (type == EnemySpawnType::BAT && m_enemyBats.size() < static_cast<size_t>(m_maxBats)) 
    {
        EnemyBat* newEnemyBat = new EnemyBat();
        if (!newEnemyBat) { LogManager::GetInstance().Log("Failed to allocate Bat"); return; }
        newEnemyBat->SetSceneReference(this);
        spawnPos.y = newEnemyBat->kGroundLevel;
        if (newEnemyBat->Initialise(*m_pRenderer, spawnPos)) m_enemyBats.push_back(newEnemyBat);
        else { LogManager::GetInstance().Log("Failed to init Bat"); delete newEnemyBat; }
    }
    else if (type == EnemySpawnType::TYPE2 && m_enemyType2.size() < static_cast<size_t>(m_maxType2)) 
    {
        EnemyType2* newEnemyType2 = new EnemyType2();
        if (!newEnemyType2) { LogManager::GetInstance().Log("Failed to allocate Type2"); return; }
        newEnemyType2->SetSceneReference(this);
        spawnPos.y = newEnemyType2->kGroundLevel;
        if (newEnemyType2->Initialise(*m_pRenderer, spawnPos)) m_enemyType2.push_back(newEnemyType2);
        else { LogManager::GetInstance().Log("Failed to init Type2"); delete newEnemyType2; }
    }
}

// ADD: Implementation for drawing player UI (Health/Stamina bars)
void SceneAbyssWalker::DrawPlayerUI(Renderer& renderer)
{
    if (!m_pPlayer || !m_pRenderer) return;

    // UI Bar constants
    const float BAR_HEIGHT = 20.0f;
    const float HEALTH_BAR_WIDTH = 250.0f;
    const float STAMINA_BAR_WIDTH = 200.0f;
    const float BAR_Y_OFFSET_FROM_BOTTOM = 40.0f;
    const float BAR_SPACING = 10.0f; // Spacing between health and stamina bar
    const float BORDER_THICKNESS = 2.0f;

    int screenWidth = m_pRenderer->GetWidth();
    int screenHeight = m_pRenderer->GetHeight();

    // Colors (R, G, B, Alpha)
    const unsigned char HEALTH_FILL_R = 200, HEALTH_FILL_G = 0, HEALTH_FILL_B = 0, BAR_FILL_A = 220;
    const unsigned char STAMINA_FILL_R = 200, STAMINA_FILL_G = 200, STAMINA_FILL_B = 0;
    const unsigned char BAR_BG_R = 50, BAR_BG_G = 50, BAR_BG_B = 50, BAR_BG_A = 200;
    const unsigned char BAR_BORDER_R = 20, BAR_BORDER_G = 20, BAR_BORDER_B = 20, BAR_BORDER_A = 255;

    // Calculate total width of the UI group (Health + Space + Stamina) to center it
    float totalUIGroupWidth = HEALTH_BAR_WIDTH + BAR_SPACING + STAMINA_BAR_WIDTH;
    float groupStartX = (static_cast<float>(screenWidth) - totalUIGroupWidth) / 2.0f;

    // --- Health Bar ---
    float healthBarX = groupStartX;
    float healthBarY = static_cast<float>(screenHeight) - BAR_Y_OFFSET_FROM_BOTTOM - BAR_HEIGHT;

    int currentHealth = m_pPlayer->GetCurrentHealth();
    int maxHealth = m_pPlayer->GetPlayerStats().GetMaxHealth(); // Use PlayerStats for max values
    float healthRatio = (maxHealth > 0) ? static_cast<float>(currentHealth) / static_cast<float>(maxHealth) : 0.0f;
    healthRatio = std::max(0.0f, std::min(1.0f, healthRatio)); // Clamp between 0 and 1

    // Health Bar Border
    renderer.DrawDebugRect(
        healthBarX - BORDER_THICKNESS,
        healthBarY - BORDER_THICKNESS,
        healthBarX + HEALTH_BAR_WIDTH + BORDER_THICKNESS,
        healthBarY + BAR_HEIGHT + BORDER_THICKNESS,
        BAR_BORDER_R, BAR_BORDER_G, BAR_BORDER_B, BAR_BORDER_A
    );

    // Health Bar Background
    renderer.DrawDebugRect(
        healthBarX,
        healthBarY,
        healthBarX + HEALTH_BAR_WIDTH,
        healthBarY + BAR_HEIGHT,
        BAR_BG_R, BAR_BG_G, BAR_BG_B, BAR_BG_A
    );

    // Health Bar Fill
    if (healthRatio > 0) 
    {
        renderer.DrawDebugRect(
            healthBarX,
            healthBarY,
            healthBarX + (HEALTH_BAR_WIDTH * healthRatio),
            healthBarY + BAR_HEIGHT,
            HEALTH_FILL_R, HEALTH_FILL_G, HEALTH_FILL_B, BAR_FILL_A
        );
    }

    // --- Stamina Bar ---
    float staminaBarX = healthBarX + HEALTH_BAR_WIDTH + BAR_SPACING;
    float staminaBarY = healthBarY; // Same Y as health bar

    float currentStamina = m_pPlayer->GetCurrentStamina();
    float maxStamina = m_pPlayer->GetPlayerStats().GetMaxStamina(); // Use PlayerStats for max values
    float staminaRatio = (maxStamina > 0.0f) ? currentStamina / maxStamina : 0.0f;
    staminaRatio = std::max(0.0f, std::min(1.0f, staminaRatio)); // Clamp

    // Stamina Bar Border
    renderer.DrawDebugRect(
        staminaBarX - BORDER_THICKNESS,
        staminaBarY - BORDER_THICKNESS,
        staminaBarX + STAMINA_BAR_WIDTH + BORDER_THICKNESS,
        staminaBarY + BAR_HEIGHT + BORDER_THICKNESS,
        BAR_BORDER_R, BAR_BORDER_G, BAR_BORDER_B, BAR_BORDER_A
    );

    // Stamina Bar Background
    renderer.DrawDebugRect(
        staminaBarX,
        staminaBarY,
        staminaBarX + STAMINA_BAR_WIDTH,
        staminaBarY + BAR_HEIGHT,
        BAR_BG_R, BAR_BG_G, BAR_BG_B, BAR_BG_A
    );

    // Stamina Bar Fill
    if (staminaRatio > 0) 
    
    {
        renderer.DrawDebugRect(
            staminaBarX,
            staminaBarY,
            staminaBarX + (STAMINA_BAR_WIDTH * staminaRatio),
            staminaBarY + BAR_HEIGHT,
            STAMINA_FILL_R, STAMINA_FILL_G, STAMINA_FILL_B, BAR_FILL_A
        );
    }
}

void SceneAbyssWalker::Draw(Renderer& renderer)
{
    if (m_pmoonBackground) m_pmoonBackground->Draw(renderer);

    WaveState currentWaveState = WaveState::PRE_WAVE_DELAY;

    if (m_pWaveSystem)
    {
        currentWaveState = m_pWaveSystem->GetCurrentState();
    }

    if (currentWaveState == WaveState::INTERMISSION)
    {
        DrawUpgradeMenu(renderer);
    }

    if (currentWaveState == WaveState::GAME_WON || currentWaveState == WaveState::GAME_END_PROMPT)
    {
        DrawEndGamePrompts(renderer);
    }

    if (m_pPlayer)
    {
        m_pPlayer->Draw(renderer);

        // Draw player hitbox
        if (m_bShowHitboxes)
        {
            Vector2 playerPos = m_pPlayer->GetPosition();
            float playerHalfWidth = Player::PLAYER_SPRITE_WIDTH / 2.0f;
            float playerHalfHeight = Player::PLAYER_SPRITE_HEIGHT / 2.0f;

            // Draw player collision box in green
            renderer.DrawDebugRect(
                playerPos.x - playerHalfWidth,
                playerPos.y - playerHalfHeight,
                playerPos.x + playerHalfWidth,
                playerPos.y + playerHalfHeight,
                0, 255, 0, 128
            );

            // Draw attack hitbox in red when attacking
            if (m_pPlayer->GetCurrentState() == PlayerState::ATTACKING)
            {
                AnimatedSprite* playerSprite = m_pPlayer->GetCurrentAnimatedSprite();
                if (playerSprite)
                {
                    int currentFrame = playerSprite->GetCurrentFrame();
                    bool isHitFrame = (currentFrame >= 2 && currentFrame <= 5);

                    if (isHitFrame)
                    {
                        float attackReach = 40.0f;
                        float pAttackMinX = m_pPlayer->IsFacingRight() ?
                            playerPos.x :
                            playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + attackReach);
                        float pAttackMaxX = m_pPlayer->IsFacingRight() ?
                            playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + attackReach) :
                            playerPos.x;
                        float pAttackMinY = playerPos.y - (Player::PLAYER_SPRITE_HEIGHT / 2.0f);
                        float pAttackMaxY = playerPos.y + (Player::PLAYER_SPRITE_HEIGHT / 2.0f);

                        renderer.DrawDebugRect(
                            pAttackMinX,
                            pAttackMinY,
                            pAttackMaxX,
                            pAttackMaxY,
                            255, 0, 0, 128
                        );
                    }
                }
            }
        }
    }

    for (EnemyBat* enemyBat : m_enemyBats)
    {
        enemyBat->Draw(renderer);

        if (m_bShowHitboxes)
        {
            Vector2 enemyPos = enemyBat->GetPosition();
            float enemyRadius = enemyBat->GetRadius();

            renderer.DrawDebugRect(
                enemyPos.x - enemyRadius,
                enemyPos.y - enemyRadius,
                enemyPos.x + enemyRadius,
                enemyPos.y + enemyRadius,
                255, 255, 0, 128  // Yellow for enemies
            );
        }
    }

    // Draw EnemyType2
    for (EnemyType2* enemyT2 : m_enemyType2)
    {
        enemyT2->Draw(renderer);
        if (m_bShowHitboxes)
        {
            Vector2 enemyPos = enemyT2->GetPosition();
            float enemyRadius = enemyT2->GetRadius();
            renderer.DrawDebugRect(
                enemyPos.x - enemyRadius, enemyPos.y - enemyRadius,
                enemyPos.x + enemyRadius, enemyPos.y + enemyRadius,
                255, 165, 0, 128  // Orange
            );
        }
    }

    DrawPlayerUI(renderer);

    // Then draw screen-space UI elements
    if (currentWaveState == WaveState::INTERMISSION)
    {
        DrawUpgradeMenu(renderer);
    }

    if (currentWaveState == WaveState::GAME_WON || currentWaveState == WaveState::GAME_END_PROMPT)
    {
        DrawEndGamePrompts(renderer);
    }
}

void SceneAbyssWalker::DebugDraw()
{
    if (ImGui::CollapsingHeader("Scene Debug"))
    {
        ImGui::Checkbox("Show Hitboxes", &m_bShowHitboxes);

        if (m_pPlayer)
        {
            m_pPlayer->DebugDraw();
        }

        ImGui::Separator();
        size_t totalEnemies = m_enemyBats.size() + m_enemyType2.size();
        ImGui::Text("Total Enemies: %zu / %d", totalEnemies, m_maxEnemies);

        ImGui::Text("Bats: %zu / %d", m_enemyBats.size(), m_maxBats);
        ImGui::Text("  Spawn Timer: %.2f / %.2f", m_batSpawnTimer, m_batSpawnInterval);

        ImGui::Text("EnemyType2s: %zu / %d", m_enemyType2.size(), m_maxType2);
        ImGui::Text("  Spawn Timer: %.2f / %.2f", m_type2SpawnTimer, m_type2SpawnInterval);

        if (ImGui::CollapsingHeader("Enemy List"))
        {
            for (size_t i = 0; i < m_enemyBats.size(); ++i)
            {
                std::string enemyNodeId = "EnemyBat " + std::to_string(i);
                if (ImGui::TreeNode(enemyNodeId.c_str()))
                {
                    m_enemyBats[i]->DebugDraw();
                    ImGui::TreePop();
                }
            }
        }

        if (ImGui::CollapsingHeader("EnemyType2 List"))
        {
            for (size_t i = 0; i < m_enemyType2.size(); ++i)
            {
                std::string enemyNodeId = "EnemyType2 " + std::to_string(i);
                if (ImGui::TreeNode(enemyNodeId.c_str()))
                {
                    m_enemyType2[i]->DebugDraw();
                    ImGui::TreePop();
                }
            }
        }
    }
}