// This includes
#include "Boss.h"

// Local includes
#include "Player.h" 
#include "Renderer.h"
#include "LogManager.h"
#include "Texture.h" 
#include "SceneAbyssWalker.h"
#include "AnimatedSprite.h"
#include "PlayerStats.h"
#include "TextureManager.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <algorithm> 
#include <cmath> 
#include <cstdlib>

const float Boss::kGroundLevel = 700.0f;
const float Boss::SPELL_EFFECT_VISUAL_SCALE = 2.0f;
const float Boss::BOSS_VISUAL_SCALE = 5.0f;
const float Boss::BOSS_SPELL_WINDUP_DURATION = 0.7f;
const float Boss::BOSS_SPELL_STRIKE_DURATION = 0.5f;
const float Boss::BOSS_SPELL_OVER_DURATION = 0.8f;

Boss::Boss()
    : Entity()
    , m_currentState(BossState::IDLE)
    , m_pTargetPlayer(nullptr)
    , m_bFacingRight(false)
    , m_iDamage(80)
    , m_moveSpeed(50.0f)
    , m_attackRange(200.0f)
    , m_detectionRange(2000.0f)
    , m_attackCD(3.0f)
    , m_timeSinceAttack(m_attackCD) // Ready for melee
    , m_bHasDealtDMG(false)
    , m_pStaticEnemy(nullptr)
    , m_minEssenceDrop(50)
    , m_maxEssenceDrop(900)
    , m_pSceneRef(nullptr)
    , m_currentPhaseTimer(0.0f) // Timer for current boss animation phase (melee or casting)
    , m_baseRadius(0.0f)
    , m_strikePhaseRadius(0.0f)
    // Spell members
    , m_spellCastRange(600.0f)
    , m_spellAttackCD(10.0f)
    , m_timeSinceSpellAttack(m_spellAttackCD) // Ready to cast spell initially
    , m_spellDamage(60)
    , m_pSpellEffectSprite(nullptr)
    , m_pSpellEffectTexture_Windup(nullptr)
    , m_pSpellEffectTexture_Strike(nullptr)
    , m_pSpellEffectTexture_Over(nullptr)
    , m_spellTargetPosition()
    , m_spellPhaseTimer(0.0f) // Timer for the duration of SPELL_WINDUP, SPELL_STRIKE, SPELL_OVER states
    , m_bSpellDamageDealtThisCast(false)
{
    SetMaxHealth(1500, true); // Enemy specific health
}

Boss::~Boss()
{
    delete m_pStaticEnemy;
    m_pStaticEnemy = nullptr;

    for (const auto& pair : m_animatedSprites)
    {
        delete pair.second;
    }
    m_animatedSprites.clear();
    m_pTargetPlayer = nullptr;
    m_pSceneRef = nullptr;
}

