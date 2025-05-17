// This includes
#include "EnemyBat.h"

// Local includes
#include "Player.h" 
#include "Renderer.h"
#include "LogManager.h"
#include "Texture.h" 
#include "imgui/imgui.h"
#include <algorithm> 
#include <cmath> 

const float ENEMYBAT_VISUAL_SCALE = 1.5f;

EnemyBat::EnemyBat()
    : Entity()
    , m_currentState(EnemyBatState::IDLE)
    , m_pTargetPlayer(nullptr)
    , m_bFacingRight(false)
    , m_iDamage(10)
    , m_moveSpeed(110.0f) // Enemy movespeed
    , m_attackRange(50.0f) 
    , m_detectionRange(2000.0f) // For now so that the enemies would find the player straight away
    , m_attackCD(5.0f)
    , m_timeSinceAttack(m_attackCD) // Ready to attack initially
    , m_attackWindUpTime(0.5f) // Damage applied 0.5s into attack animation
    , m_currentAttackTime(0.0f)
    , m_bHasDealtDMG(false)
    , m_pStaticEnemy(nullptr)
{
    SetMaxHealth(50, true); // Enemy specific health
    SetRadius(static_cast<float>(ENEMY_DEFAULT_SPRITE_WIDTH) * ENEMYBAT_VISUAL_SCALE / 2.5f); // Scales sprite
}

EnemyBat::~EnemyBat()
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

bool EnemyBat::Initialise(Renderer& renderer, const Vector2& startPosition)
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

    m_pStaticEnemy = renderer.CreateSprite("assets/enemyBat/Bat-IdleFly.png");
    if (m_pStaticEnemy)
    {
        m_pStaticEnemy->SetScale(ENEMYBAT_VISUAL_SCALE, ENEMYBAT_VISUAL_SCALE);
    }

    if (!InitialiseAnimatedSprite(renderer, EnemyBatState::IDLE, "assets/enemyBat/Bat-IdleFly.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.2f, true)) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyBatState::WALKING, "assets/enemyBat/Bat-Run.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.15f, true)) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyBatState::ATTACKING, "assets/enemyBat/Bat-Attack1.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.1f, false, [this]() { this->OnAttackAnimationComplete(); })) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyBatState::HURT, "assets/enemyBat/Bat-Hurt.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.3f, false, [this]() { this->OnHurtAnimationComplete(); })) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyBatState::DEATH, "assets/enemyBat/Bat-Die.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.15f, false, [this]() { this->OnDeathAnimationComplete(); })) return false;

    TransitionToState(EnemyBatState::IDLE);
    return true;
}


bool EnemyBat::InitialiseAnimatedSprite(Renderer& renderer, EnemyBatState state, const char* pcFilename,
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
    sprite->SetScale(ENEMYBAT_VISUAL_SCALE, ENEMYBAT_VISUAL_SCALE);
    m_animatedSprites[state] = sprite;
    return true;
}

void EnemyBat::Process(float deltaTime)
{
    if (!m_bAlive)
    {
        AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
        if (m_currentState == EnemyBatState::DEATH && currentSprite) 
        {
            UpdateSprite(currentSprite, deltaTime); // Process death animation
        }
        return;
    }

    UpdateAI(deltaTime);

    if (m_currentState == EnemyBatState::WALKING)
    {
        MoveToPlayer(deltaTime);
    }

    if (m_position.y > kGroundLevel) { // Basic ground clamping
        m_position.y = kGroundLevel;
        m_velocity.y = 0;
    }

    // attack timing and damage
    m_timeSinceAttack += deltaTime;
    if (m_currentState == EnemyBatState::ATTACKING)
    {
        m_currentAttackTime += deltaTime;

        if (m_currentAttackTime >= m_attackWindUpTime && !m_bHasDealtDMG)
        {
            AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
            if (currentSprite && m_pTargetPlayer && m_pTargetPlayer->IsAlive())
            {
                Vector2 directionToPlayer = m_pTargetPlayer->GetPosition() - m_position;
                float distanceToPlayer = directionToPlayer.Length();

                bool playerInFront = (m_bFacingRight && directionToPlayer.x >= 0) ||
                    (!m_bFacingRight && directionToPlayer.x <= 0);

                if (playerInFront &&
                    distanceToPlayer < (m_attackRange + m_pTargetPlayer->GetRadius()) &&
                    m_currentAttackTime <= m_attackWindUpTime + 0.15f)
                {
                    m_pTargetPlayer->TakeDamage(m_iDamage);
                    m_bHasDealtDMG = true;
                    LogManager::GetInstance().Log("Enemy dealt damage to player.");
                }
            }
        }
    }

    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (currentSprite)
    {
        UpdateSprite(currentSprite, deltaTime);
    } 
}

