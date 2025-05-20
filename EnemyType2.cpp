// This includes
#include "EnemyType2.h"

// Local includes
#include "Player.h" 
#include "Renderer.h"
#include "LogManager.h"
#include "Texture.h" 

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <algorithm> 
#include <cmath> 
#include <cstdlib>

const float ENEMYTYPE2_VISUAL_SCALE = 2.5f;

EnemyType2::EnemyType2()
    : Entity()
    , m_currentState(EnemyType2State::IDLE)
    , m_pTargetPlayer(nullptr)
    , m_bFacingRight(false)
    , m_iDamage(25)
    , m_moveSpeed(60.0f) // Enemy movespeed
    , m_attackRange(70.0f)
    , m_detectionRange(2000.0f) // For now so that the enemies would find the player straight away
    , m_attackCD(5.0f)
    , m_timeSinceAttack(m_attackCD) // Ready to attack initially
    , m_bHasDealtDMG(false)
    , m_pStaticEnemy(nullptr)
    , m_minEssenceDrop(10)
    , m_maxEssenceDrop(30)
    , m_pSceneRef(nullptr)
{
    SetMaxHealth(125, true); // Enemy specific health
}

EnemyType2::~EnemyType2()
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

bool EnemyType2::Initialise(Renderer& renderer, const Vector2& startPosition)
{
    if (!Entity::Initialise(renderer)) return false;

    m_position = startPosition;
    m_position.y = kGroundLevel;

    // Determine initial facing direction based on player position relative to enemy
    if (m_pTargetPlayer)
    {
        m_bFacingRight = (m_pTargetPlayer->GetPosition().x > m_position.x);
    }
    else
    {
        m_bFacingRight = false;
    }

    m_pStaticEnemy = renderer.CreateSprite("assets/enemyType2/Idle.png");
    if (m_pStaticEnemy)
    {
        m_pStaticEnemy->SetScale(ENEMYTYPE2_VISUAL_SCALE, ENEMYTYPE2_VISUAL_SCALE);
    }

    // Radius for specific animations for TYPE 2
    m_baseRadius = (static_cast<float>(ENEMY_DEFAULT_SPRITE_WIDTH) * ENEMYTYPE2_VISUAL_SCALE) / 2.0f;
    m_strikePhaseRadius = (static_cast<float>(ENEMY_DEFAULT_SPRITE_ATTACKSTRIKE_WIDTH) * ENEMYTYPE2_VISUAL_SCALE) / 2.0f;
    SetRadius(m_baseRadius);

    if (!InitialiseAnimatedSprite(renderer, EnemyType2State::IDLE, "assets/enemyType2/Idle.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.1f, true)) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyType2State::WALKING, "assets/enemyType2/Walk.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.15f, true)) return false;
    // Attack Sequence Section
    if (!InitialiseAnimatedSprite(renderer, EnemyType2State::ATTACKING_WINDUP, "assets/enemyType2/Attack_Windup.png", ENEMY_DEFAULT_SPRITE_ATTACKWINDUP_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.08f, false, [this]() { this->TransitionToState(EnemyType2State::ATTACKING_STRIKE); })) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyType2State::ATTACKING_STRIKE, "assets/enemyType2/Attack_Strike.png", ENEMY_DEFAULT_SPRITE_ATTACKSTRIKE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.08f, false, [this]() { this->TransitionToState(EnemyType2State::ATTACKING_OVER); })) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyType2State::ATTACKING_OVER, "assets/enemyType2/Attack_Over.png", ENEMY_DEFAULT_SPRITE_ATTACK_END_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.1f, false, [this]() { this->OnAttackSequenceComplete(); })) return false;
    
    if (!InitialiseAnimatedSprite(renderer, EnemyType2State::HURT, "assets/enemyType2/Hurt.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.1f, false, [this]() { this->OnHurtAnimationComplete(); })) return false;
    if (!InitialiseAnimatedSprite(renderer, EnemyType2State::DEATH, "assets/enemyType2/Death.png", ENEMY_DEFAULT_SPRITE_WIDTH, ENEMY_DEFAULT_SPRITE_HEIGHT, 0.08f, false, [this]() { this->OnDeathAnimationComplete(); })) return false;

    TransitionToState(EnemyType2State::IDLE);
    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (currentSprite)
    {
        UpdateSprite(currentSprite, 0.0f);
    }
    return true;
}


bool EnemyType2::InitialiseAnimatedSprite(Renderer& renderer, EnemyType2State state, const char* pcFilename,
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
    sprite->SetScale(ENEMYTYPE2_VISUAL_SCALE, ENEMYTYPE2_VISUAL_SCALE);
    m_animatedSprites[state] = sprite;
    return true;
}

void EnemyType2::Process(float deltaTime)
{
    if (!m_bAlive)
    {
        AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
        if (m_currentState == EnemyType2State::DEATH && currentSprite)
        {
            UpdateSprite(currentSprite, deltaTime);
        }
        return;
    }

    UpdateAI(deltaTime);

    if (m_currentState == EnemyType2State::WALKING)
    {
        MoveToPlayer(deltaTime);
    }

    if (m_position.y > kGroundLevel) { // Basic ground clamping
        m_position.y = kGroundLevel;
        m_velocity.y = 0;
    }

    m_timeSinceAttack += deltaTime; // CD for attack seqeuence


    if (m_currentState == EnemyType2State::ATTACKING_WINDUP ||
        m_currentState == EnemyType2State::ATTACKING_STRIKE ||
        m_currentState == EnemyType2State::ATTACKING_OVER)
    {
        m_currentPhaseTimer += deltaTime;
    }

    // DMG for ATTACK_STRIKE phase
    if (m_currentState == EnemyType2State::ATTACKING_STRIKE)
    {
        const float damageStart = 0.05f;
        const float damageEnd = 0.25f;

        if (!m_bHasDealtDMG && m_currentPhaseTimer >= damageStart && m_currentPhaseTimer <= damageEnd)
        {
            if (m_pTargetPlayer && m_pTargetPlayer->IsAlive())
            {
                Vector2 directionToPlayer = m_pTargetPlayer->GetPosition() - m_position;

                float distanceToPlayer = directionToPlayer.Length();
                bool playerInFront = (m_bFacingRight && directionToPlayer.x >= 0) || (!m_bFacingRight && directionToPlayer.x <= 0);

                if (playerInFront && distanceToPlayer < (m_attackRange + m_pTargetPlayer->GetRadius()))
                {
                    m_pTargetPlayer->TakeDamage(m_iDamage);
                    m_bHasDealtDMG = true; // Ensure damage is dealt only once per strike
                    LogManager::GetInstance().Log("EnemyType2 (STRIKE) dealt damage to player.");
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

void EnemyType2::UpdateAI(float deltaTime)
{
    bool isCurrentlyInAttackSequence = (m_currentState == EnemyType2State::ATTACKING_WINDUP ||
        m_currentState == EnemyType2State::ATTACKING_STRIKE ||
        m_currentState == EnemyType2State::ATTACKING_OVER);

    if (!m_pTargetPlayer || !m_pTargetPlayer->IsAlive() ||
        isCurrentlyInAttackSequence ||
        m_currentState == EnemyType2State::HURT ||
        m_currentState == EnemyType2State::DEATH)
    {
        if (!isCurrentlyInAttackSequence &&
            m_currentState != EnemyType2State::HURT &&
            m_currentState == EnemyType2State::WALKING &&
            m_currentState != EnemyType2State::DEATH)
        {
            TransitionToState(EnemyType2State::IDLE);
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
            TransitionToState(EnemyType2State::ATTACKING_WINDUP);
            m_timeSinceAttack = 0.0f;
        }
        else if (!isCurrentlyInAttackSequence)
        {
            TransitionToState(EnemyType2State::IDLE);
        }
    }
    else if (distanceToPlayerSquared <= detectionRangeSq) // if the player is in detection range but not in attacking range
    {
        if (m_currentState != EnemyType2State::WALKING && !isCurrentlyInAttackSequence)
        {
            TransitionToState(EnemyType2State::WALKING);
        }
    }
    else
    {
        if (m_currentState == EnemyType2State::WALKING)
        {
            TransitionToState(EnemyType2State::IDLE);
        }
    }
}

void EnemyType2::MoveToPlayer(float deltaTime)
{
    if (!m_pTargetPlayer) return;
    Vector2 playerPos = m_pTargetPlayer->GetPosition();
    float directionX = 0.0f;
    float deadZone = GetRadius() * 0.5f;

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

void EnemyType2::Draw(Renderer& renderer)
{
    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (!m_bAlive && m_currentState == EnemyType2State::DEATH && currentSprite && currentSprite->IsAnimationComplete()) {
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

void EnemyType2::UpdateSprite(AnimatedSprite* sprite, float deltaTime)
{
    if (!sprite) return;
    sprite->Process(deltaTime);
    sprite->SetX(static_cast<int>(m_position.x));
    sprite->SetY(static_cast<int>(m_position.y));
    sprite->SetFlipHorizontal(m_bFacingRight);
}


AnimatedSprite* EnemyType2::GetCurrentAnimatedSprite()
{
    auto it = m_animatedSprites.find(m_currentState);
    if (it != m_animatedSprites.end())
    {
        return it->second;
    }
    auto idle_it = m_animatedSprites.find(EnemyType2State::IDLE);
    if (idle_it != m_animatedSprites.end()) return idle_it->second;
    return nullptr;
}

void EnemyType2::TransitionToState(EnemyType2State newState)
{
    bool wasInAnyAttackPhase = (m_currentState == EnemyType2State::ATTACKING_WINDUP ||
        m_currentState == EnemyType2State::ATTACKING_STRIKE ||
        m_currentState == EnemyType2State::ATTACKING_OVER);

    bool willBeInAnyAttackPhase = (newState == EnemyType2State::ATTACKING_WINDUP ||
        newState == EnemyType2State::ATTACKING_STRIKE ||
        newState == EnemyType2State::ATTACKING_OVER);

    if (m_currentState == newState && newState != EnemyType2State::ATTACKING_WINDUP)
    {
        AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
        if (currentSprite && currentSprite->IsAnimating() && !currentSprite->IsLooping())
        {
            return;
        }
    }
    if (m_currentState == EnemyType2State::DEATH && newState != EnemyType2State::DEATH) return; // Cannot leave death state
    if (m_currentState == EnemyType2State::HURT && GetCurrentAnimatedSprite() && !GetCurrentAnimatedSprite()->IsAnimationComplete() && newState != EnemyType2State::DEATH) return; // Must finish hurt anim

    EnemyType2State oldState = m_currentState;
    m_currentState = newState;
    m_currentPhaseTimer = 0.0f; // Reset phase timer for any new state

    // Manage Collision Radius and attack flags
    if (newState == EnemyType2State::ATTACKING_STRIKE)
    {
        SetRadius(m_strikePhaseRadius);
        m_bHasDealtDMG = false;
    }
    else if (newState == EnemyType2State::ATTACKING_WINDUP || newState == EnemyType2State::ATTACKING_OVER)
    {
        SetRadius(m_baseRadius);
        if (newState == EnemyType2State::ATTACKING_WINDUP)
        {
            m_bHasDealtDMG = false;
        }
    }
    else if (wasInAnyAttackPhase && !willBeInAnyAttackPhase) 
    {
        // Transitioning OUT of all attack phases to a non-attack phase
        SetRadius(m_baseRadius);
    }

    AnimatedSprite* newSprite = GetCurrentAnimatedSprite();
    if (newSprite)
    {
        newSprite->Restart();
        newSprite->Animate();
    }
    else 
    {
        LogManager::GetInstance().Log(("EnemyType2::TransitionToState: No sprite found for new state " + std::to_string(static_cast<int>(newState))).c_str());
    }
}

void EnemyType2::SetSceneReference(SceneAbyssWalker* scene)
{
    m_pSceneRef = scene;
}

void EnemyType2::TakeDamage(int amount)
{
    if (!m_bAlive) return;

    bool wasAlive = m_bAlive;
    bool wasInAnyAttackPhase = (m_currentState == EnemyType2State::ATTACKING_WINDUP ||
                                m_currentState == EnemyType2State::ATTACKING_STRIKE ||
                                m_currentState == EnemyType2State::ATTACKING_OVER);

    Entity::TakeDamage(amount);

    if (wasAlive && !m_bAlive)
    {
        TransitionToState(EnemyType2State::DEATH);
        m_velocity.Set(0.0f, 0.0f);
        if (wasInAnyAttackPhase) 
        {
            SetRadius(m_baseRadius); // Reset radius if died during attack
        }

        if (m_pTargetPlayer)
        {
            int droppedEssence = 0;
            if (m_maxEssenceDrop >= m_minEssenceDrop)
            {
                droppedEssence = (rand() & (m_maxEssenceDrop - m_minEssenceDrop + 1)) + m_minEssenceDrop;
            }

            else
            {
                droppedEssence = m_minEssenceDrop;
            }

            if (droppedEssence > 0)
            {
                m_pTargetPlayer->GainEssence(droppedEssence);
            }
        }
    }
    else if (amount > 0) // Took damage and still alive
    {
        if (wasInAnyAttackPhase) 
        {
            SetRadius(m_baseRadius); // Reset radius if hurt during attack
        }
        TransitionToState(EnemyType2State::HURT);
    }
}

void EnemyType2::OnAttackSequenceComplete()
{
    if (m_currentState == EnemyType2State::ATTACKING_OVER)
    {
        m_bHasDealtDMG = false;
        TransitionToState(EnemyType2State::IDLE); // Or back to walking if player is still in range but needs cooldown
    }
}


void EnemyType2::OnHurtAnimationComplete()
{
    if (m_currentState == EnemyType2State::HURT)
    {
        TransitionToState(EnemyType2State::IDLE);
    }
}

void EnemyType2::OnDeathAnimationComplete()
{
    LogManager::GetInstance().Log("Enemy death animation complete. Ready for cleanup.");
}

bool EnemyType2::IsAttacking() const
{
    return m_currentState == EnemyType2State::ATTACKING_WINDUP ||
           m_currentState == EnemyType2State::ATTACKING_STRIKE ||
           m_currentState == EnemyType2State::ATTACKING_OVER;
}

void EnemyType2::DebugDraw()
{
    Entity::DebugDraw();

    if (ImGui::TreeNode((void*)this, "Enemy Type 2 Specifics"))
    {
        const char* stateNames[] = { "IDLE", "WALKING", "ATTACKING", "HURT", "DEATH" };
        int stateIndex = static_cast<int>(m_currentState);
        if (stateIndex >= 0 && stateIndex < sizeof(stateNames) / sizeof(char*))
        {
            ImGui::Text("Enemy Type2 State: %s", stateNames[stateIndex]);
        }
        else
        {
            ImGui::Text("Enemy State: UNKNOWN (%d)", stateIndex);
        }

        ImGui::Text("Facing: %s", m_bFacingRight ? "Right" : "Left");
        ImGui::SliderInt("Attack Damage", &m_iDamage, 0, 50);
        ImGui::DragFloat("Move Speed", &m_moveSpeed, 1.0f, 0.0f, 300.0f);

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

Vector2& EnemyType2::GetPosition()
{
    return m_position;
}