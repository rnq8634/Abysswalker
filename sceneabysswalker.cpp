// This inlcude
#include "SceneAbyssWalker.h"

// Local Include
#include "Player.h"
#include "EnemyBat.h" 
#include "Renderer.h"
#include "InputSystem.h"
#include "Sprite.h"
#include "LogManager.h" 
#include "AnimatedSprite.h" 

// Lib includes
#include "fmod.hpp"

// IMGUI
#include "imgui/imgui.h"

SceneAbyssWalker::SceneAbyssWalker(FMOD::System* pFMODSystem)
    : m_pPlayer(nullptr)
    , m_pFMODSystem(pFMODSystem)
    , m_pSound(0)
    , m_pChannel(0)
    , m_pJumpSound(0)
    , m_pAttackSound(0)
    , m_pRollSound(0)
    , m_pHurtSound(0)
    , m_pRenderer(nullptr)
    , m_pmoonBackground(nullptr)
    , m_ptree5Background(nullptr)
    , m_ptree4Background(nullptr)
    , m_ptree3Background(nullptr)
    , m_ptree2Background(nullptr)
    , m_ptree1Background(nullptr)
    , m_spawnTimer(0.0f)
{
}

SceneAbyssWalker::~SceneAbyssWalker()
{
    // FMOD resources
    if (m_pSound) { m_pSound->release(); m_pSound = nullptr; }
    if (m_pJumpSound) { m_pJumpSound->release(); m_pJumpSound = nullptr; }
    if (m_pAttackSound) { m_pAttackSound->release(); m_pAttackSound = nullptr; }
    if (m_pRollSound) { m_pRollSound->release(); m_pRollSound = nullptr; }
    if (m_pHurtSound) { m_pHurtSound->release(); m_pHurtSound = nullptr; }

    delete m_pPlayer;
    m_pPlayer = nullptr;

    for (EnemyBat* enemyBat : m_enemyType1)
    {
        delete enemyBat;
    }
    m_enemyType1.clear();

    m_pRenderer = nullptr;

    // Background
    delete m_pmoonBackground; m_pmoonBackground = nullptr;
    delete m_ptree5Background; m_ptree5Background = nullptr;
    delete m_ptree4Background; m_ptree4Background = nullptr;
    delete m_ptree3Background; m_ptree3Background = nullptr;
    delete m_ptree2Background; m_ptree2Background = nullptr;
    delete m_ptree1Background; m_ptree1Background = nullptr;
}

void SceneAbyssWalker::fullBackground(Renderer& renderer)
{
    m_pmoonBackground = renderer.CreateSprite("assets/backgrounds/main_background.png");
    m_ptree5Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree5.png");
    m_ptree4Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree4.png");
    m_ptree3Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree3.png");
    m_ptree2Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree2.png");
    m_ptree1Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree1.png");

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

    auto setupTreeLayer = [&](Sprite* treeSprite, float desiredRelativeHeight)
        {
            if (!treeSprite) return;
            float scaleToScreenWidthX = static_cast<float>(screenWidth) / treeSprite->GetOriginalWidth();
            float scaleToScreenWidthY = scaleToScreenWidthX;

            treeSprite->SetScale(scaleToScreenWidthX, scaleToScreenWidthY);
            treeSprite->SetX(screenCenterX);

            float scaledTreeHeight = treeSprite->GetOriginalHeight() * scaleToScreenWidthY;
            treeSprite->SetY(static_cast<int>(screenBottomY - (scaledTreeHeight / 2.0f)));
            treeSprite->SetFlipHorizontal(true);
            treeSprite->SetAngle(180.0f);
        };

    setupTreeLayer(m_ptree5Background, 1.0f);
    setupTreeLayer(m_ptree4Background, 1.0f);
    setupTreeLayer(m_ptree3Background, 1.0f);
    setupTreeLayer(m_ptree2Background, 1.0f);
    setupTreeLayer(m_ptree1Background, 1.0f);
}

