#ifndef __COLLISIONSYSTEM_H__
#define __COLLISIONSYSTEM_H__

// Local include
#include "Boss.h"

// Lib includes
#include <vector>

// Forward Declarations
class Player;
class EnemyBat;
class EnemyType2;
class AnimatedSprite;
class Boss;
class Vector2;

class CollisionSystem
{
public:
	CollisionSystem();
	~CollisionSystem();

	void ProcessCollisions(Player* player, std::vector<EnemyBat*>& bats, std::vector<EnemyType2*>& type2s, Boss* boss);

private:
	void CheckPlayerHitByEnemyBat(Player* player, EnemyBat* enemy);
	void CheckPlayerHitByEnemyType2(Player* player, EnemyType2* enemy);
	void CheckPlayerHitByBoss(Player* player, Boss* boss);

	void CheckEnemyBatHitByPlayerAttack(Player* player, EnemyBat* enemy);
	void CheckEnemyType2HitByPlayerAttack(Player* player, EnemyType2* enemy);
	void CheckBossHitByPlayerAttack(Player* player, Boss* boss);

	const float PLAYER_ATTACK_REACH = 25.0f;
};

#endif // !__COLLISIONSYSTEM_H__

