// This includes
#include "Enemy.h"

// Local includes
#include "Player.h" 
#include "Renderer.h"
#include "LogManager.h"
#include "Texture.h" 
#include "imgui/imgui.h"
#include <algorithm> 
#include <cmath> 

const float ENEMY_VISUAL_SCALE = 1.5f;

Enemy::Enemy()
    : Entity()
    , m_currentState(EnemyState::IDLE)
    , m_pTargetPlayer(nullptr)
    , m_bFacingRight(false)
    , m_damage(10)
    , m_moveSpeed(120.0f) // enemy movespeed
    , m_attackRange(60.0f) 
    , m_detectionRange(2000.0f) // for now so that the enemies would find the player straight away
    , m_attackCooldown(2.5f)
    , m_timeSinceLastAttack(m_attackCooldown) // Ready to attack initially
    , m_attackWindUpTime(0.5f) // damage applied 0.5s into attack animation
    , m_currentAttackTime(0.0f)
    , m_pStaticEnemy(nullptr)
{
    SetMaxHealth(50, true); // Enemy specific health
    SetRadius(static_cast<float>(ENEMY_DEFAULT_SPRITE_WIDTH) * ENEMY_VISUAL_SCALE / 2.5f); // scales sprite
}

Enemy::~Enemy()
{
    delete m_pStaticEnemy;
    m_pStaticEnemy = nullptr;

    for (const auto& pair : m_animatedSprites)
    {
        delete pair.second;
    }
    m_animatedSprites.clear();
    m_pTargetPlayer = nullptr;
}

bool Enemy::Initialise(Renderer& renderer, const Vector2& startPosition)
{
    if (!Entity::Initialise(renderer)) return false;

    m_position = startPosition;
    m_position.y = kGroundLevel; // Ensure Y is on ground

    // Determine initial facing direction based on player position relative to enemy
    if (m_pTargetPlayer) 
    {
        m_bFacingRight = (m_pTargetPlayer->GetPosition().x > m_position.x);
    }
    else
    {
        m_bFacingRight = false;
    }

    m_pStaticEnemy = renderer.CreateSprite("assets/enemy/Bat-IdleFly.png");
    if (m_pStaticEnemy)
    {
        m_pStaticEnemy->SetScale(ENEMY_VISUAL_SCALE, ENEMY_VISUAL_SCALE);
    }

    if (!InitialiseAnimatedSprite(renderer, EnemyState::IDLE, "assets/enemy/Bat-IdleFly.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.2f, true)) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyState::WALKING, "assets/enemy/Bat-Run.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.15f, true)) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyState::ATTACKING, "assets/enemy/Bat-Attack1.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.1f, false, [this]() { this->OnAttackAnimationComplete(); })) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyState::HURT, "assets/enemy/Bat-Hurt.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.3f, false, [this]() { this->OnHurtAnimationComplete(); })) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyState::DEATH, "assets/enemy/Bat-Die.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.15f, false, [this]() { this->OnDeathAnimationComplete(); })) return false;

    TransitionToState(EnemyState::IDLE);
    return true;
}


bool Enemy::InitialiseAnimatedSprite(Renderer& renderer, EnemyState state, const char* pcFilename,
    int frameWidth, int frameHeight, float frameDuration, bool loop, AnimationCallBack onComplete)
{
    AnimatedSprite* sprite = renderer.CreateAnimatedSprite(pcFilename);
    if (!sprite)
    {
        LogManager::GetInstance().Log(("Failed to create animated sprite for enemy: " + std::string(pcFilename)).c_str());
        return false;
    }
    sprite->SetupFrames(frameWidth, frameHeight);
    sprite->SetFrameDuration(frameDuration);
    sprite->SetLooping(loop);
    if (onComplete)
    {
        sprite->SetAnimationCompleteCallback(onComplete);
    }
    sprite->SetScale(ENEMY_VISUAL_SCALE, ENEMY_VISUAL_SCALE);
    m_animatedSprites[state] = sprite;
    return true;
}