bool SceneAbyssWalker::Initialise(Renderer& renderer)
{
    m_pRenderer = &renderer;

    // Load FMOD
    FMOD_RESULT result;

    // Load BG Music (need to find music first)
    /*
    result = m_pFMODSystem->createStream("assets/sounds/background_music.mp3", FMOD_LOOP_NORMAL, 0, &m_pSound);
    if (result != FMOD_OK)
    {
        LogManager::GetInstance().Log("Failed to load background music!");
    }
    else
    {
        // Play background music
        m_pFMODSystem->playSound(m_pSound, 0, false, &m_pChannel);
        if (m_pChannel)
        {
            m_pChannel->setVolume(0.5f); // Set volume to 50%
        }
    }
    */

    // For loading sound effects (PLACEHOLDER)!!!
    /*
    m_pFMODSystem->createSound("assets/sounds/jump.wav", FMOD_DEFAULT, 0, &m_pJumpSound);
    m_pFMODSystem->createSound("assets/sounds/attack.wav", FMOD_DEFAULT, 0, &m_pAttackSound);
    m_pFMODSystem->createSound("assets/sounds/roll.wav", FMOD_DEFAULT, 0, &m_pRollSound);
    m_pFMODSystem->createSound("assets/sounds/hurt.wav", FMOD_DEFAULT, 0, &m_pHurtSound);
    */

    fullBackground(*m_pRenderer);

    // load player
    m_pPlayer = new Player();
    if (!m_pPlayer || !m_pPlayer->Initialise(*m_pRenderer))
    {
        LogManager::GetInstance().Log("Failed to initialise Player.");
        delete m_pPlayer; m_pPlayer = nullptr;
        return false;
    }

    m_spawnTimer = m_spawnInterval; // Start ready to spawn

    return true;
}

void SceneAbyssWalker::Process(float deltaTime, InputSystem& inputSystem)
{
    if (!m_pPlayer || !m_pRenderer) return;

    // Player input processing
    const float moveSpeed = 150.0f;
    const float rollSpeed = 200.0f;
    bool isMoving = false;

    if (m_pPlayer->IsAlive())
    {
        if (inputSystem.GetKeyState(SDL_SCANCODE_A) == BS_HELD)
        {
            m_pPlayer->MoveLeft(moveSpeed);
            isMoving = true;
        }
        else if (inputSystem.GetKeyState(SDL_SCANCODE_D) == BS_HELD)
        {
            m_pPlayer->MoveRight(moveSpeed);
            isMoving = true;
        }

        if (inputSystem.GetKeyState(SDL_SCANCODE_SPACE) == BS_PRESSED)
        {
            m_pPlayer->Jump();
            /*
            if (m_pJumpSound)
            {
                m_pFMODSystem->playSound(m_pJumpSound, 0, false, 0);
            }
            */
        }

        if (inputSystem.GetKeyState(SDL_SCANCODE_J) == BS_PRESSED)
        {
            m_pPlayer->Attack();
            // add sound ^^^ like jump
        }
        if (inputSystem.GetKeyState(SDL_SCANCODE_LSHIFT) == BS_PRESSED || inputSystem.GetKeyState(SDL_SCANCODE_Q) == BS_PRESSED)
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

    for (EnemyBat* enemyBat : m_enemyType1)
    {
        if (enemyBat->m_pTargetPlayer == nullptr && m_pPlayer)
        {
            enemyBat->m_pTargetPlayer = m_pPlayer;
        }
        enemyBat->Process(deltaTime);
    }
    
    HandleCollisions();

    m_enemyType1.erase(std::remove_if(m_enemyType1.begin(), m_enemyType1.end(),
        [](EnemyBat* enemyBat)
        {
            if (!enemyBat->IsAlive())
            {
                AnimatedSprite* sprite = enemyBat->GetCurrentAnimatedSprite();
                if (sprite && sprite->IsAnimationComplete())
                {
                    delete enemyBat;
                    return true;
                }
                if (!sprite)
                {
                    LogManager::GetInstance().Log("Dead enemy removed (no death animation).");
                    delete enemyBat;
                    return true;
                }
            }
            return false;
        }), m_enemyType1.end());

    if (m_pPlayer->IsAlive())
    {
        UpdateSpawning(deltaTime);
    }
}

void SceneAbyssWalker::HandleCollisions()
{
    if (!m_pPlayer) return;

    for (EnemyBat* enemyBat : m_enemyType1)
    {
        if (!enemyBat->IsAlive()) continue;

        if (m_pPlayer->IsAlive())
        {
            // Check for player getting hit by enemy
            if (m_pPlayer->GetCurrentState() != PlayerState::ROLLING &&
                m_pPlayer->GetCurrentState() != PlayerState::HURT &&
                m_pPlayer->CheckCollision(*enemyBat))
            {
                m_pPlayer->TakeDamage(1);
                if (m_pHurtSound)
                {
                    m_pFMODSystem->playSound(m_pHurtSound, 0, false, 0);
                }
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
                        float attackReach = 40.0f;

                        float pAttackMinX = m_pPlayer->IsFacingRight() ?
                            playerPos.x :
                            playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + attackReach);
                        float pAttackMaxX = m_pPlayer->IsFacingRight() ?
                            playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + attackReach) :
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
                            enemyBat->TakeDamage(25); // Player attack damage to enemyBat
                            LogManager::GetInstance().Log("Player hit Bat!");
                        }
                    }
                }
            }
        }
    }
}


