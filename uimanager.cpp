// UIManager.cpp
#include "UIManager.h"

#include "Renderer.h"
#include "Player.h"
#include "Sprite.h"
// #include "AnimatedSprite.h" // If used later
#include "LogManager.h"
// #include "Texture.h" // Not directly needed anymore
// #include "TextureManager.h" // Not directly needed anymore

#include <algorithm>
#include <stdexcept>

UIManager::UIManager()
	: m_pPlayer(nullptr)
	, m_screenWidth(0)
	, m_screenHeight(0)
	, m_pHealthBarFrame(nullptr)
	, m_pHealthBarFill(nullptr)
	, m_healthFillOriginalWidth(0)
	, m_healthFillOriginalHeight(0)
	, m_healthFillInitialX(0)
	, m_targetHealthPercent(1.0f)
	, m_displayHealthPercent(1.0f)
	, m_pStaminaBarFrame(nullptr)
	, m_pStaminaBarFill(nullptr)
	, m_staminaFillOriginalWidth(0)
	, m_staminaFillOriginalHeight(0)
	, m_staminaFillInitialX(0)
	, m_targetStaminaPercent(1.0f)
	, m_displayStaminaPercent(1.0f)
{
}

UIManager::~UIManager()
{
	delete m_pHealthBarFrame; m_pHealthBarFrame = nullptr;
	delete m_pHealthBarFill;  m_pHealthBarFill = nullptr;
	delete m_pStaminaBarFrame; m_pStaminaBarFrame = nullptr;
	delete m_pStaminaBarFill;  m_pStaminaBarFill = nullptr;
	m_pPlayer = nullptr;
}

bool UIManager::LoadAndPositionUIElement(Renderer& renderer, Sprite*& frameSprite, Sprite*& fillSprite,
	const char* frameAsset, const char* fillAsset,
	int frameX, int frameY,
	int& originalFillWidth, int& originalFillHeight,
	int fillVisualOffsetX, int fillVisualOffsetY)
{
	try {
		// --- Load Frame ---
		frameSprite = renderer.CreateSprite(frameAsset);
		if (!frameSprite) {
			throw std::runtime_error("Failed to create frame sprite: " + std::string(frameAsset));
		}
		frameSprite->SetX(frameX); // Sets center of the frame sprite
		frameSprite->SetY(frameY); // Sets center of the frame sprite

		// --- Load Fill ---
		fillSprite = renderer.CreateSprite(fillAsset);
		if (!fillSprite) {
			throw std::runtime_error("Failed to create fill sprite: " + std::string(fillAsset));
		}
		originalFillWidth = fillSprite->GetOriginalWidth();
		originalFillHeight = fillSprite->GetOriginalHeight();

		// Position fill sprite. Its X,Y is its center.
		// We want the fill's top-left to be at (frame_top_left_X + fillVisualOffsetX, frame_top_left_Y + fillVisualOffsetY)
		int frameOriginalWidth = frameSprite->GetOriginalWidth();
		int frameOriginalHeight = frameSprite->GetOriginalHeight();

		int frameTopLeftX = frameX - frameOriginalWidth / 2;
		int frameTopLeftY = frameY - frameOriginalHeight / 2;

		int fillDesiredTopLeftX = frameTopLeftX + fillVisualOffsetX;
		int fillDesiredTopLeftY = frameTopLeftY + fillVisualOffsetY;

		// Calculate center for the fill sprite based on its desired top-left
		fillSprite->SetX(fillDesiredTopLeftX + originalFillWidth / 2);
		fillSprite->SetY(fillDesiredTopLeftY + originalFillHeight / 2);

	}
	catch (const std::exception& e) {
		LogManager::GetInstance().Log("UI Element Load Failed: ");
		LogManager::GetInstance().Log(e.what());
		delete frameSprite; frameSprite = nullptr;
		delete fillSprite; fillSprite = nullptr;
		return false;
	}
	return true;
}