void Enemy::Process(float deltaTime)
{
    if (!m_bAlive)
    {
        AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
        if (m_currentState == EnemyState::DEATH && currentSprite) {
            UpdateSprite(currentSprite, deltaTime); // Process death animation
        }
        return;
    }

    UpdateAI(deltaTime);

    if (m_currentState == EnemyState::WALKING)
    {
        MoveToPlayer(deltaTime);
    }

    if (m_position.y > kGroundLevel) { // Basic ground clamping
        m_position.y = kGroundLevel;
        m_velocity.y = 0;
    }

    // attack timing and damage
    m_timeSinceLastAttack += deltaTime;
    if (m_currentState == EnemyState::ATTACKING)
    {
        m_currentAttackTime += deltaTime;

        if (m_currentAttackTime >= m_attackWindUpTime && m_currentAttackTime != -1.0f)
        {
            if (m_pTargetPlayer && m_pTargetPlayer->IsAlive())
            {
                Vector2 directionToPlayer = m_pTargetPlayer->GetPosition() - m_position;
                float distanceToPlayer = directionToPlayer.Length();

                bool playerInFront = (m_bFacingRight && directionToPlayer.x >= 0) ||
                    (!m_bFacingRight && directionToPlayer.x <= 0);

                if (playerInFront && distanceToPlayer < (m_attackRange + m_pTargetPlayer->GetRadius()))
                {
                    m_pTargetPlayer->TakeDamage(m_damage);
                    LogManager::GetInstance().Log("Enemy dealt damage to player.");
                }
            }
            m_currentAttackTime = -1.0f;
        }
    }
    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (currentSprite)
    {
        UpdateSprite(currentSprite, deltaTime);
    }
    
}

void Enemy::UpdateAI(float deltaTime)
{
    if (!m_pTargetPlayer || !m_pTargetPlayer->IsAlive() ||
        m_currentState == EnemyState::ATTACKING ||
        m_currentState == EnemyState::HURT ||
        m_currentState == EnemyState::DEATH)
    {
        if (m_currentState != EnemyState::ATTACKING &&
            m_currentState != EnemyState::HURT &&
            m_currentState != EnemyState::DEATH)
        {
            if (m_currentState == EnemyState::WALKING) TransitionToState(EnemyState::IDLE);
        }
        return;
    }

    Vector2 directionToPlayer = m_pTargetPlayer->GetPosition() - m_position;
    float distanceToPlayerSquared = directionToPlayer.LengthSquared(); // Use squared for performance

    // Face the player
    if (directionToPlayer.x > 0.1f) m_bFacingRight = true;
    else if (directionToPlayer.x < -0.1f) m_bFacingRight = false;

    float effectiveAttackRange = m_attackRange; // Can be adjusted based on player size for more precision
    float detectionRangeSq = m_detectionRange * m_detectionRange;
    float attackRangeSq = effectiveAttackRange * effectiveAttackRange;

    if (distanceToPlayerSquared <= attackRangeSq)
    {
        if (m_timeSinceLastAttack >= m_attackCooldown)
        {
            TransitionToState(EnemyState::ATTACKING);
            m_timeSinceLastAttack = 0.0f;
            m_currentAttackTime = 0.0f; // Reset wind-up timer
        }
        else if (m_currentState != EnemyState::ATTACKING) 
        { // Ensure not already attacking
            TransitionToState(EnemyState::IDLE);
        }
    }
    else if (distanceToPlayerSquared <= detectionRangeSq) // if the player is in detection range but not in attacking range
    {
        if (m_currentState != EnemyState::WALKING && m_currentState != EnemyState::ATTACKING)
        {
            TransitionToState(EnemyState::WALKING);
        }
    }
    else
    {
        //m_velocity.x = 0;
        if (m_currentState == EnemyState::WALKING)
        {
            TransitionToState(EnemyState::IDLE);
        }
    }
}

void Enemy::MoveToPlayer(float deltaTime) 
{
    if (!m_pTargetPlayer) return;
    Vector2 playerPos = m_pTargetPlayer->GetPosition();
    float directionX = 0.0f;
    float deadZone = 2.0f;
    
    if (playerPos.x < m_position.x - deadZone)
    {
        directionX = -1.0f;
    }
    else if (playerPos.x > m_position.x + deadZone)
    {
        directionX = 1.0f;
    }

    m_position.x += directionX * m_moveSpeed * deltaTime;
}

void Enemy::Draw(Renderer& renderer)
{
    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (!m_bAlive && m_currentState == EnemyState::DEATH && currentSprite && currentSprite->IsAnimationComplete()) {
        return; // Dead and animation finished, scene will clean up
    }

    if (currentSprite)
    {
        currentSprite->Draw(renderer);
    }
    else 
    {
    }
}

void Enemy::UpdateSprite(AnimatedSprite* sprite, float deltaTime)
{
    if (!sprite) return;
    sprite->Process(deltaTime);
    sprite->SetX(static_cast<int>(m_position.x));
    sprite->SetY(static_cast<int>(m_position.y));
    sprite->SetFlipHorizontal(!m_bFacingRight);
}


