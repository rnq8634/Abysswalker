// Local includes
#include "Scene.h"
#include <vector>
#include <string>
#include "Vector2.h"

// Forward declarations
class Player;
class Enemy;
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
	void SpawnEnemy(bool spawnOnLeft);
	void HandleCollisions();

private:
	SceneAbyssWalker(const SceneAbyssWalker& scene) = delete;
	SceneAbyssWalker& operator=(const SceneAbyssWalker& scene) = delete;

	// Member data
public:

protected:
	std::vector<Enemy*> m_enemies;
	Renderer* m_pRenderer;

	Sprite* m_pmoonBackground;
	Sprite* m_ptree5Background;
	Sprite* m_ptree4Background;
	Sprite* m_ptree3Background;
	Sprite* m_ptree2Background;
	Sprite* m_ptree1Background;

	// Spawn logic
	float m_spawnTimer;
	const float m_spawnInterval = 7.0f; // spawns a mob every 7 secs
	const int m_maxEnemies = 8;
	const int m_maxEnemiesPerSide = 4;
	
private:

};