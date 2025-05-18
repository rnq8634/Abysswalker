#ifndef __COLLISIONSYSTEM_H__
#define __COLLISIONSYSTEM_H__

// Lib includes
#include <vector>

// Forward Declarations
class Player;
class EnemyBat;
class EnemyType2;
class AnimatedSprite;
class Vector2;

class CollisionSystem
{
public:
	CollisionSystem();
	~CollisionSystem();

	void ProcessCollisions(Player* player, std::vector<EnemyBat*>& bats, std::vector<EnemyType2*>& type2s);

private:
	void CheckPlayerHitByEnemy(Player* player, EnemyBat* enemy);
	void CheckPlayerHitByEnemy(Player* player, EnemyType2* enemy);

	void CheckEnemyHitByPlayerAttack(Player* player, EnemyBat* enemy);
	void CheckEnemyHitByPlayerAttack(Player* player, EnemyType2* enemy);

	const float PLAYER_ATTACK_REACH = 25.0f;
};

#endif // !__COLLISIONSYSTEM_H__

