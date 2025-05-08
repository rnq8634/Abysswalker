// Local includes
#include "Scene.h"
#include <vector>
#include <string>

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
	~SceneAbyssWalker() override;

	bool Initialise(Renderer& renderer) override;
	void Process(float deltaTime, InputSystem& inputSystem) override;
	void Draw(Renderer& renderer) override;
	void DebugDraw() override;

	void fullBackground(Renderer& renderer);

protected:
	void UpdateSpawning(float deltaTime);
	void SpawnEnemy(bool spawnOnLeft);
	void HandleCollisions();

private:
	SceneAbyssWalker(const SceneAbyssWalker& scene);
	SceneAbyssWalker& operator=(const SceneAbyssWalker& scene);

	// Member data
public:

protected:
	Player* m_pPlayer;
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