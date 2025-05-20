// This includes
#include "CollisionSystem.h"

// Local includes
#include "Player.h"
#include "EnemyBat.h"
#include "EnemyType2.h"
#include "AnimatedSprite.h"
#include "LogManager.h"

CollisionSystem::CollisionSystem()
{
}

CollisionSystem::~CollisionSystem()
{
}

void CollisionSystem::ProcessCollisions(Player* player, std::vector<EnemyBat*>& bats, std::vector<EnemyType2*>& type2s, Boss* boss)
{
	if (!player || !player->IsAlive()) return;

	for (EnemyBat* enemyBat : bats)
	{
		if (!enemyBat->IsAlive()) continue;

		CheckPlayerHitByEnemyBat(player, enemyBat);
		CheckEnemyBatHitByPlayerAttack(player, enemyBat);
	}

	for (EnemyType2* enemyT2 : type2s)
	{
		if (!enemyT2->IsAlive()) continue;

		CheckPlayerHitByEnemyType2(player, enemyT2);
		CheckEnemyType2HitByPlayerAttack(player, enemyT2);
	}

    if (boss && boss->IsAlive())
    {
        CheckPlayerHitByBoss(player, boss);
        CheckBossHitByPlayerAttack(player, boss);
    }
}

// Bat
void CollisionSystem::CheckPlayerHitByEnemyBat(Player* player, EnemyBat* enemy)
{
    if (player->GetCurrentState() != PlayerState::ROLLING &&
        player->GetCurrentState() != PlayerState::HURT &&
        !player->IsInvincible() &&
        enemy->IsAttacking() &&
        player->CheckCollision(*enemy))
    {
    }
}

// Type 2
void CollisionSystem::CheckPlayerHitByEnemyType2(Player* player, EnemyType2* enemy)
{
    if (player->GetCurrentState() != PlayerState::ROLLING &&
        player->GetCurrentState() != PlayerState::HURT &&
        !player->IsInvincible() &&
        enemy->IsAttacking() &&
        player->CheckCollision(*enemy))
    {
    }
}

// Boss
void CollisionSystem::CheckPlayerHitByBoss(Player* player, Boss* boss)
{
    if (player->GetCurrentState() != PlayerState::ROLLING &&
        player->GetCurrentState() != PlayerState::HURT &&
        !player->IsInvincible() &&
        boss->IsAttacking() &&
        player->CheckCollision(*boss))
    {
    }
}

// --- Enemies taking damage from Player's attack ---
// Bat damaged by Player
void CollisionSystem::CheckEnemyBatHitByPlayerAttack(Player* player, EnemyBat* enemy)
{
    if (player->GetCurrentState() == PlayerState::ATTACKING)
    {
        AnimatedSprite* playerSprite = player->GetCurrentAnimatedSprite();
        if (playerSprite)
        {
            // Using frame numbers from your original code
            int currentFrame = playerSprite->GetCurrentFrame();
            bool isHitFrame = (currentFrame >= 2 && currentFrame <= 5);

            if (isHitFrame)
            {
                Vector2 playerPos = player->GetPosition();
                float pAttackMinX, pAttackMaxX, pAttackMinY, pAttackMaxY;

                // Replicate player attack hitbox logic from SceneAbyssWalker
                if (player->IsFacingRight()) 
                {
                    pAttackMinX = playerPos.x;
                    pAttackMaxX = playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                }
                else 
                {
                    pAttackMinX = playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                    pAttackMaxX = playerPos.x;
                }
                pAttackMinY = playerPos.y - (Player::PLAYER_SPRITE_HEIGHT / 2.0f);
                pAttackMaxY = playerPos.y + (Player::PLAYER_SPRITE_HEIGHT / 2.0f);

                Vector2 enemyPos = enemy->GetPosition();
                float enemyRadius = enemy->GetRadius();
                float eMinX = enemyPos.x - enemyRadius;
                float eMaxX = enemyPos.x + enemyRadius;
                float eMinY = enemyPos.y - enemyRadius;
                float eMaxY = enemyPos.y + enemyRadius;

                bool overlapX = pAttackMinX < eMaxX && pAttackMaxX > eMinX;
                bool overlapY = pAttackMinY < eMaxY && pAttackMaxY > eMinY;

                if (overlapX && overlapY)
                {
                    if (player->DamageDoneToTarget(enemy)) // Player tracks who it hit this swing
                    {
                        enemy->TakeDamage(player->GetAttackDamage());
                        // SoundSystem::GetInstance().PlaySound("ENEMY_BAT_HURT");
                    }
                }
            }
        }
    }
}

