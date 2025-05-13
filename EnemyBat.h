#ifndef __ENEMYBAT_H
#define __ENEMYBAT_H

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

enum class EnemyBatState
{
    IDLE,
    WALKING,
    ATTACKING,
    HURT,
    DEATH
};

class EnemyBat : public Entity
{
    using AnimationCallBack = std::function<void()>;

    // Member methods
public:
    EnemyBat();
    virtual ~EnemyBat();
 
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
        EnemyBatState state,
        const char* pcFilename,
        int frameWidth,
        int frameHeight,
        float frameDuration,
        bool loop,
        AnimationCallBack onComplete = nullptr
    );
    void TransitionToState(EnemyBatState newState);
    void UpdateSprite(AnimatedSprite* sprite, float deltaTime);

    // AI and Combat
    virtual void UpdateAI(float deltaTime); // Made virtual for different enemy types

private:

    // member data
public:
    // Needs to stay in public
    Player* m_pTargetPlayer;

    const float kGroundLevel = 850.0f;
    // Sprite dimensions for reference or default radius, actual values depend on enemy type
    static const int ENEMY_DEFAULT_SPRITE_WIDTH = 64;
    static const int ENEMY_DEFAULT_SPRITE_HEIGHT = 64;

protected:
    EnemyBatState m_currentState;
    bool m_bFacingRight; // True if facing right, false if facing left

    // Enemy stats
    int m_iDamage;
    float m_moveSpeed;
    float m_attackRange;
    float m_detectionRange;
    float m_attackCD;
    float m_timeSinceAttack;
    float m_attackWindUpTime;
    float m_currentAttackTime;
    bool m_bHasDealtDMG;

    // SPrites
    std::map<EnemyBatState, AnimatedSprite*> m_animatedSprites;
    Sprite* m_pStaticEnemy;

private:
    EnemyBat(const EnemyBat& enemybat) = delete;
    EnemyBat& operator=(const EnemyBat& enemybat) = delete;
};

#endif // __ENEMYBAT_H