bool Boss::Initialise(Renderer& renderer, const Vector2& startPosition)
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

    m_pStaticEnemy = renderer.CreateSprite("assets/boss/Idle.png");
    if (m_pStaticEnemy)
    {
        m_pStaticEnemy->SetScale(BOSS_VISUAL_SCALE, BOSS_VISUAL_SCALE);
    }

    // Radius for specific animations for Boss
    m_baseRadius = (static_cast<float>(BOSS_DEFAULT_SPRITE_WIDTH) * BOSS_VISUAL_SCALE) / 2.0f;
    m_strikePhaseRadius = (static_cast<float>(BOSS_DEFAULT_SPRITE_ATTACKSTRIKE_WIDTH) * BOSS_VISUAL_SCALE) / 2.0f;
    SetRadius(m_baseRadius);

    for (auto& pair : m_animatedSprites) { delete pair.second; }
    m_animatedSprites.clear();

    // -- Loading Boss Anims --
    if (!InitialiseAnimatedSprite(renderer, BossState::IDLE, "assets/boss/Idle.png", BOSS_DEFAULT_SPRITE_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.1f, true)) return false;
    if (!InitialiseAnimatedSprite(renderer, BossState::WALKING, "assets/boss/Walk.png", BOSS_DEFAULT_SPRITE_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.15f, true)) return false;
    
    // Attack Sequence Section
    if (!InitialiseAnimatedSprite(renderer, BossState::ATTACKING_WINDUP, "assets/boss/Attack_Windup.png", BOSS_DEFAULT_SPRITE_ATTACKWINDUP_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.08f, false, [this]() { this->TransitionToState(BossState::ATTACKING_STRIKE); })) return false;
    if (!InitialiseAnimatedSprite(renderer, BossState::ATTACKING_STRIKE, "assets/boss/Attack_Strike.png", BOSS_DEFAULT_SPRITE_ATTACKSTRIKE_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.08f, false, [this]() { this->TransitionToState(BossState::ATTACKING_OVER); })) return false;
    if (!InitialiseAnimatedSprite(renderer, BossState::ATTACKING_OVER, "assets/boss/Attack_Over.png", BOSS_DEFAULT_SPRITE_ATTACK_END_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.1f, false, [this]() { this->OnAttackSequenceComplete(); })) return false;

    // Cast Animation
    if (!InitialiseAnimatedSprite(renderer, BossState::CASTING, "assets/boss/Cast.png", BOSS_DEFAULT_SPRITE_CAST_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.1f, false, [this]() { this->OnCastingAnimComplete(); })) return false;

    // Hurt and Death
    if (!InitialiseAnimatedSprite(renderer, BossState::HURT, "assets/Boss/Hurt.png", BOSS_DEFAULT_SPRITE_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.1f, false, [this]() { this->OnHurtAnimationComplete(); })) return false;
    if (!InitialiseAnimatedSprite(renderer, BossState::DEATH, "assets/Boss/Death.png", BOSS_DEFAULT_SPRITE_WIDTH, BOSS_DEFAULT_SPRITE_HEIGHT, 0.08f, false, [this]() { this->OnDeathAnimationComplete(); })) return false;

    // Spell
    if (renderer.GetTextureManager()) 
    {
        m_pSpellEffectTexture_Windup = renderer.GetTextureManager()->GetTexture("assets/boss/Cast_Windup.png");
        m_pSpellEffectTexture_Strike = renderer.GetTextureManager()->GetTexture("assets/boss/Cast_Strike.png");
        m_pSpellEffectTexture_Over = renderer.GetTextureManager()->GetTexture("assets/boss/Cast_End.png");

        if (!m_pSpellEffectTexture_Windup || !m_pSpellEffectTexture_Strike || !m_pSpellEffectTexture_Over) 
        {
            LogManager::GetInstance().Log("Boss::Initialise - WARNING: Failed to load one or more spell effect textures via TextureManager. Spell visuals might not work.");
        }
    }
    else 
    {
        LogManager::GetInstance().Log("Boss::Initialise - Renderer's TextureManager is null. Cannot load spell effect textures.");
        return false; // Or handle differently
    }

    delete m_pSpellEffectSprite; // Delete if re-initializing
    m_pSpellEffectSprite = new AnimatedSprite();
    if (!m_pSpellEffectSprite) 
    {
        LogManager::GetInstance().Log("Boss::Initialise - Failed to new AnimatedSprite for spell effect.");
        return false;
    }

    m_timeSinceAttack = m_attackCD;
    m_timeSinceSpellAttack = m_spellAttackCD;
    SetMaxHealth(1500, true); // Reset health on initialise
    m_bAlive = true;

    TransitionToState(BossState::IDLE);
    return true;
}

bool Boss::InitialiseAnimatedSprite(Renderer& renderer, BossState state, const char* pcFilename,
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
    sprite->SetScale(BOSS_VISUAL_SCALE, BOSS_VISUAL_SCALE);
    m_animatedSprites[state] = sprite;
    return true;
}