// Type 2 damaged by Player
void CollisionSystem::CheckEnemyType2HitByPlayerAttack(Player* player, EnemyType2* enemy)
{
    if (player->GetCurrentState() == PlayerState::ATTACKING)
    {
        AnimatedSprite* playerSprite = player->GetCurrentAnimatedSprite();
        if (playerSprite)
        {
            int currentFrame = playerSprite->GetCurrentFrame();
            bool isHitFrame = (currentFrame >= 2 && currentFrame <= 5);

            if (isHitFrame)
            {
                Vector2 playerPos = player->GetPosition();
                float pAttackMinX, pAttackMaxX, pAttackMinY, pAttackMaxY;

                if (player->IsFacingRight()) {
                    pAttackMinX = playerPos.x;
                    pAttackMaxX = playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                }
                else {
                    pAttackMinX = playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                    pAttackMaxX = playerPos.x;
                }
                pAttackMinY = playerPos.y - (Player::PLAYER_SPRITE_HEIGHT / 2.0f);
                pAttackMaxY = playerPos.y + (Player::PLAYER_SPRITE_HEIGHT / 2.0f);

                Vector2 enemyPos = enemy->GetPosition();
                float enemyRadius = enemy->GetRadius();
                float eMinX = enemyPos.x - enemyRadius;
                float eMaxX = enemyPos.x + enemyRadius;
                float eMinY = enemyPos.y - enemyRadius;
                float eMaxY = enemyPos.y + enemyRadius;

                bool overlapX = pAttackMinX < eMaxX && pAttackMaxX > eMinX;
                bool overlapY = pAttackMinY < eMaxY && pAttackMaxY > eMinY;

                if (overlapX && overlapY)
                {
                    if (player->DamageDoneToTarget(enemy))
                    {
                        enemy->TakeDamage(player->GetAttackDamage());
                        // SoundSystem::GetInstance().PlaySound("ENEMY_TYPE2_HURT");
                    }
                }
            }
        }
    }
}

// Boss hit by Player attack
void CollisionSystem::CheckBossHitByPlayerAttack(Player* player, Boss* boss)
{
    if (!boss || !boss->IsAlive()) return; // Add a check for boss validity

    if (player->GetCurrentState() == PlayerState::ATTACKING)
    {
        AnimatedSprite* playerSprite = player->GetCurrentAnimatedSprite();
        if (playerSprite)
        {
            int currentFrame = playerSprite->GetCurrentFrame();
            bool isHitFrame = (currentFrame >= 2 && currentFrame <= 5);

            if (isHitFrame)
            {
                Vector2 playerPos = player->GetPosition();
                float pAttackMinX, pAttackMaxX, pAttackMinY, pAttackMaxY;

                if (player->IsFacingRight()) {
                    pAttackMinX = playerPos.x;
                    pAttackMaxX = playerPos.x + (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                }
                else {
                    pAttackMinX = playerPos.x - (Player::PLAYER_SPRITE_WIDTH / 2.0f + PLAYER_ATTACK_REACH);
                    pAttackMaxX = playerPos.x;
                }
                pAttackMinY = playerPos.y - (Player::PLAYER_SPRITE_HEIGHT / 2.0f);
                pAttackMaxY = playerPos.y + (Player::PLAYER_SPRITE_HEIGHT / 2.0f);

                Vector2 enemyPos = boss->GetPosition(); // Use boss's position
                float enemyRadius = boss->GetRadius();  // Use boss's (newly adjusted) radius
                float eMinX = enemyPos.x - enemyRadius;
                float eMaxX = enemyPos.x + enemyRadius;
                float eMinY = enemyPos.y - enemyRadius;
                float eMaxY = enemyPos.y + enemyRadius;

                bool overlapX = pAttackMinX < eMaxX && pAttackMaxX > eMinX;
                bool overlapY = pAttackMinY < eMaxY && pAttackMaxY > eMinY;

                if (overlapX && overlapY)
                {
                    if (player->DamageDoneToTarget(boss)) // Pass boss as Entity*
                    {
                        boss->TakeDamage(player->GetAttackDamage());
                        LogManager::GetInstance().Log("Player hit the Boss!"); // For debugging
                    }
                }
            }
        }
    }
}