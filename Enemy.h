#ifndef __ENEMY_H
#define __ENEMY_H

#include "Entity.h"
#include "AnimatedSprite.h" 
#include <map>
#include <string>
#include <functional> 

// Forward declaration
class Player;
class Renderer;

enum class EnemyState
{
    IDLE,
    WALKING,
    ATTACKING,
    HURT,
    DEATH
};

class Enemy : public Entity
{
    using AnimationCallBack = std::function<void()>;

public:
    Enemy();
    ~Enemy() override;

    bool Initialise(Renderer& renderer) override; // Default Entity::Initialise signature
    virtual bool Initialise(Renderer& renderer, Player* targetPlayer, const Vector2& startPosition); // Specific Enemy Initialise

    void Process(float deltaTime) override;
    void Draw(Renderer& renderer) override;
    void DebugDraw() override;

    void TakeDamage(int amount) override;

    bool IsAttacking() const;
    AnimatedSprite* GetCurrentAnimatedSprite(); // For Scene to check death animation complete

public: // Make ground level accessible for spawner or other systems
    const float kGroundLevel = 580.0f;
    // Sprite dimensions for reference or default radius, actual values depend on enemy type
    static const int ENEMY_DEFAULT_SPRITE_WIDTH = 64;
    static const int ENEMY_DEFAULT_SPRITE_HEIGHT = 64;


protected:
    void OnHurtAnimationComplete();
    void OnDeathAnimationComplete();
    void OnAttackAnimationComplete();


    bool InitialiseAnimatedSprite(
        Renderer& renderer,
        EnemyState state,
        const char* pcFilename,
        int frameWidth,
        int frameHeight,
        float frameDuration,
        bool loop,
        AnimationCallBack onComplete = nullptr
    );
    void TransitionToState(EnemyState newState);
    // AnimatedSprite* GetCurrentAnimatedSprite(); // Moved to public
    void UpdateSprite(AnimatedSprite* sprite, float deltaTime);

    // AI and Combat
    virtual void UpdateAI(float deltaTime); // Made virtual for different enemy types

protected:
    EnemyState m_currentState;
    std::map<EnemyState, AnimatedSprite*> m_animatedSprites;
    Player* m_pTargetPlayer;

    bool m_bFacingRight; // True if facing right, false if facing left
    int m_damage;
    float m_moveSpeed;
    float m_attackRange;
    float m_detectionRange;
    float m_attackCooldown;       // Time between attacks
    float m_timeSinceLastAttack;  // Timer for attack cooldown
    float m_attackWindUpTime;     // Time from starting attack state to dealing damage
    float m_currentAttackTime;    // Timer for wind-up

private:
    Enemy(const Enemy& enemy) = delete;
    Enemy& operator=(const Enemy& enemy) = delete;
};

#endif // __ENEMY_H