void Boss::Process(float deltaTime)
{
    if (!m_bAlive)
    {
        AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
        // Only process death animation if it's not yet complete
        if (m_currentState == BossState::DEATH && currentSprite && !currentSprite->IsAnimationComplete())
        {
            UpdateSprite(currentSprite, deltaTime);
        }
        return;
    }

    // Update general timers
    m_timeSinceAttack += deltaTime;
    m_timeSinceSpellAttack += deltaTime;
    m_currentPhaseTimer += deltaTime;


    UpdateAI(deltaTime);

    if (m_currentState == BossState::WALKING)
    {
        MoveToPlayer(deltaTime);
    }

    if (m_position.y > kGroundLevel) 
    {
        m_position.y = kGroundLevel;
        m_velocity.y = 0;
    }

    // --- MELEE ATTACK DAMAGE LOGIC ---
    if (m_currentState == BossState::ATTACKING_STRIKE)
    {
        AnimatedSprite* attackSprite = GetCurrentAnimatedSprite();
        bool isDamageFrame = false;
        if (attackSprite) 
        {
            if (attackSprite->GetCurrentFrame() == 2) 
            {
                isDamageFrame = true;
            }
        }

        if (!m_bHasDealtDMG && isDamageFrame)
        {
            if (m_pTargetPlayer && m_pTargetPlayer->IsAlive())
            {
                Vector2 directionToPlayer = m_pTargetPlayer->GetPosition() - m_position;
                float distanceToPlayer = directionToPlayer.Length();
                bool playerInFront = (m_bFacingRight && directionToPlayer.x >= 0) || (!m_bFacingRight && directionToPlayer.x <= 0);

                float effectiveMeleeRange = GetRadius() + m_pTargetPlayer->GetRadius();

                if (playerInFront && distanceToPlayer < effectiveMeleeRange)
                {
                    m_pTargetPlayer->TakeDamage(m_iDamage);
                    m_bHasDealtDMG = true;
                    LogManager::GetInstance().Log("Boss (MELEE STRIKE) dealt damage to player.");
                }
            }
        }
    }

    // --- SPELL EFFECT PROCESSING (Boss is in SPELL_WINDUP, SPELL_STRIKE, or SPELL_OVER state) ---
    if (m_currentState == BossState::SPELL_WINDUP || m_currentState == BossState::SPELL_STRIKE || m_currentState == BossState::SPELL_OVER)
    {
        m_spellPhaseTimer += deltaTime; 

        if (m_pSpellEffectSprite) 
        {
            m_pSpellEffectSprite->Process(deltaTime); 
        }

        if (m_currentState == BossState::SPELL_WINDUP) 
        {
            if (m_spellPhaseTimer >= BOSS_SPELL_WINDUP_DURATION || (m_pSpellEffectSprite && m_pSpellEffectSprite->IsAnimationComplete())) 
            {
                TransitionToState(BossState::SPELL_STRIKE);
            }
        }
        else if (m_currentState == BossState::SPELL_STRIKE) 
        {
            // Damage logic
            if (!m_bSpellDamageDealtThisCast) 
            {
                if (m_pTargetPlayer && m_pTargetPlayer->IsAlive()) 
                {
                    float spellAOERadius = BOSS_DEFAULT_SPRITE_CASTSTRIKE_WIDTH * SPELL_EFFECT_VISUAL_SCALE / 2.0f;
                    float playerRadius = m_pTargetPlayer->GetRadius();
                    float combinedRadius = playerRadius + spellAOERadius;
                    float distanceToSpellCenterSq = (m_pTargetPlayer->GetPosition() - m_spellTargetPosition).LengthSquared();

                    if (distanceToSpellCenterSq < (combinedRadius * combinedRadius)) 
                    {
                        m_pTargetPlayer->TakeDamage(m_spellDamage);
                        m_bSpellDamageDealtThisCast = true;
                        LogManager::GetInstance().Log("Boss Spell Effect dealt damage to player.");
                    }
                }
            }
            if (m_spellPhaseTimer >= BOSS_SPELL_STRIKE_DURATION || (m_pSpellEffectSprite && m_pSpellEffectSprite->IsAnimationComplete())) 
            {
                TransitionToState(BossState::SPELL_OVER);
            }
        }
        else if (m_currentState == BossState::SPELL_OVER) 
        {
            if (m_spellPhaseTimer >= BOSS_SPELL_OVER_DURATION || (m_pSpellEffectSprite && m_pSpellEffectSprite->IsAnimationComplete())) 
            {
                LogManager::GetInstance().Log("Spell effect finished. Boss transitioning to IDLE.");
                TransitionToState(BossState::IDLE);
            }
        }
    }


    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (currentSprite)
    {
        UpdateSprite(currentSprite, deltaTime);
    }
}

