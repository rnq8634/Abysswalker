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
{
    m_pmoonBackground = nullptr;
}

SceneAbyssWalker::~SceneAbyssWalker()
{
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
    fullBackground(*m_pRenderer);

    // Load Player
    m_pPlayer = new Player();
    if (!m_pPlayer || !m_pPlayer->Initialise(*m_pRenderer))
    {
        LogManager::GetInstance().Log("Failed to initialise Player!!");
        delete m_pPlayer; m_pPlayer = nullptr;
        return false;
    }
    m_pPlayer->ResetForNewGame();

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
            HandleCollisions();
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

// Drawing UI Bars for Player (Health and Stamina)
void SceneAbyssWalker::DrawPlayerUI(Renderer& renderer)
{
    if (!m_pPlayer || !m_pRenderer) return;

    // UI Bar Configs
    const float BAR_HEIGHT = 20.0f;
    const float HEALTH_BAR_WIDTH = 300.0f;
    const float STAMINA_BAR_WIDTH = 200.0f;
    const float BAR_Y_OFFSET_FROM_BOTTOM = 40.0f;
    const float BAR_SPACING = 20.0f;
    const float BORDER_THICKNESS = 5.0f;

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
    renderer.DrawDebugRect(healthBarX - BORDER_THICKNESS, healthBarY - BORDER_THICKNESS,
        healthBarX + HEALTH_BAR_WIDTH + BORDER_THICKNESS,
        healthBarY + BAR_HEIGHT + BORDER_THICKNESS,
        BAR_BORDER_R, BAR_BORDER_G, BAR_BORDER_B, BAR_BORDER_A
    );

    // Health Bar Background
    renderer.DrawDebugRect(healthBarX, healthBarY,
        healthBarX + HEALTH_BAR_WIDTH,
        healthBarY + BAR_HEIGHT,
        BAR_BG_R, BAR_BG_G, BAR_BG_B, BAR_BG_A
    );

    // Health Bar Fill
    if (healthRatio > 0) 
    {
        renderer.DrawDebugRect(healthBarX, healthBarY,
            healthBarX + (HEALTH_BAR_WIDTH * healthRatio),
            healthBarY + BAR_HEIGHT,
            HEALTH_FILL_R, HEALTH_FILL_G, HEALTH_FILL_B, BAR_FILL_A
        );
    }

    // --- Stamina Bar ---
    float staminaBarX = healthBarX + HEALTH_BAR_WIDTH + BAR_SPACING;
    float staminaBarY = healthBarY;

    float currentStamina = m_pPlayer->GetCurrentStamina();
    float maxStamina = m_pPlayer->GetPlayerStats().GetMaxStamina();
    float staminaRatio = (maxStamina > 0.0f) ? currentStamina / maxStamina : 0.0f;
    staminaRatio = std::max(0.0f, std::min(1.0f, staminaRatio));

    // Stamina Bar Border
    renderer.DrawDebugRect(staminaBarX - BORDER_THICKNESS, staminaBarY - BORDER_THICKNESS,
        staminaBarX + STAMINA_BAR_WIDTH + BORDER_THICKNESS,
        staminaBarY + BAR_HEIGHT + BORDER_THICKNESS,
        BAR_BORDER_R, BAR_BORDER_G, BAR_BORDER_B, BAR_BORDER_A
    );

    // Stamina Bar Background
    renderer.DrawDebugRect(staminaBarX, staminaBarY,
        staminaBarX + STAMINA_BAR_WIDTH,
        staminaBarY + BAR_HEIGHT,
        BAR_BG_R, BAR_BG_G, BAR_BG_B, BAR_BG_A
    );

    // Stamina Bar Fill
    if (staminaRatio > 0) 
    {
        renderer.DrawDebugRect(staminaBarX, staminaBarY,
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