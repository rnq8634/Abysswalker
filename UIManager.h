// UIManager.h
#ifndef __UIMANAGER_H__
#define __UIMANAGER_H__

#include <string>

// Forward Declarations
class Renderer;
class Player;
class Sprite;
class AnimatedSprite;
// class Texture; // Not directly needed in header anymore

enum class HealthBarAnimState
{
	IDLE_STATIC,
	DECREASING,
	INCREASING,
	REVIVING_FULL
};

class UIManager
{
public:
	// Member method
	UIManager();
	~UIManager();

	bool Initialise(Renderer& renderer, Player* player);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);

private:
	UIManager(const UIManager&);
	UIManager& operator=(const UIManager&);

	// health bar specific anims
	AnimatedSprite* m_pHealthBarDamageAnim; // this will take damage
	AnimatedSprite* m_pHealthBarHealAnim; // tjos will show heals
	HealthBarAnimState m_healthAnimState;

	bool LoadAndPositionUIElement(Renderer& renderer, Sprite*& frameSprite, Sprite*& fillSprite,
		const char* frameAsset, const char* fillAsset,
		int frameX, int frameY, // Position for the frame sprite
		int& originalFillWidth, int& originalFillHeight, // Out params
		int fillOffsetX, int fillOffsetY); // Visual offset of fill from frame's top-left


	void UpdateHealthVisuals(); // Removed parameter, will use member m_displayHealthPercent
	void UpdateStaminaVisuals(); // Removed parameter

private:
	// --- Member Data ---
	Player* m_pPlayer;
	int m_screenWidth;
	int m_screenHeight;

	int m_targetHealthFrame;
	int m_lastPlayerHealth;

	// --- Health Bar Elements ---
	Sprite* m_pHealthBarFrame;
	Sprite* m_pHealthBarFill;
	int m_healthFillOriginalWidth;
	int m_healthFillOriginalHeight;
	int m_healthFillInitialX; // Store initial X pos of fill bar (its center)

	float m_targetHealthPercent;
	float m_displayHealthPercent;

	// --- Stamina Bar Elements ---
	Sprite* m_pStaminaBarFrame;
	Sprite* m_pStaminaBarFill;
	int m_staminaFillOriginalWidth;
	int m_staminaFillOriginalHeight;
	int m_staminaFillInitialX; // Store initial X pos of fill bar (its center)

	float m_targetStaminaPercent;
	float m_displayStaminaPercent;


	// Constants for positioning/layout
	const int BAR_Y_OFFSET = 25;
	const int HEALTH_X_OFFSET = 0;
	const int STAMINA_X_OFFSET = 25; // Offset from right screen edge
	const float BAR_CHANGE_SPEED = 1.5f; // Speed for visual bar changes (health/stamina)
	const int FILL_BAR_OFFSET_X = 2; // How many pixels fill is inset from left of frame
	const int FILL_BAR_OFFSET_Y = 2; // How many pixels fill is inset from top of frame
};

#endif // __UIMANAGER_H__