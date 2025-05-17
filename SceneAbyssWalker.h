#ifndef __SCENEABYSSWALKER_H
#define __SCENEABYSSWALKER_H

// Local includes
#include "Scene.h"
#include "Vector2.h"

// Lib Includes
#include <vector>
#include <string>

// Forward declarations
class Player;
class EnemyBat;
class EnemyType2;
class Renderer;
class InputSystem;
class Sprite;

class SceneAbyssWalker : public Scene
{
	// Member methods
public:
	SceneAbyssWalker();
	~SceneAbyssWalker();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime, InputSystem& inputSystem);
	void Draw(Renderer& renderer);
	void DebugDraw();

	void fullBackground(Renderer& renderer);

protected:
	Player* m_pPlayer;
	void UpdateSpawning(float deltaTime);

	enum class EnemySpawnType { BAT, TYPE2 };

	void SpawnEnemy(EnemySpawnType, bool spawnOnLeft);
	void HandleCollisions();

	void CleanupDead();

private:
	SceneAbyssWalker(const SceneAbyssWalker& scene) = delete;
	SceneAbyssWalker& operator=(const SceneAbyssWalker& scene) = delete;

	// Member data
public:

protected:
	std::vector<EnemyBat*> m_enemyBats;
	std::vector<EnemyType2*> m_enemyType2;

	Renderer* m_pRenderer;

	Sprite* m_pmoonBackground;
	Sprite* m_ptree5Background;
	Sprite* m_ptree4Background;
	Sprite* m_ptree3Background;
	Sprite* m_ptree2Background;
	Sprite* m_ptree1Background;

	// Spawn logic
	const int m_maxEnemies = 20; // Max enemies limit

	// Bats
	float m_batSpawnTimer;
	const float m_batSpawnInterval = 5.0f; // spawns a mob every X secs
	const int m_maxBats = 10; // Maximum bats allowed
	const int m_maxBatsPerSide = 5; // how many enemies spawn on each side

	// Type 2
	float m_type2SpawnTimer;
	const float m_type2SpawnInterval = 10.0f;
	const int m_maxType2 = 5;
	const int m_maxType2PerSide = 3;

	// Type 3

	bool m_bShowHitboxes; // For debugging

private:

};

#endif // __SCENEABYSSWALKER_H