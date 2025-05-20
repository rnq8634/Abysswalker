#ifndef __SCENEABYSSWALKER_H
#define __SCENEABYSSWALKER_H

// Local includes
#include "Scene.h"
#include "Vector2.h"
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

// Forward declarations
class Player;
class EnemyBat;
class EnemyType2;
class Renderer;
class InputSystem;
class Sprite;
class Texture;
class Boss;

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
	Player* GetPlayer() const { return m_pPlayer; }

	void SetupUpgradeMenuUI();
	void ClearUpgradeMenuUI();
	void SetupGameEndPromptUI(const std::string& titleMessage);

	void ClearGameEndPromptUI();

	void AddEnemy(EnemyBat* bat);
	void AddEnemy(EnemyType2* type2);
	void SpawnBoss();

	void PlayerRequestsQuit() { m_playerChoseToQuit = true; }

	Boss* GetBoss() const { return m_pBoss; }

	void DebugSkipToLastWave();

protected:
	void CleanupDead();

	// UI Tingz
	void UpdateUpgradeMenuUI(InputSystem& inputSystem);
	void DrawUpgradeMenu(Renderer& renderer);
	void DrawEndGamePrompts(Renderer& renderer);
	void UpdateGameEndPromptUI(InputSystem& inputSystem);

	// Sound Tingz
	void StartBGM();
	void ProcessBGMTransition();

private:
	// Wave Info Display
	Sprite* m_pWaveTimerTextSprite;
	Texture* m_pWaveTimerTextTexture;
	std::string m_lastWaveTimerStr;

	Sprite* m_pWaveCountTextSprite;
	Texture* m_pWaveCountTextTexture;
	std::string m_lastWaveCountStr;

	FMOD::Channel* m_pCurrentBGMChannel;
	enum class CurrentPlayingBGM { NONE, BGM1, BGM2 };
	CurrentPlayingBGM m_currentBGMState;

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
	Boss* m_pBoss;

	std::vector<EnemyBat*> m_enemyBats;
	std::vector<EnemyType2*> m_enemyType2;

	bool m_playerChoseToQuit;
	bool m_bInitialised;
	bool m_bBossHasSpawned;

	// Font details for custom UI
	const char* m_uiFontPath = "assets/fonts/OptimusPrinceps.ttf";
	const int m_uiFontSize = 18;
	const int m_uiTitleFontSize = 24;

private:

};

#endif // __SCENEABYSSWALKER_H