void Boss::UpdateAI(float deltaTime)
{
    if (!m_bAlive) return;

    if (m_currentState == BossState::HURT ||
        m_currentState == BossState::DEATH ||
        IsAttacking() || 
        m_currentState == BossState::CASTING || 
        m_currentState == BossState::SPELL_WINDUP || 
        m_currentState == BossState::SPELL_STRIKE ||
        m_currentState == BossState::SPELL_OVER)
    {
        if ((m_currentState == BossState::IDLE || m_currentState == BossState::WALKING) && (!m_pTargetPlayer || !m_pTargetPlayer->IsAlive())) 
        {
            TransitionToState(BossState::IDLE);
        }
        return;
    }

    if (!m_pTargetPlayer || !m_pTargetPlayer->IsAlive())
    {
        if (m_currentState == BossState::WALKING) TransitionToState(BossState::IDLE);
        return;
    }

    Vector2 directionToPlayer = m_pTargetPlayer->GetPosition() - m_position;
    float distanceToPlayer = directionToPlayer.Length();

    if (std::abs(directionToPlayer.x) > 1.0f)
    {
        m_bFacingRight = (directionToPlayer.x > 0.0f);
    }

    bool canMelee = m_timeSinceAttack >= m_attackCD;
    bool canSpell = m_timeSinceSpellAttack >= m_spellAttackCD;

    // AI Decision Making:
    if (canSpell && distanceToPlayer <= m_spellCastRange && distanceToPlayer > (m_attackRange + 50.0f))
    {
        if (rand() % 2 == 0) // 50% chance to cast spell
        {
            LogManager::GetInstance().Log("Boss AI: Choosing SPELL attack. Transitioning to CASTING.");
            m_spellTargetPosition = m_pTargetPlayer->GetPosition();
            TransitionToState(BossState::CASTING);
            m_timeSinceSpellAttack = 0.0f;
            return;
        }
    }

    if (canMelee && distanceToPlayer <= m_attackRange)
    {
        LogManager::GetInstance().Log("Boss AI: Choosing MELEE attack.");
        TransitionToState(BossState::ATTACKING_WINDUP);
        m_timeSinceAttack = 0.0f;
    }
    else if (distanceToPlayer <= m_detectionRange)
    {
        if (m_currentState != BossState::WALKING)
        {
            TransitionToState(BossState::WALKING);
        }
    }
    else
    {
        if (m_currentState == BossState::WALKING || m_currentState != BossState::IDLE)
        {
            TransitionToState(BossState::IDLE);
        }
    }
}

void Boss::MoveToPlayer(float deltaTime)
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

void Boss::Draw(Renderer& renderer)
{
    AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
    if (!m_bAlive && m_currentState == BossState::DEATH && currentSprite && currentSprite->IsAnimationComplete()) 
    {
        return; // Dead and animation finished, scene will clean up
    }

    if (currentSprite)
    {
        currentSprite->Draw(renderer);
    }
    
    if (m_pSpellEffectSprite &&
        (m_currentState == BossState::SPELL_WINDUP ||
            m_currentState == BossState::SPELL_STRIKE ||
            m_currentState == BossState::SPELL_OVER))
    {
        m_pSpellEffectSprite->Draw(renderer);
    }
}

void Boss::UpdateSprite(AnimatedSprite* sprite, float deltaTime)
{
    if (!sprite) return;
    sprite->Process(deltaTime);

    if (m_bAlive)
    {
        float tintFactor = 0.2f;
        sprite->SetRedTint(tintFactor);
        sprite->SetGreenTint(tintFactor);
        sprite->SetBlueTint(tintFactor);
    }
    else
    {
        sprite->SetRedTint(1.0f);
        sprite->SetGreenTint(1.0f);
        sprite->SetBlueTint(1.0f);
    }

    sprite->SetX(static_cast<int>(m_position.x));
    sprite->SetY(static_cast<int>(m_position.y));
    sprite->SetFlipHorizontal(m_bFacingRight);
}