void EnemyBat::UpdateAI(float deltaTime)
{
    if (!m_pTargetPlayer || !m_pTargetPlayer->IsAlive() ||
        m_currentState == EnemyBatState::ATTACKING ||
        m_currentState == EnemyBatState::HURT ||
        m_currentState == EnemyBatState::DEATH)
    {
        if (m_currentState != EnemyBatState::ATTACKING &&
            m_currentState != EnemyBatState::HURT &&
            m_currentState != EnemyBatState::DEATH)
        {
            if (m_currentState == EnemyBatState::WALKING) TransitionToState(EnemyBatState::IDLE);
        }
        return;
    }

    Vector2 directionToPlayer = m_pTargetPlayer->GetPosition() - m_position;
    float distanceToPlayerSquared = directionToPlayer.LengthSquared(); // Use squared for performance

    m_bFacingRight = (directionToPlayer.x > 0.0f);

    float effectiveAttackRange = m_attackRange; // Can be adjusted based on player size for more precision
    float detectionRangeSq = m_detectionRange * m_detectionRange;
    float attackRangeSq = effectiveAttackRange * effectiveAttackRange;

    if (distanceToPlayerSquared <= attackRangeSq)
    {
        if (m_timeSinceAttack >= m_attackCD)
        {
            TransitionToState(EnemyBatState::ATTACKING);
            m_timeSinceAttack = 0.0f;
            m_currentAttackTime = 0.0f; // Reset wind-up timer
        }
        else if (m_currentState != EnemyBatState::ATTACKING) 
        { // Ensure not already attacking
            TransitionToState(EnemyBatState::IDLE);
        }
    }
    else if (distanceToPlayerSquared <= detectionRangeSq) // if the player is in detection range but not in attacking range
    {
        if (m_currentState != EnemyBatState::WALKING && m_currentState != EnemyBatState::ATTACKING)
        {
            TransitionToState(EnemyBatState::WALKING);
        }
    }
    else
    {
        //m_velocity.x = 0;
        if (m_currentState == EnemyBatState::WALKING)
        {
            TransitionToState(EnemyBatState::IDLE);
        }
    }
}

void EnemyBat::MoveToPlayer(float deltaTime) 
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

void EnemyBat::Draw(Renderer& renderer)
{
    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (!m_bAlive && m_currentState == EnemyBatState::DEATH && currentSprite && currentSprite->IsAnimationComplete()) {
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

void EnemyBat::UpdateSprite(AnimatedSprite* sprite, float deltaTime)
{
    if (!sprite) return;
    sprite->Process(deltaTime);
    sprite->SetX(static_cast<int>(m_position.x));
    sprite->SetY(static_cast<int>(m_position.y));
    sprite->SetFlipHorizontal(m_bFacingRight);
}


AnimatedSprite* EnemyBat::GetCurrentAnimatedSprite()
{
    auto it = m_animatedSprites.find(m_currentState);
    if (it != m_animatedSprites.end())
    {
        return it->second;
    }
    auto idle_it = m_animatedSprites.find(EnemyBatState::IDLE);
    if (idle_it != m_animatedSprites.end()) return idle_it->second;
    return nullptr;
}

void EnemyBat::TransitionToState(EnemyBatState newState)
{
    if (m_currentState == newState && GetCurrentAnimatedSprite() && GetCurrentAnimatedSprite()->IsAnimating()) return;

    if (m_currentState == EnemyBatState::DEATH) return;
    if (m_currentState == EnemyBatState::HURT && GetCurrentAnimatedSprite() && !GetCurrentAnimatedSprite()->IsAnimationComplete() && newState != EnemyBatState::DEATH) return;

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

void EnemyBat::TakeDamage(int amount)
{
    if (!m_bAlive) return;

    Entity::TakeDamage(amount); // Base class handles health and m_bAlive flag

    if (!m_bAlive)
    {
        TransitionToState(EnemyBatState::DEATH);
        m_velocity.Set(0.0f, 0.0f); // Stop all movement on death
    }
    else if (amount > 0) // Took damage and if still alive
    {
        TransitionToState(EnemyBatState::HURT);
    }
}

void EnemyBat::OnAttackAnimationComplete() 
{
    if (m_currentState == EnemyBatState::ATTACKING) 
    {
        m_currentAttackTime = 0.0f;
        m_bHasDealtDMG = false;
        TransitionToState(EnemyBatState::IDLE); // Or back to walking if player is still in range but needs cooldown
    }
}


void EnemyBat::OnHurtAnimationComplete()
{
    if (m_currentState == EnemyBatState::HURT)
    {
        TransitionToState(EnemyBatState::IDLE);
    }
}

void EnemyBat::OnDeathAnimationComplete()
{
    LogManager::GetInstance().Log("Enemy death animation complete. Ready for cleanup.");
}

bool EnemyBat::IsAttacking() const
{
    return m_currentState == EnemyBatState::ATTACKING;
}

void EnemyBat::DebugDraw()
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
        ImGui::SliderInt("Attack Damage", &m_iDamage, 0, 50);
        ImGui::DragFloat("Move Speed", &m_moveSpeed, 1.0f, 0.0f, 300.0f);
        ImGui::DragFloat("Attack Range", &m_attackRange, 1.0f, 0.0f, 200.0f);
        ImGui::DragFloat("Detection Range", &m_detectionRange, 1.0f, 0.0f, 1000.0f);
        ImGui::Text("Attack Cooldown: %.2f / %.2f", m_timeSinceAttack, m_attackCD);
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

Vector2& EnemyBat::GetPosition()
{
    return m_position;
}