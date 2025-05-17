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

// IMGUI
#include "imgui/imgui.h"

// NEED TO COMPLETE THESE
const char* SF_PLAYER_JUMP = "player_jump";
const char* SF_PLAYER_ATTACK = "player_jump";
const char* SF_PLAYER_ROLL = "player_jump";
const char* SF_PLAYER_HURT = "player_jump";
const char* BGM_ABYSSWALKER = "player_jump";

// Player attack const for now (Stats UPG need to be added)
const float PLAYER_ATTACK_REACH = 1.0f;
const int PLAYER_ATTACK_DAMAGE = 25; // Base dmg

SceneAbyssWalker::SceneAbyssWalker()
    : m_pPlayer(nullptr)
    , m_pRenderer(nullptr)
    , m_pmoonBackground(nullptr)
    , m_ptree5Background(nullptr)
    , m_ptree4Background(nullptr)
    , m_ptree3Background(nullptr)
    , m_ptree2Background(nullptr)
    , m_ptree1Background(nullptr)
    , m_batSpawnTimer(0.0f)
    , m_type2SpawnTimer(0.0f)
    , m_bShowHitboxes(false)
{
}

SceneAbyssWalker::~SceneAbyssWalker()
{
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

    fullBackground(*m_pRenderer);

    // load player
    m_pPlayer = new Player();
    if (!m_pPlayer || !m_pPlayer->Initialise(*m_pRenderer))
    {
        LogManager::GetInstance().Log("Failed to initialise Player.");
        delete m_pPlayer; m_pPlayer = nullptr;
        return false;
    }

    m_batSpawnTimer = m_batSpawnInterval; // Start ready to spawn enemies
    m_type2SpawnTimer = m_type2SpawnInterval;

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
        // Need to add controller support
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

    // Process the bats
    for (EnemyBat* enemyBat : m_enemyBats)
    {
        if (enemyBat->m_pTargetPlayer == nullptr && m_pPlayer)
        {
            enemyBat->m_pTargetPlayer = m_pPlayer;
        }
        enemyBat->Process(deltaTime);
    }

    // Process Type 2's
    for (EnemyType2* enemyT2 : m_enemyType2)
    {
        if (enemyT2->m_pTargetPlayer == nullptr && m_pPlayer)
        {
            enemyT2->m_pTargetPlayer = m_pPlayer;
        }
        enemyT2->Process(deltaTime);
    }
    
    HandleCollisions();
    CleanupDead();

    if (m_pPlayer->IsAlive())
    {
        UpdateSpawning(deltaTime);
    }
}

void SceneAbyssWalker::CleanupDead()
{
    // Cleaning up the enemy bats
    m_enemyBats.erase(std::remove_if(m_enemyBats.begin(), m_enemyBats.end(), [](EnemyBat* enemyBat)
        {
            if (!enemyBat->IsAlive())
            {
                AnimatedSprite* sprite = enemyBat->GetCurrentAnimatedSprite();
                if ((sprite && sprite->IsAnimationComplete()) || !sprite)
                {
                    if (!sprite) LogManager::GetInstance().Log("Dead bats been removed");
                    delete enemyBat;
                    return true;
                }
            }
            return false;
        }), m_enemyBats.end());

    // Cleaning up Type 2's
    m_enemyType2.erase(std::remove_if(m_enemyType2.begin(), m_enemyType2.end(), [](EnemyType2* enemyType2)
        {
            if (!enemyType2->IsAlive())
            {
                AnimatedSprite* sprite = enemyType2->GetCurrentAnimatedSprite();
                if ((sprite && sprite->IsAnimationComplete()) || !sprite)
                {
                    if (!sprite) LogManager::GetInstance().Log("Dead bats been removed");
                    delete enemyType2;
                    return true;
                }
            }
            return false;
        }), m_enemyType2.end());
}

