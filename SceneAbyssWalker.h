#ifndef __SCENEABYSSWALKER_H
#define __SCENEABYSSWALKER_H

// Local includes
#include "Scene.h"
#include "Vector2.h"
#include "PlayerStats.h"
#include "fmod.hpp"
#include "Player.h"
#include "WaveSystem.h"
#include "UpgradeMenu.h"

// Lib Includes
#include <vector>
#include <string>
#include <map>

// Forward declarations
class Player;
class EnemyBat;
class EnemyType2;
class Renderer;
class InputSystem;
class Sprite;
class Texture;
class SoundSystem;

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

	void NotifyEnemyKilled();
	void RestartGame();

	void fullBackground(Renderer& renderer);

	void EndWaveEnemyCleanup();

	WaveSystem* GetWaveSystem() const { return m_pWaveSystem; }

	void SetupUpgradeMenuUI();
	void ClearUpgradeMenuUI();
	void SetupGameEndPromptUI(const std::string& titleMessage);

	void ClearGameEndPromptUI();

	std::vector<UIButton> m_gameEndButtons;

protected:
	void UpdateSpawning(float deltaTime);

	enum class EnemySpawnType { BAT, TYPE2 };

	void SpawnEnemy(EnemySpawnType, bool spawnOnLeft);
	void HandleCollisions();
	void CleanupDead();

	
	void UpdateUpgradeMenuUI(InputSystem& inputSystem);
	void DrawUpgradeMenu(Renderer& renderer);
	void DrawEndGamePrompts(Renderer& renderer);
	void UpdateGameEndPromptUI(InputSystem& inputSystem);

private:
	void ActivateGameEndButtonAction(const std::string& identifier);

	void DrawPlayerUI(Renderer& renderer);

	// Member data
public:

protected:
	SceneAbyssWalker(const SceneAbyssWalker& scene) = delete;
	SceneAbyssWalker& operator=(const SceneAbyssWalker& scene) = delete;

	WaveSystem* m_pWaveSystem;
	Player* m_pPlayer;
	UpgradeMenu* m_pUpgradeMenu;

	std::vector<EnemyBat*> m_enemyBats;
	std::vector<EnemyType2*> m_enemyType2;
	Renderer* m_pRenderer;

	int m_selectedUpgradeButtonIndex;
	int m_selectedGameEndButtonIndex;

	Sprite* m_pmoonBackground;

	// Spawn logic
	const int m_maxEnemies = 20; // Max enemies limit

	// Bats
	float m_batSpawnTimer;
	const float m_batSpawnInterval = 3.5f; // spawns a mob every X secs
	const int m_maxBats = 10; // Maximum bats allowed

	// Type 2
	float m_type2SpawnTimer;
	const float m_type2SpawnInterval = 7.0f;
	const int m_maxType2 = 5;

	bool m_bShowHitboxes; // For debugging

	bool m_playerChoseToQuit;

	// Custom UI Elements for Game End Prompts
	Sprite* m_pGameEndTitleSprite;
	Texture* m_pGameEndTitleTexture;
	Sprite* m_pGameEndReviveCostSprite; // Optional, if showing revive cost
	Texture* m_pGameEndReviveCostTexture;

	// Font details for custom UI
	const char* m_uiFontPath = "assets/fonts/OptimusPrinceps.ttf";
	const int m_uiFontSize = 18;
	const int m_uiTitleFontSize = 24;

private:

};

#endif // __SCENEABYSSWALKER_H