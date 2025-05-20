#ifndef __BOSS_H__
#define __BOSS_H__

// Local Includes
#include "Entity.h"
#include "AnimatedSprite.h" 

// Lib inclduesS
#include <map>
#include <string>
#include <functional> 

// Forward declaration
class Player;
class Renderer;
class SceneAbyssWalker;
class Texture;

enum class BossState
{
    IDLE,
    WALKING,
    ATTACKING_WINDUP, // 70 x 93
    ATTACKING_STRIKE, // 140 x 93
    ATTACKING_OVER, // 70 x 93
    CASTING, // 70 x 93
    SPELL_WINDUP, // 40 x 30
    SPELL_STRIKE, // 40 x 30
    SPELL_OVER, // 40 x 30
    HURT,
    DEATH
};

class Boss : public Entity
{
    using AnimationCallBack = std::function<void()>;

    // Member methods
public:
    Boss();
    virtual ~Boss();

    virtual bool Initialise(Renderer& renderer, const Vector2& startPosition); // Specific Enemy Initialise
    void Process(float deltaTime);
    void Draw(Renderer& renderer);
    void DebugDraw();

    void SetSceneReference(SceneAbyssWalker* scene);
    Vector2& GetPosition();

    void TakeDamage(int amount);
    bool IsAttacking() const;
    bool IsCastingSpell() const;
    AnimatedSprite* GetCurrentAnimatedSprite(); // For Scene to check death animation complete

    void TransitionToState(BossState newState);
    void UpdateSprite(AnimatedSprite* sprite, float deltaTime);
    BossState GetCurrentState() const { return m_currentState; }

protected:
    void OnHurtAnimationComplete();
    void OnDeathAnimationComplete();
    void OnAttackSequenceComplete();

    // Spell animation
    void OnCastingAnimComplete();

    void MoveToPlayer(float deltaTime);

    bool InitialiseAnimatedSprite(
        Renderer& renderer,
        BossState state,
        const char* pcFilename,
        int frameWidth,
        int frameHeight,
        float frameDuration,
        bool loop,
        AnimationCallBack onComplete = nullptr
    );

    virtual void UpdateAI(float deltaTime);

    float m_baseRadius;
    float m_strikePhaseRadius;

private:

    // Member data
public:
    // Needs to stay in public
    Player* m_pTargetPlayer;
    static const float kGroundLevel;

    // Sprite dimensions for EnemyType2 (ALWAYS DOUBLE CHECK)
    static const int BOSS_DEFAULT_SPRITE_HEIGHT = 93;
    static const int BOSS_DEFAULT_SPRITE_WIDTH = 70;

    // Attack
    static const int BOSS_DEFAULT_SPRITE_ATTACKWINDUP_WIDTH = 70;
    static const int BOSS_DEFAULT_SPRITE_ATTACKSTRIKE_WIDTH = 140;
    static const int BOSS_DEFAULT_SPRITE_ATTACK_END_WIDTH = 70;

    // Cast Animation to Cast the Spell (From Boss)
    static const int BOSS_DEFAULT_SPRITE_CAST_WIDTH = 70; // needs to be added

    // Cast Windup (Spell)
    static const int BOSS_DEFAULT_SPRITE_CASTWINDUP_WIDTH = 40;
    static const int BOSS_DEFAULT_SPRITE_CASTWINDUP_HEIGHT = 30;

    // Cast Strike
    static const int BOSS_DEFAULT_SPRITE_CASTSTRIKE_WIDTH = 40;
    static const int BOSS_DEFAULT_SPRITE_CASTSTRIKE_HEIGHT = 60;

    // Cast End
    static const int BOSS_DEFAULT_SPRITE_CAST_END_WIDTH = 40;
    static const int BOSS_DEFAULT_SPRITE_CAST_END_HEIGHT = 30;

    static const float BOSS_VISUAL_SCALE;
    static const float SPELL_EFFECT_VISUAL_SCALE;

    static const float BOSS_SPELL_WINDUP_DURATION;
    static const float BOSS_SPELL_STRIKE_DURATION;
    static const float BOSS_SPELL_OVER_DURATION;

protected:
    Renderer* m_pRenderer;
    BossState m_currentState;
    bool m_bFacingRight; // True if facing right, false if facing left

    // Boss Attack stats
    int m_iDamage;
    float m_moveSpeed;
    float m_attackRange;
    float m_detectionRange;
    float m_attackCD;
    float m_timeSinceAttack;

    // Boss Spell Stats
    float m_spellCastRange;
    float m_spellAttackCD;
    float m_timeSinceSpellAttack;
    int m_spellDamage;

    // Textures & Sprites for spell
    AnimatedSprite* m_pSpellEffectSprite;       
    Texture* m_pSpellEffectTexture_Windup;
    Texture* m_pSpellEffectTexture_Strike;
    Texture* m_pSpellEffectTexture_Over;

    // Where the spell will try and target
    Vector2 m_spellTargetPosition;

    float m_spellPhaseTimer;           
    bool m_bSpellDamageDealtThisCast;

    SceneAbyssWalker* m_pSceneRef;

    // Essence drops
    int m_minEssenceDrop;
    int m_maxEssenceDrop;
    float m_currentPhaseTimer;

    bool m_bHasDealtDMG;

    // SPrites
    std::map<BossState, AnimatedSprite*> m_animatedSprites;
    Sprite* m_pStaticEnemy;

private:
    Boss(const Boss& boss) = delete;
    Boss& operator=(const Boss& boss) = delete;
};

#endif // __BOSS_H__