AnimatedSprite* Enemy::GetCurrentAnimatedSprite()
{
    auto it = m_animatedSprites.find(m_currentState);
    if (it != m_animatedSprites.end())
    {
        return it->second;
    }
    auto idle_it = m_animatedSprites.find(EnemyState::IDLE);
    if (idle_it != m_animatedSprites.end()) return idle_it->second;
    return nullptr;
}

void Enemy::TransitionToState(EnemyState newState)
{
    if (m_currentState == newState && GetCurrentAnimatedSprite() && GetCurrentAnimatedSprite()->IsAnimating()) return;

    if (m_currentState == EnemyState::DEATH) return;
    if (m_currentState == EnemyState::HURT && GetCurrentAnimatedSprite() && !GetCurrentAnimatedSprite()->IsAnimationComplete() && newState != EnemyState::DEATH) return;

    m_currentState = newState;
    AnimatedSprite* newSprite = GetCurrentAnimatedSprite();
    if (newSprite)
    {
        newSprite->Restart();
        newSprite->Animate();
    }
    else {
        LogManager::GetInstance().Log(("Enemy::TransitionToState: No sprite found for new state " + std::to_string(static_cast<int>(newState))).c_str());
    }
}

void Enemy::TakeDamage(int amount)
{
    if (!m_bAlive) return;

    Entity::TakeDamage(amount); // Base class handles health and m_bAlive flag

    if (!m_bAlive)
    {
        TransitionToState(EnemyState::DEATH);
        m_velocity.Set(0.0f, 0.0f); // Stop all movement on death
    }
    else if (amount > 0) // Took damage and if still alive
    {
        TransitionToState(EnemyState::HURT);
    }
}

void Enemy::OnAttackAnimationComplete() 
{
    if (m_currentState == EnemyState::ATTACKING) 
    {
        TransitionToState(EnemyState::IDLE); // Or back to walking if player is still in range but needs cooldown
    }
    m_currentAttackTime = 0.0f; // Ensure attack timer is reset
}


void Enemy::OnHurtAnimationComplete()
{
    if (m_currentState == EnemyState::HURT)
    {
        TransitionToState(EnemyState::IDLE);
    }
}

void Enemy::OnDeathAnimationComplete()
{
    LogManager::GetInstance().Log("Enemy death animation complete. Ready for cleanup.");
}

bool Enemy::IsAttacking() const
{
    return m_currentState == EnemyState::ATTACKING;
}

void Enemy::DebugDraw()
{
    Entity::DebugDraw();

    if (ImGui::TreeNode((void*)this, "Enemy Specifics"))
    {
        const char* stateNames[] = { "IDLE", "WALKING", "ATTACKING", "HURT", "DEATH" };
        int stateIndex = static_cast<int>(m_currentState);
        if (stateIndex >= 0 && stateIndex < sizeof(stateNames) / sizeof(char*)) 
        {
            ImGui::Text("Enemy State: %s", stateNames[stateIndex]);
        }
        else 
        {
            ImGui::Text("Enemy State: UNKNOWN (%d)", stateIndex);
        }
            
        ImGui::Text("Facing: %s", m_bFacingRight ? "Right" : "Left");
        ImGui::SliderInt("Attack Damage", &m_damage, 0, 50);
        ImGui::DragFloat("Move Speed", &m_moveSpeed, 1.0f, 0.0f, 300.0f);
        ImGui::DragFloat("Attack Range", &m_attackRange, 1.0f, 0.0f, 200.0f);
        ImGui::DragFloat("Detection Range", &m_detectionRange, 1.0f, 0.0f, 1000.0f);
        ImGui::Text("Attack Cooldown: %.2f / %.2f", m_timeSinceLastAttack, m_attackCooldown);
        ImGui::Text("Attack Windup Timer: %.2f / %.2f", m_currentAttackTime, m_attackWindUpTime);

        if (m_pTargetPlayer) ImGui::Text("Dist to Player: %.2f", (m_pTargetPlayer->GetPosition() - m_position).Length());
        else ImGui::Text("Target Player: None");

        AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
        if (currentSprite)
        {
            ImGui::Separator();
            ImGui::Text("Current Enemy AnimatedSprite:");
            currentSprite->DebugDraw();
        }
        ImGui::TreePop();
    }
}

Vector2& Enemy::GetPosition()
{
    return m_position;
}