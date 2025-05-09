#ifndef __ENEMY_H
#define __ENEMY_H

// Includes
#include "Entity.h"
#include "AnimatedSprite.h" 

// Lib inclduesS
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

    // Member methods
public:
    Enemy();
    virtual ~Enemy();
 
    virtual bool Initialise(Renderer& renderer, const Vector2& startPosition); // Specific Enemy Initialise
    void Process(float deltaTime);
    void Draw(Renderer& renderer);
    void DebugDraw();

    Vector2& GetPosition();

    void TakeDamage(int amount);

    bool IsAttacking() const;
    AnimatedSprite* GetCurrentAnimatedSprite(); // For Scene to check death animation complete

protected:
    void OnHurtAnimationComplete();
    void OnDeathAnimationComplete();
    void OnAttackAnimationComplete();

    void MoveToPlayer(float deltaTime);

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
    void UpdateSprite(AnimatedSprite* sprite, float deltaTime);

    // AI and Combat
    virtual void UpdateAI(float deltaTime); // Made virtual for different enemy types

private:

    // member data
public:
    Player* m_pTargetPlayer;

    const float kGroundLevel = 1200.0f;
    // Sprite dimensions for reference or default radius, actual values depend on enemy type
    static const int ENEMY_DEFAULT_SPRITE_WIDTH = 64;
    static const int ENEMY_DEFAULT_SPRITE_HEIGHT = 64;

protected:
    EnemyState m_currentState;
    std::map<EnemyState, AnimatedSprite*> m_animatedSprites;

    Sprite* m_pStaticEnemy;

    bool m_bFacingRight; // True if facing right, false if facing left
    int m_damage;
    float m_moveSpeed;
    float m_attackRange;
    float m_detectionRange;
    float m_attackCooldown; // Time between enemy attack
    float m_timeSinceLastAttack; // Time for attack CD
    float m_attackWindUpTime; // Time from starting attacking state to deal damage
    float m_currentAttackTime; // Time for windup on attack

private:
    Enemy(const Enemy& enemy) = delete;
    Enemy& operator=(const Enemy& enemy) = delete;
};

#endif // __ENEMY_H