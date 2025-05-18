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
#include "GameEndPrompt.h"
#include "EnemySpawner.h"
#include "PlayerHUD.h"
#include "CollisionSystem.h"

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

	void AddEnemy(EnemyBat* bat);
	void AddEnemy(EnemyType2* type2);

	void PlayerRequestsQuit() { m_playerChoseToQuit = true; }

protected:
	void CleanupDead();

	
	void UpdateUpgradeMenuUI(InputSystem& inputSystem);
	void DrawUpgradeMenu(Renderer& renderer);
	void DrawEndGamePrompts(Renderer& renderer);
	void UpdateGameEndPromptUI(InputSystem& inputSystem);

private:
	void DrawPlayerUI(Renderer& renderer);

	// Member data
public:

protected:
	SceneAbyssWalker(const SceneAbyssWalker& scene) = delete;
	SceneAbyssWalker& operator=(const SceneAbyssWalker& scene) = delete;

	WaveSystem* m_pWaveSystem;
	Player* m_pPlayer;
	UpgradeMenu* m_pUpgradeMenu;
	GameEndPrompt* m_pGameEndPrompt;
	Sprite* m_pmoonBackground;
	Renderer* m_pRenderer;
	EnemySpawner* m_pEnemySpawner;
	PlayerHUD* m_pPlayerHUD;
	CollisionSystem* m_pCollisionSystem;

	std::vector<EnemyBat*> m_enemyBats;
	std::vector<EnemyType2*> m_enemyType2;

	bool m_bShowHitboxes; // For debugging

	bool m_playerChoseToQuit;

	// Font details for custom UI
	const char* m_uiFontPath = "assets/fonts/OptimusPrinceps.ttf";
	const int m_uiFontSize = 18;
	const int m_uiTitleFontSize = 24;

private:

};

#endif // __SCENEABYSSWALKER_H