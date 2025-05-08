// This inlcude
#include "SceneAbyssWalker.h"

// Local Include
#include "Player.h"
#include "Enemy.h" 
#include "Renderer.h"
#include "InputSystem.h"
#include "Sprite.h"
#include "LogManager.h" 
#include "AnimatedSprite.h" 

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <algorithm> // For std::remove_if


SceneAbyssWalker::SceneAbyssWalker()
    : m_pPlayer(nullptr)
    , m_pRenderer(nullptr) // Initialize m_pRenderer
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
    delete m_pPlayer;
    m_pPlayer = nullptr;

    for (Enemy* enemy : m_enemies)
    {
        delete enemy;
    }
    m_enemies.clear();

    // m_pRenderer is not owned by SceneAbyssWalker, so don't delete it here.
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
            treeSprite->SetY(screenBottomY - (scaledTreeHeight / 2.0f));
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
    m_pRenderer = &renderer; // Store the renderer

    fullBackground(*m_pRenderer); // Use stored renderer

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
    if (!m_pPlayer || !m_pRenderer) return; // Check if m_pRenderer is also valid

    // Player input processing
    const float moveSpeed = 125.0f;
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
        }

        if (inputSystem.GetKeyState(SDL_SCANCODE_J) == BS_PRESSED)
        {
            m_pPlayer->Attack();
        }
        if (inputSystem.GetKeyState(SDL_SCANCODE_LSHIFT) == BS_PRESSED || inputSystem.GetKeyState(SDL_SCANCODE_Q) == BS_PRESSED)
        {
            m_pPlayer->Roll(rollSpeed);
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

    for (Enemy* enemy : m_enemies)
    {
        enemy->Process(deltaTime);
    }

    HandleCollisions();

    m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
        [](Enemy* enemy) {
            if (!enemy->IsAlive()) {
                AnimatedSprite* sprite = enemy->GetCurrentAnimatedSprite();
                if (sprite && sprite->IsAnimationComplete()) {
                    delete enemy;
                    return true;
                }
                if (!sprite) {
                    LogManager::GetInstance().Log("Dead enemy removed (no death animation).");
                    delete enemy;
                    return true;
                }
            }
            return false;
        }), m_enemies.end());

    if (m_pPlayer->IsAlive())
    {
        UpdateSpawning(deltaTime); // Renderer argument removed
    }
}

void SceneAbyssWalker::HandleCollisions() {
    if (!m_pPlayer || !m_pPlayer->IsAlive() || m_pPlayer->GetCurrentState() != PlayerState::ATTACKING) {
        return;
    }

    AnimatedSprite* playerSprite = m_pPlayer->GetCurrentAnimatedSprite();
    if (!playerSprite) return;

    int currentFrame = playerSprite->GetCurrentFrame();
    bool isHitFrame = (currentFrame >= 2 && currentFrame <= 5); // Example hit frames

    if (!isHitFrame) return;

    Vector2 playerPos = m_pPlayer->GetPosition();
    float attackReach = 40.0f;

    float pAttackMinX = m_pPlayer->IsFacingRight() ? playerPos.x : playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + attackReach);
    float pAttackMaxX = m_pPlayer->IsFacingRight() ? playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + attackReach) : playerPos.x;
    float pAttackMinY = playerPos.y - (Player::PLAYER_SPRITE_HEIGHT / 2.0f);
    float pAttackMaxY = playerPos.y + (Player::PLAYER_SPRITE_HEIGHT / 2.0f);

    for (Enemy* enemy : m_enemies) {
        if (!enemy->IsAlive()) continue;

        Vector2 enemyPos = enemy->GetPosition();
        float enemyRadius = enemy->GetRadius();
        float eMinX = enemyPos.x - enemyRadius;
        float eMaxX = enemyPos.x + enemyRadius;
        float eMinY = enemyPos.y - enemyRadius;
        float eMaxY = enemyPos.y + enemyRadius;

        bool overlapX = pAttackMinX < eMaxX && pAttackMaxX > eMinX;
        bool overlapY = pAttackMinY < eMaxY && pAttackMaxY > eMinY;

        if (overlapX && overlapY) {
            enemy->TakeDamage(25); // Player attack damage
            LogManager::GetInstance().Log("Player hit enemy!");
        }
    }
}