void SceneAbyssWalker::UpdateSpawning(float deltaTime)
{
    if (!m_pRenderer) 
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::UpdateSpawning - Renderer is null!");
        return;
    }

    if (m_maxEnemies >= 0 && m_enemyType1.size() < static_cast<size_t>(m_maxEnemies))
    {
        m_spawnTimer += deltaTime;
        if (m_spawnTimer >= m_spawnInterval)
        {
            m_spawnTimer = 0.0f;

            if (m_enemyType1.size() < m_maxEnemies)
            {
                int leftCount = 0;
                int rightCount = 0;
                for (const auto& enemy : m_enemyType1)
                {
                    if (enemy->GetPosition().x < m_pRenderer->GetWidth() / 2.0f)
                    {
                        leftCount++;
                    }
                    else {
                        rightCount++;
                    }
                }

                bool trySpawnLeft = (leftCount <= rightCount);

                if (trySpawnLeft && leftCount < m_maxEnemiesPerSide)
                {
                    SpawnEnemy(true);
                }
                else if (!trySpawnLeft && rightCount < m_maxEnemiesPerSide)
                {
                    SpawnEnemy(false);
                }
                else if (leftCount < m_maxEnemiesPerSide)
                {
                    SpawnEnemy(true);
                }
                else if (rightCount < m_maxEnemiesPerSide)
                {
                    SpawnEnemy(false);
                }
            }
        }
    }
}

void SceneAbyssWalker::SpawnEnemy(bool spawnOnLeft)
{
    if (!m_pRenderer) 
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::SpawnEnemy - Renderer is null!");
        return;
    }

    // Enemy bat
    EnemyBat* newEnemyBat = new EnemyBat();
    if (!newEnemyBat) 
    {
        LogManager::GetInstance().Log("Failed to allocate memory for new enemy.");
        return;
    }

    if (!m_pPlayer) 
    {
        LogManager::GetInstance().Log("Cannot spawn enemy, player is null.");
        delete newEnemyBat;
        return;
    }

    Vector2 spawnPos;
    const float spawnXOffset = 50.0f;
    if (spawnOnLeft)
    {
        spawnPos.x = -spawnXOffset;
    }
    else
    {
        spawnPos.x = static_cast<float>(m_pRenderer->GetWidth()) + spawnXOffset;
    }
    spawnPos.y = newEnemyBat->kGroundLevel;

    if (newEnemyBat->Initialise(*m_pRenderer, spawnPos))
    {
        m_enemyType1.push_back(newEnemyBat);
        LogManager::GetInstance().Log(("Spawned new enemy on " + std::string(spawnOnLeft ? "left" : "right")).c_str());
    }
    else
    {
        LogManager::GetInstance().Log("Failed to initialise new enemy.");
        delete newEnemyBat;
    }
}


void SceneAbyssWalker::Draw(Renderer& renderer)
{
    if (m_pmoonBackground) m_pmoonBackground->Draw(renderer);
    if (m_ptree5Background) m_ptree5Background->Draw(renderer);
    if (m_ptree4Background) m_ptree4Background->Draw(renderer);
    if (m_ptree3Background) m_ptree3Background->Draw(renderer);
    if (m_ptree2Background) m_ptree2Background->Draw(renderer);
    if (m_ptree1Background) m_ptree1Background->Draw(renderer);

    if (m_pPlayer)
    {
        m_pPlayer->Draw(renderer);
    }

    for (EnemyBat* enemyBat : m_enemyType1)
    {
        enemyBat->Draw(renderer);
    }
}

void SceneAbyssWalker::DebugDraw()
{
    if (m_pPlayer)
    {
        m_pPlayer->DebugDraw();
    }

    ImGui::Separator();
    ImGui::Text("Enemies: %zu / %d (Max Total)", m_enemyType1.size(), m_maxEnemies);
    ImGui::Text("Spawn Timer: %.2f / %.2f", m_spawnTimer, m_spawnInterval);


    if (ImGui::CollapsingHeader("Enemy List")) 
    {
        for (size_t i = 0; i < m_enemyType1.size(); ++i)
        {
            std::string enemyNodeId = "EnemyBat " + std::to_string(i);
            if (ImGui::TreeNode(enemyNodeId.c_str()))
            {
                m_enemyType1[i]->DebugDraw();
                ImGui::TreePop();
            }
        }
    }
}