AnimatedSprite* Boss::GetCurrentAnimatedSprite()
{
    auto it = m_animatedSprites.find(m_currentState);
    if (it != m_animatedSprites.end())
    {
        return it->second;
    }
    auto idle_it = m_animatedSprites.find(BossState::IDLE);
    if (idle_it != m_animatedSprites.end()) return idle_it->second;
    return nullptr;
}

void Boss::TransitionToState(BossState newState)
{
    // Basic state transition guards
    if (m_currentState == newState &&
        newState != BossState::ATTACKING_WINDUP && 
        newState != BossState::CASTING)
    {
        AnimatedSprite* currentSprite = GetCurrentAnimatedSprite();
        if (currentSprite && currentSprite->IsAnimating() && !currentSprite->IsLooping() && !currentSprite->IsAnimationComplete()) 
        {
            return;
        }
    }
    if (m_currentState == BossState::DEATH && newState != BossState::DEATH) return;
    if (m_currentState == BossState::HURT) 
    {
        AnimatedSprite* hurtSprite = GetCurrentAnimatedSprite();
        if (hurtSprite && !hurtSprite->IsAnimationComplete() && newState != BossState::DEATH) return;
    }
    // Prevent interrupting spell sequence unless for HURT or DEATH
    if ((m_currentState == BossState::CASTING ||
        m_currentState == BossState::SPELL_WINDUP ||
        m_currentState == BossState::SPELL_STRIKE ||
        m_currentState == BossState::SPELL_OVER) && (newState != BossState::HURT && newState != BossState::DEATH && newState != BossState::IDLE && newState != BossState::SPELL_WINDUP && newState != BossState::SPELL_STRIKE && newState != BossState::SPELL_OVER)) 
    {
        AnimatedSprite* currentActionSprite = GetCurrentAnimatedSprite();
        if (m_currentState == BossState::CASTING && currentActionSprite && !currentActionSprite->IsAnimationComplete()) return; 
    }

    LogManager::GetInstance().Log(("Boss: Transitioning from state " + std::to_string(static_cast<int>(m_currentState)) + " to " + std::to_string(static_cast<int>(newState))).c_str());

    BossState oldState = m_currentState;
    m_currentState = newState;
    m_currentPhaseTimer = 0.0f;

    // --- Manage Melee Attack Radius ---
    bool wasInAnyAttackPhase = (oldState == BossState::ATTACKING_WINDUP || oldState == BossState::ATTACKING_STRIKE || oldState == BossState::ATTACKING_OVER);

    bool willBeInAnyAttackPhase = (newState == BossState::ATTACKING_WINDUP || newState == BossState::ATTACKING_STRIKE || newState == BossState::ATTACKING_OVER);

    if (newState == BossState::ATTACKING_STRIKE) 
    {
        SetRadius(m_strikePhaseRadius);
        m_bHasDealtDMG = false;
    }
    else if (newState == BossState::ATTACKING_WINDUP || newState == BossState::ATTACKING_OVER) 
    {
        SetRadius(m_baseRadius);
        if (newState == BossState::ATTACKING_WINDUP) m_bHasDealtDMG = false;
    }
    else if (wasInAnyAttackPhase && !willBeInAnyAttackPhase) 
    {
        SetRadius(m_baseRadius);
    }

    // --- Spell Effect Specific Setup on State Transition ---
    if (newState == BossState::CASTING) 
    {
        LogManager::GetInstance().Log("Boss: Entered CASTING state (boss animation begins).");
    }
    else if (newState == BossState::SPELL_WINDUP) 
    {
        LogManager::GetInstance().Log("Boss: Entered SPELL_WINDUP state (spell effect visual starts windup).");
        m_spellPhaseTimer = 0.0f; 
        m_bSpellDamageDealtThisCast = false;
        if (m_pSpellEffectSprite && m_pSpellEffectTexture_Windup) 
        {
            m_pSpellEffectSprite->Initialise(*m_pSpellEffectTexture_Windup);
            m_pSpellEffectSprite->SetupFrames(BOSS_DEFAULT_SPRITE_CASTWINDUP_WIDTH, BOSS_DEFAULT_SPRITE_CASTWINDUP_HEIGHT);
            int totalFrames = m_pSpellEffectSprite->GetTotalFrames();
            m_pSpellEffectSprite->SetFrameDuration(totalFrames > 0 ? BOSS_SPELL_WINDUP_DURATION / totalFrames : 0.1f);
            m_pSpellEffectSprite->SetLooping(false);
            m_pSpellEffectSprite->SetX(static_cast<int>(m_spellTargetPosition.x));
            m_pSpellEffectSprite->SetY(static_cast<int>(m_spellTargetPosition.y));
            m_pSpellEffectSprite->Restart();
            m_pSpellEffectSprite->Animate();
        }
        else 
        {
            LogManager::GetInstance().Log("Error: Spell effect sprite or windup texture missing for SPELL_WINDUP.");
            TransitionToState(BossState::IDLE);
        }
    }
    else if (newState == BossState::SPELL_STRIKE) 
    {
        LogManager::GetInstance().Log("Boss: Entered SPELL_STRIKE state (spell effect visual strikes).");
        m_spellPhaseTimer = 0.0f; // Reset timer for THIS spell effect phase
        // m_bSpellDamageDealtThisCast is reset in WINDUP or before CASTING
        if (m_pSpellEffectSprite && m_pSpellEffectTexture_Strike) 
        {
            m_pSpellEffectSprite->Initialise(*m_pSpellEffectTexture_Strike);
            m_pSpellEffectSprite->SetupFrames(BOSS_DEFAULT_SPRITE_CASTSTRIKE_WIDTH, BOSS_DEFAULT_SPRITE_CASTSTRIKE_HEIGHT);
            int totalFrames = m_pSpellEffectSprite->GetTotalFrames();
            m_pSpellEffectSprite->SetFrameDuration(totalFrames > 0 ? BOSS_SPELL_STRIKE_DURATION / totalFrames : 0.08f);
            m_pSpellEffectSprite->SetLooping(false);
            m_pSpellEffectSprite->SetX(static_cast<int>(m_spellTargetPosition.x));
            m_pSpellEffectSprite->SetY(static_cast<int>(m_spellTargetPosition.y));
            m_pSpellEffectSprite->Restart();
            m_pSpellEffectSprite->Animate();
        }
        else 
        {
            LogManager::GetInstance().Log("Error: Spell effect sprite or strike texture missing for SPELL_STRIKE.");
            TransitionToState(BossState::IDLE); // Fallback
        }
    }
    else if (newState == BossState::SPELL_OVER) 
    {
        LogManager::GetInstance().Log("Boss: Entered SPELL_OVER state (spell effect visual fades).");
        m_spellPhaseTimer = 0.0f; // Reset timer for THIS spell effect phase
        if (m_pSpellEffectSprite && m_pSpellEffectTexture_Over) 
        {
            m_pSpellEffectSprite->Initialise(*m_pSpellEffectTexture_Over);
            m_pSpellEffectSprite->SetupFrames(BOSS_DEFAULT_SPRITE_CAST_END_WIDTH, BOSS_DEFAULT_SPRITE_CAST_END_HEIGHT);
            int totalFrames = m_pSpellEffectSprite->GetTotalFrames();
            m_pSpellEffectSprite->SetFrameDuration(totalFrames > 0 ? BOSS_SPELL_OVER_DURATION / totalFrames : 0.1f);
            m_pSpellEffectSprite->SetLooping(false);
            m_pSpellEffectSprite->SetX(static_cast<int>(m_spellTargetPosition.x));
            m_pSpellEffectSprite->SetY(static_cast<int>(m_spellTargetPosition.y));
            m_pSpellEffectSprite->Restart();
            m_pSpellEffectSprite->Animate();
        }
        else 
        {
            LogManager::GetInstance().Log("Error: Spell effect sprite or over texture missing for SPELL_OVER.");
            TransitionToState(BossState::IDLE); // Fallback
        }
    }

    AnimatedSprite* newBossSprite = GetCurrentAnimatedSprite(); // Gets the sprite for the boss's body
    if (newBossSprite) 
    {

        AnimatedSprite* spriteToAnimate = nullptr;
        if (newState == BossState::SPELL_WINDUP || newState == BossState::SPELL_STRIKE || newState == BossState::SPELL_OVER) 
        {
            auto it_idle = m_animatedSprites.find(BossState::IDLE);
            if (it_idle != m_animatedSprites.end()) 
            {
                spriteToAnimate = it_idle->second;
            }
        }
        else 
        {
            spriteToAnimate = newBossSprite; // Use the direct animation for the new state
        }

        if (spriteToAnimate) 
        {
            spriteToAnimate->Restart();
            spriteToAnimate->Animate();
        }

    }
    else 
    {
        LogManager::GetInstance().Log(("Boss::TransitionToState: No sprite found for new state " + std::to_string(static_cast<int>(newState))).c_str());
    }
}