bool UIManager::Initialise(Renderer& renderer, Player* player)
{
	if (!player) {
		LogManager::GetInstance().Log("UIManager::Initialise Error: Player pointer is null.");
		return false;
	}
	m_pPlayer = player;
	m_screenWidth = renderer.GetWidth();
	m_screenHeight = renderer.GetHeight();

	const char* healthFrameAsset = "assets/ui/health_frame.png";
	const char* healthFillAsset = "assets/ui/health_fill.png";
	const char* staminaFrameAsset = "assets/ui/stamina_frame.png";
	const char* staminaFillAsset = "assets/ui/stamina_fill.png";

	// Temporarily create frame sprites to get their dimensions for layout calculations
	Sprite* tempHealthFrame = renderer.CreateSprite(healthFrameAsset);
	if (!tempHealthFrame) { LogManager::GetInstance().Log("Failed to temp load health frame for dims"); return false; }
	int healthFrameW = tempHealthFrame->GetOriginalWidth();
	int healthFrameH = tempHealthFrame->GetOriginalHeight();
	delete tempHealthFrame; tempHealthFrame = nullptr;

	Sprite* tempStaminaFrame = renderer.CreateSprite(staminaFrameAsset);
	if (!tempStaminaFrame) { LogManager::GetInstance().Log("Failed to temp load stamina frame for dims"); return false; }
	int staminaFrameW = tempStaminaFrame->GetOriginalWidth();
	int staminaFrameH = tempStaminaFrame->GetOriginalHeight();
	delete tempStaminaFrame; tempStaminaFrame = nullptr;

	// Calculate center positions for Frame sprites
	int healthFrameX = (m_screenWidth / 2) - (healthFrameW / 2) + (healthFrameW / 2) + HEALTH_X_OFFSET; // Simplified: screenWidth/2 + HEALTH_X_OFFSET
	healthFrameX = m_screenWidth / 2 + HEALTH_X_OFFSET;
	int healthFrameY = m_screenHeight - (healthFrameH / 2) - BAR_Y_OFFSET;

	int staminaFrameX = m_screenWidth - (staminaFrameW / 2) - STAMINA_X_OFFSET;
	int staminaFrameY = m_screenHeight - (staminaFrameH / 2) - BAR_Y_OFFSET;


	if (!LoadAndPositionUIElement(renderer, m_pHealthBarFrame, m_pHealthBarFill,
		healthFrameAsset, healthFillAsset,
		healthFrameX, healthFrameY,
		m_healthFillOriginalWidth, m_healthFillOriginalHeight,
		FILL_BAR_OFFSET_X, FILL_BAR_OFFSET_Y)) {
		return false;
	}
	m_healthFillInitialX = m_pHealthBarFill->GetX(); // Store actual initial X center of the fill sprite

	if (!LoadAndPositionUIElement(renderer, m_pStaminaBarFrame, m_pStaminaBarFill,
		staminaFrameAsset, staminaFillAsset,
		staminaFrameX, staminaFrameY,
		m_staminaFillOriginalWidth, m_staminaFillOriginalHeight,
		FILL_BAR_OFFSET_X, FILL_BAR_OFFSET_Y)) {
		return false;
	}
	m_staminaFillInitialX = m_pStaminaBarFill->GetX(); // Store actual initial X center

	// Initial update
	m_targetHealthPercent = m_pPlayer->IsAlive() ? (float)m_pPlayer->GetCurrentHealth() / m_pPlayer->GetMaxHealth() : 0.0f;
	m_displayHealthPercent = m_targetHealthPercent;
	m_targetStaminaPercent = m_pPlayer->IsAlive() ? m_pPlayer->GetCurrentStamina() / m_pPlayer->GetMaxStamina() : 0.0f;
	m_displayStaminaPercent = m_targetStaminaPercent;

	UpdateHealthVisuals();
	UpdateStaminaVisuals();

	LogManager::GetInstance().Log("UIManager Initialised Successfully.");
	return true;
}