void SceneAbyssWalker::HandleCollisions()
{
    if (!m_pPlayer || !m_pPlayer->IsAlive()) return;

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
                SoundSystem::GetInstance().PlaySound(SF_PLAYER_HURT);
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
                                enemyBat->TakeDamage(PLAYER_ATTACK_DAMAGE); // Player attack damage to enemyBat
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
                SoundSystem::GetInstance().PlaySound(SF_PLAYER_HURT);
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
                            enemyT2->TakeDamage(PLAYER_ATTACK_DAMAGE);
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
    if (!m_pRenderer) 
    {
        LogManager::GetInstance().Log("SceneAbyssWalker::UpdateSpawning - Renderer is null!");
        return;
    }

    size_t totalCurrentEnemies = m_enemyBats.size() + m_enemyType2.size();

    // Check if bats can be spawned
    if (totalCurrentEnemies < static_cast<size_t>(m_maxEnemies) && m_enemyBats.size() < static_cast<size_t>(m_maxBats))
    {
        m_batSpawnTimer += deltaTime;
        if (m_batSpawnTimer >= m_batSpawnInterval)
        {
            m_batSpawnTimer = 0.0f;

            int leftBatCount = 0;
            int rightBatCount = 0;
            for (const auto& enemy : m_enemyBats)
            {
                if (enemy->GetPosition().x < m_pRenderer->GetWidth() / 2.0f)
                {
                    leftBatCount++;
                }
                else
                {
                    rightBatCount++;
                }
            }

            bool trySpawnLeftBat = (leftBatCount <= rightBatCount);
            if (trySpawnLeftBat && leftBatCount < m_maxBatsPerSide)
            {
                SpawnEnemy(EnemySpawnType::BAT, true);
            }
            else if (!trySpawnLeftBat && rightBatCount < m_maxBatsPerSide)
            {
                SpawnEnemy(EnemySpawnType::BAT, false);
            }
            else if (leftBatCount < m_maxBatsPerSide)
            {
                SpawnEnemy(EnemySpawnType::BAT, true);
            }
            else if (rightBatCount < m_maxBatsPerSide)
            {
                SpawnEnemy(EnemySpawnType::BAT, false);
            }
        }
    }

    totalCurrentEnemies = m_enemyBats.size() + m_enemyType2.size();

    // CHeck if Type 2's can be spawned
    if (totalCurrentEnemies < static_cast<size_t>(m_maxEnemies) && m_enemyType2.size() < static_cast<size_t>(m_maxType2))
    {
        m_type2SpawnTimer += deltaTime;
        if (m_type2SpawnTimer >= m_type2SpawnInterval)
        {
            m_type2SpawnTimer = 0.0f;

            int leftT2Count = 0;
            int rightT2Count = 0;
            for (const auto& enemy : m_enemyType2)
            {
                if (enemy->GetPosition().x < m_pRenderer->GetWidth() / 2.0f)
                {
                    leftT2Count++;
                }
                else
                {
                    rightT2Count++;
                }
            }

            bool trySpawnLeft_Type2 = (leftT2Count <= rightT2Count);
            if (trySpawnLeft_Type2 && leftT2Count < m_maxType2) SpawnEnemy(EnemySpawnType::TYPE2, true);
            else if (!trySpawnLeft_Type2 && rightT2Count < m_maxType2) SpawnEnemy(EnemySpawnType::TYPE2, false);
            else if (leftT2Count < m_maxType2) SpawnEnemy(EnemySpawnType::TYPE2, true);
            else if (rightT2Count < m_maxType2) SpawnEnemy(EnemySpawnType::TYPE2, false);
        }
    }
}

void SceneAbyssWalker::SpawnEnemy(EnemySpawnType type, bool spawnOnLeft)
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

    size_t totalCurrentEnemies = m_enemyBats.size() + m_enemyType2.size();
    if (totalCurrentEnemies >= static_cast<size_t>(m_maxEnemies))
    {
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

    if (type == EnemySpawnType::BAT)
    {
        if (m_enemyBats.size() >= static_cast<size_t>(m_maxBats))
        {
            return;
        }

        EnemyBat* newEnemyBat = new EnemyBat();

        if (!newEnemyBat)
        {
            LogManager::GetInstance().Log("Failed to allocate memory for new bat");
            return;
        }

        spawnPos.y = newEnemyBat->kGroundLevel;

        if (newEnemyBat->Initialise(*m_pRenderer, spawnPos))
        {
            m_enemyBats.push_back(newEnemyBat);
        }
        else
        {
            LogManager::GetInstance().Log("Failed to init new bat");
            delete newEnemyBat;
        }
    }
    else if (type == EnemySpawnType::TYPE2)
    {
        if (m_enemyType2.size() >= static_cast<size_t>(m_maxType2))
        {
            return;
        }

        EnemyType2* newEnemyType2 = new EnemyType2();

        if (!newEnemyType2)
        {
            LogManager::GetInstance().Log("Failed to allocate memory for Type2");
            return;
        }

        spawnPos.y = newEnemyType2->kGroundLevel;

        if (newEnemyType2->Initialise(*m_pRenderer, spawnPos))
        {
            m_enemyType2.push_back(newEnemyType2);
        }
        else
        {
            LogManager::GetInstance().Log("Failed to init Type 2");
            delete newEnemyType2;
        }
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