void Boss::SetSceneReference(SceneAbyssWalker* scene)
{
    m_pSceneRef = scene;
}

void Boss::TakeDamage(int amount)
{
    if (!m_bAlive) return;

    bool wasAlive = m_bAlive;
    bool wasAttacking = IsAttacking();
    bool wasCasting = IsCastingSpell(); // Check if was in any casting phase

    Entity::TakeDamage(amount);

    if (wasAlive && !m_bAlive)
    {
        LogManager::GetInstance().Log("Boss has died.");
        if (wasAttacking || wasCasting) { // If died during melee or casting animation
            SetRadius(m_baseRadius);
        }
        
        TransitionToState(BossState::DEATH);
        m_velocity.Set(0.0f, 0.0f);

        if (m_pTargetPlayer)
        {
            int droppedEssence = m_minEssenceDrop;
            if (m_maxEssenceDrop > m_minEssenceDrop) 
            {
                droppedEssence += (rand() % (m_maxEssenceDrop - m_minEssenceDrop + 1));
            }
            if (droppedEssence > 0)
            {
                m_pTargetPlayer->GainEssence(droppedEssence);
                LogManager::GetInstance().Log(("Boss dropped " + std::to_string(droppedEssence) + " essence.").c_str());
            }
        }
        if (m_pSceneRef && m_pSceneRef->GetWaveSystem()) 
        {
            // Notify wave system if boss defeat is a specific condition
            // For example: m_pSceneRef->GetWaveSystem()->NotifyBossDefeated();
        }
    }
    else if (amount > 0 && m_bAlive)
    {
        if (m_currentState != BossState::HURT && m_currentState != BossState::DEATH)
        {
            bool shouldInterrupt = true;

            if (shouldInterrupt) {
                if (wasAttacking || wasCasting)
                {
                    SetRadius(m_baseRadius);
                }

                TransitionToState(BossState::HURT);
                LogManager::GetInstance().Log("Boss transitioned to HURT state.");
            }
        }
    }
}