void UIManager::Process(float deltaTime)
{
	if (!m_pPlayer) return;

	float currentMaxHealth = static_cast<float>(m_pPlayer->GetMaxHealth());
	float currentMaxStamina = m_pPlayer->GetMaxStamina();

	if (!m_pPlayer->IsAlive()) {
		m_targetHealthPercent = 0.0f;
		// Stamina also to 0 or last value if preferred when dead
		m_targetStaminaPercent = 0.0f;
	}
	else {
		m_targetHealthPercent = (currentMaxHealth > 0) ? (float)m_pPlayer->GetCurrentHealth() / currentMaxHealth : 0.0f;
		m_targetStaminaPercent = (currentMaxStamina > 0) ? m_pPlayer->GetCurrentStamina() / currentMaxStamina : 0.0f;
	}
	m_targetHealthPercent = std::max(0.0f, std::min(1.0f, m_targetHealthPercent));
	m_targetStaminaPercent = std::max(0.0f, std::min(1.0f, m_targetStaminaPercent));


	bool healthNeedsUpdate = false;
	if (std::abs(m_displayHealthPercent - m_targetHealthPercent) > 0.001f) {
		if (m_displayHealthPercent > m_targetHealthPercent) {
			m_displayHealthPercent -= BAR_CHANGE_SPEED * deltaTime;
			m_displayHealthPercent = std::max(m_displayHealthPercent, m_targetHealthPercent);
		}
		else {
			m_displayHealthPercent += BAR_CHANGE_SPEED * deltaTime; // Animate healing too
			m_displayHealthPercent = std::min(m_displayHealthPercent, m_targetHealthPercent);
		}
		healthNeedsUpdate = true;
	}

	bool staminaNeedsUpdate = false;
	if (std::abs(m_displayStaminaPercent - m_targetStaminaPercent) > 0.001f) {
		if (m_displayStaminaPercent > m_targetStaminaPercent) {
			m_displayStaminaPercent -= BAR_CHANGE_SPEED * deltaTime; // Or make stamina drain/regen instant
			m_displayStaminaPercent = std::max(m_displayStaminaPercent, m_targetStaminaPercent);
		}
		else {
			m_displayStaminaPercent += BAR_CHANGE_SPEED * deltaTime;
			m_displayStaminaPercent = std::min(m_displayStaminaPercent, m_targetStaminaPercent);
		}
		staminaNeedsUpdate = true;
	}

	if (healthNeedsUpdate) UpdateHealthVisuals();
	if (staminaNeedsUpdate) UpdateStaminaVisuals();
}

void UIManager::UpdateHealthVisuals()
{
	if (!m_pHealthBarFill) return;
	float scaleX = std::max(0.0001f, m_displayHealthPercent); // Ensure non-zero for calculations

	m_pHealthBarFill->SetScaleX(scaleX);
	m_pHealthBarFill->SetScaleY(1.0f); // Keep original height scale

	// Adjust X position to keep left edge fixed, since sprite scales from its center
	int originalWidth = m_pHealthBarFill->GetOriginalWidth(); // This is fill's original texture width
	// Calculate the shift needed for the center of the sprite
	float widthReduction = originalWidth * (1.0f - scaleX);
	int newXCenter = m_healthFillInitialX - static_cast<int>(widthReduction / 2.0f);
	m_pHealthBarFill->SetX(newXCenter);
}

void UIManager::UpdateStaminaVisuals()
{
	if (!m_pStaminaBarFill) return;
	float scaleX = std::max(0.0001f, m_displayStaminaPercent);

	m_pStaminaBarFill->SetScaleX(scaleX);
	m_pStaminaBarFill->SetScaleY(1.0f);

	int originalWidth = m_pStaminaBarFill->GetOriginalWidth();
	float widthReduction = originalWidth * (1.0f - scaleX);
	int newXCenter = m_staminaFillInitialX - static_cast<int>(widthReduction / 2.0f);
	m_pStaminaBarFill->SetX(newXCenter);
}

void UIManager::Draw(Renderer& renderer)
{
	// Draw order: Fill first, then Frame on top
	if (m_pHealthBarFill) m_pHealthBarFill->Draw(renderer);
	if (m_pHealthBarFrame) m_pHealthBarFrame->Draw(renderer);

	if (m_pStaminaBarFill) m_pStaminaBarFill->Draw(renderer);
	if (m_pStaminaBarFrame) m_pStaminaBarFrame->Draw(renderer);
}