void SceneAbyssWalker::UpdateSpawning(float deltaTime) // Renderer argument removed
{
    if (!m_pRenderer) { // Safety check for stored renderer
        LogManager::GetInstance().Log("SceneAbyssWalker::UpdateSpawning - Renderer is null!");
        return;
    }

    m_spawnTimer += deltaTime;
    if (m_spawnTimer >= m_spawnInterval)
    {
        m_spawnTimer = 0.0f;

        if (m_enemies.size() < m_maxEnemies)
        {
            int leftCount = 0;
            int rightCount = 0;
            for (const auto& enemy : m_enemies) {
                // Ensure m_pRenderer is used here
                if (enemy->GetPosition().x < m_pRenderer->GetWidth() / 2.0f) {
                    leftCount++;
                }
                else {
                    rightCount++;
                }
            }

            bool trySpawnLeft = (leftCount <= rightCount);

            if (trySpawnLeft && leftCount < m_maxEnemiesPerSide) {
                SpawnEnemy(true); // Renderer argument removed
            }
            else if (!trySpawnLeft && rightCount < m_maxEnemiesPerSide) {
                SpawnEnemy(false); // Renderer argument removed
            }
            else if (leftCount < m_maxEnemiesPerSide) {
                SpawnEnemy(true); // Renderer argument removed
            }
            else if (rightCount < m_maxEnemiesPerSide) {
                SpawnEnemy(false); // Renderer argument removed
            }
        }
    }
}

void SceneAbyssWalker::SpawnEnemy(bool spawnOnLeft) // Renderer argument removed
{
    if (!m_pRenderer) { // Safety check for stored renderer
        LogManager::GetInstance().Log("SceneAbyssWalker::SpawnEnemy - Renderer is null!");
        return;
    }

    Enemy* newEnemy = new Enemy();
    if (!newEnemy) {
        LogManager::GetInstance().Log("Failed to allocate memory for new enemy.");
        return;
    }

    if (!m_pPlayer) {
        LogManager::GetInstance().Log("Cannot spawn enemy, player is null.");
        delete newEnemy;
        return;
    }

    Vector2 spawnPos;
    const float spawnXOffset = 70.0f;
    if (spawnOnLeft)
    {
        spawnPos.x = -spawnXOffset;
    }
    else
    {
        // Ensure m_pRenderer is used here
        spawnPos.x = static_cast<float>(m_pRenderer->GetWidth()) + spawnXOffset;
    }
    spawnPos.y = newEnemy->kGroundLevel;

    // Ensure *m_pRenderer is used here for initialization
    if (newEnemy->Initialise(*m_pRenderer, m_pPlayer, spawnPos))
    {
        m_enemies.push_back(newEnemy);
        LogManager::GetInstance().Log(("Spawned new enemy on " + std::string(spawnOnLeft ? "left" : "right")).c_str());
    }
    else
    {
        LogManager::GetInstance().Log("Failed to initialise new enemy.");
        delete newEnemy;
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

    for (Enemy* enemy : m_enemies)
    {
        enemy->Draw(renderer);
    }
}

void SceneAbyssWalker::DebugDraw()
{
    if (m_pPlayer)
    {
        m_pPlayer->DebugDraw();
    }

    ImGui::Separator();
    ImGui::Text("Enemies: %zu / %d (Max Total)", m_enemies.size(), m_maxEnemies);
    ImGui::Text("Spawn Timer: %.2f / %.2f", m_spawnTimer, m_spawnInterval);


    if (ImGui::CollapsingHeader("Enemy List")) {
        for (size_t i = 0; i < m_enemies.size(); ++i)
        {
            std::string enemyNodeId = "Enemy " + std::to_string(i);
            if (ImGui::TreeNode(enemyNodeId.c_str()))
            {
                m_enemies[i]->DebugDraw();
                ImGui::TreePop();
            }
        }
    }
}