void Boss::OnAttackSequenceComplete()
{
    if (m_currentState == BossState::ATTACKING_OVER)
    {
        m_bHasDealtDMG = false;
        TransitionToState(BossState::IDLE); // Or back to walking if player is still in range but needs cooldown
    }
}


void Boss::OnHurtAnimationComplete()
{
    if (m_currentState == BossState::HURT)
    {
        TransitionToState(BossState::IDLE);
    }
}

void Boss::OnDeathAnimationComplete()
{
    LogManager::GetInstance().Log("Enemy death animation complete. Ready for cleanup.");
}

void Boss::OnCastingAnimComplete() 
{
    if (m_currentState == BossState::CASTING) 
    {
        LogManager::GetInstance().Log("Boss Casting Animation Complete. Activating spell effect.");
        IsCastingSpell();
        TransitionToState(BossState::IDLE);
    }
}

bool Boss::IsAttacking() const
{
    return m_currentState == BossState::ATTACKING_WINDUP ||
        m_currentState == BossState::ATTACKING_STRIKE ||
        m_currentState == BossState::ATTACKING_OVER;
}

bool Boss::IsCastingSpell() const
{
    return m_currentState == BossState::CASTING ||
        m_currentState == BossState::SPELL_WINDUP ||
        m_currentState == BossState::SPELL_STRIKE ||
        m_currentState == BossState::SPELL_OVER;
}

void Boss::DebugDraw()
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

Vector2& Boss::GetPosition()
{
    return m_position;
}