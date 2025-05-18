#ifndef __SCENEABYSSWALKER_H
#define __SCENEABYSSWALKER_H

// Local includes
#include "Scene.h"
#include "Vector2.h"
#include "PlayerStats.h"
#include "fmod.hpp"
#include "Player.h"

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

enum class WaveState
{
	PRE_WAVE_DELAY,
	IN_WAVE,
	INTERMISSION,
	GAME_WON,
	GAME_END_PROMPT
};

struct UIElemRect
{
	float x, y, width, height;
};

struct UIButton 
{
	UIElemRect rect;
	StatType statToUpgrade;
	bool isHovered;
	std::string identifier;

	Sprite* textSprite;
	Texture* textTexture;

	UIButton() : textSprite(nullptr), textTexture(nullptr), isHovered(false)
	{}

	bool IsMouseOver(float mouseX, float mouseY) const 
	{
		return mouseX >= rect.x && mouseX <= rect.x + rect.width &&
			   mouseY >= rect.y && mouseY <= rect.y + rect.height;
	}
};

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

protected:
	Player* m_pPlayer;
	void UpdateSpawning(float deltaTime);

	enum class EnemySpawnType { BAT, TYPE2 };

	void SpawnEnemy(EnemySpawnType, bool spawnOnLeft);
	void HandleCollisions();
	void CleanupDead();

	void ProcessWave(float deltaTime, InputSystem& inputSystem);
	void StartNewWave();
	void EndWave();
	void StartIntermission();

	void SetupUpgradeMenuUI();
	void UpdateUpgradeMenuUI(InputSystem& inputSystem);

	void DrawUpgradeMenu(Renderer& renderer);
	void ClearUpgradeMenuUI();
	void DrawEndGamePrompts(Renderer& renderer);

	void SetupGameEndPromptUI(const std::string& titleMessage);
	void UpdateGameEndPromptUI(InputSystem& inputSystem);
	void ClearGameEndPromptUI();

private:
	void UpdateButtonListUI(std::vector<UIButton>& buttons, int& selectedButtonIndex, InputSystem& inputSystem, bool& actionTriggered, std::string& actionIdentifier);
	void ActivateButtonAction(const std::string& identifier);
	void ActivateGameEndButtonAction(const std::string& identifier);

	void DrawPlayerUI(Renderer& renderer);

	// Member data
public:

protected:
	SceneAbyssWalker(const SceneAbyssWalker& scene) = delete;
	SceneAbyssWalker& operator=(const SceneAbyssWalker& scene) = delete;

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

	// Type 3

	bool m_bShowHitboxes; // For debugging

	// Wave System
	WaveState m_currentWaveState;
	int m_currentWaveNumber;
	float m_waveTimer;
	float m_intermissionTimer;
	int m_enemiesKilledThisWave;

	const float PRE_WAVE_DELAY_DURATION = 3.0f; // Seconds before wave starts
	const float WAVE_DURATION = 60.0f;          // 1 minute per wave
	const float INTERMISSION_DURATION = 30.0f;  // 30 seconds for upgrades
	const int KILLS_TO_END_WAVE_EARLY = 10; // Base is 50 but lowered for debugging purposes
	const int MAX_WAVES = 10;

	bool m_playerChoseToQuit;

	// Custom UI Elements for Upgrade Menu
	std::vector<UIButton> m_upgradeButtons;
	Sprite* m_pUpgradeMenuTitleSprite; // For "Upgrade Stats"
	Texture* m_pUpgradeMenuTitleTexture;
	Sprite* m_pEssenceTextSprite;
	Texture* m_pEssenceTextTexture;

	// Custom UI Elements for Game End Prompts
	std::vector<UIButton> m_gameEndButtons;
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