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
    , m_bShowHitboxes(false)
    , m_playerChoseToQuit(false)
    , m_pWaveSystem(nullptr)
    , m_pEnemySpawner(nullptr)
    , m_pGameEndPrompt(nullptr)
    , m_pUpgradeMenu(nullptr)
    , m_pPlayerHUD(nullptr)
    , m_pCollisionSystem(nullptr)
{
    m_pmoonBackground = nullptr;
}

SceneAbyssWalker::~SceneAbyssWalker()
{
    delete m_pCollisionSystem;
    m_pCollisionSystem = nullptr;

    delete m_pPlayerHUD;
    m_pPlayerHUD = nullptr;

    delete m_pEnemySpawner;
    m_pEnemySpawner = nullptr;

    delete m_pGameEndPrompt;
    m_pGameEndPrompt = nullptr;

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
    if (!m_pmoonBackground)
    {
        fullBackground(*m_pRenderer);
    }

    for (EnemyBat* enemyBat : m_enemyBats) delete enemyBat;
    m_enemyBats.clear();
    for (EnemyType2* enemyType2 : m_enemyType2) delete enemyType2;
    m_enemyType2.clear();

    // Load Player 
    if (m_pPlayer) 
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::Initialise - Player exists, calling ResetForNewGame.");
        m_pPlayer->ResetForNewGame();
    }
    else 
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::Initialise - Player is NULL, creating new.");
        m_pPlayer = new Player();
        if (!m_pPlayer || !m_pPlayer->Initialise(*m_pRenderer)) 
        {
            delete m_pPlayer; 
            m_pPlayer = nullptr;
            return false;
        }
    }

    // Load Upgrade Menu prompt (Shows up at the end of each wave)
    m_pUpgradeMenu = new UpgradeMenu(m_pRenderer, m_pPlayer, this);
    if (!m_pUpgradeMenu)
    {
        LogManager::GetInstance().Log("Failed to load UpgradeMenu!!");
        return false;
    }

    // End Game Prompt (When player dies or loses)
    m_pGameEndPrompt = new GameEndPrompt(m_pRenderer, m_pPlayer, this);
    if (!m_pGameEndPrompt)
    {
        LogManager::GetInstance().Log("Failed to load Game End Prompt Menu!!");
        return false;
    }

    // Load Wave System
    m_pWaveSystem = new WaveSystem(this, m_pPlayer);
    if (!m_pWaveSystem)
    {
        LogManager::GetInstance().Log("Failed to load WaveManager!!");
        return false;
    }

    // Load Enemy Spawner
    m_pEnemySpawner = new EnemySpawner(this, m_pPlayer, m_pRenderer);
    if (!m_pEnemySpawner)
    {
        LogManager::GetInstance().Log("Failed to load EnemySpawner!!");
        return false;
    }

    // Load the Player HUD
    m_pPlayerHUD = new PlayerHUD(m_pRenderer, m_pPlayer);
    if (!m_pPlayerHUD)
    {
        LogManager::GetInstance().Log("Failed to load playerHUD!!");
        return false;
    }

    // Load the Collision System
    m_pCollisionSystem = new CollisionSystem();
    if (!m_pCollisionSystem)
    {
        LogManager::GetInstance().Log("Failed to load the Collision Systenm!!");
        return false;
    }

    if (m_pWaveSystem) m_pWaveSystem->ResetForNewGame();
    if (m_pEnemySpawner) m_pEnemySpawner->Reset();
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

    if (m_pWaveSystem) m_pWaveSystem->ResetForNewGame();
    if (m_pEnemySpawner) m_pEnemySpawner->Reset();

    if (m_pUpgradeMenu) m_pUpgradeMenu->SetActive(false);
    if (m_pGameEndPrompt) m_pGameEndPrompt->SetActive(false);

    if (m_pPlayer) m_pPlayer->ResetForNewGame();

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
    else if (currentWaveState == WaveState::GAME_END_PROMPT || currentWaveState == WaveState::GAME_WON)
    {
        UpdateGameEndPromptUI(inputSystem);
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

        bool proecessEnemies = (currentWaveState == WaveState::IN_WAVE);
        if (processEnemies) 
        {
            if (m_pCollisionSystem && m_pPlayer && m_pPlayer->IsAlive())
            {
                m_pCollisionSystem->ProcessCollisions(m_pPlayer, m_enemyBats, m_enemyType2);
            }

            if (m_pPlayer->IsAlive() && m_pEnemySpawner)
            {
                m_pEnemySpawner->Update(deltaTime, m_enemyBats, m_enemyType2);
            }
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

void SceneAbyssWalker::AddEnemy(EnemyBat* bat)
{
    if (bat) 
    {
        m_enemyBats.push_back(bat);
    }
}

void SceneAbyssWalker::AddEnemy(EnemyType2* type2)
{
    if (type2) 
    {
        m_enemyType2.push_back(type2);
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
    if (m_pGameEndPrompt)
    {
        m_pGameEndPrompt->SetActive(false);
    }
}

void SceneAbyssWalker::SetupGameEndPromptUI(const std::string& titleMessage) 
{
    if (m_pGameEndPrompt)
    {
        m_pGameEndPrompt->SetActive(true, titleMessage);
    }
}

void SceneAbyssWalker::UpdateGameEndPromptUI(InputSystem& inputSystem) 
{
    if (m_pGameEndPrompt && m_pGameEndPrompt->IsActive())
    {
        m_pGameEndPrompt->UpdateUI(inputSystem);
    }
}

void SceneAbyssWalker::DrawEndGamePrompts(Renderer& renderer) 
{
    if (m_pGameEndPrompt && m_pGameEndPrompt->IsActive())
    {
        m_pGameEndPrompt->Draw(renderer);
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

    if (m_pPlayerHUD)
    {
        m_pPlayerHUD->Draw();
    }

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