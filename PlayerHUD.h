#ifndef __PLAYERHUD_H__
#define __PLAYERHUD_H__

// Local includes
#include "Texture.h"
#include "Sprite.h"
#include "LogManager.h"
#include "Renderer.h"

// Lib inclkudes
#include <string>

// Forward declarations
class Renderer;
class Player;
class Sprite;
class Texture;

class PlayerHUD
{
public:
    PlayerHUD(Renderer* renderer, Player* player);
    ~PlayerHUD();

    void Draw();

    float GetBarsYPosition() const { if (!m_pRenderer) return 0.0f; return static_cast<float>(m_pRenderer->GetHeight()) - BAR_Y_OFFSET_FROM_BOTTOM - BAR_HEIGHT; }
    const float totalBarGroupWidth = HEALTH_BAR_WIDTH + BAR_SPACING + STAMINA_BAR_WIDTH;
    float GetBarSpacing() const { return BAR_SPACING; }
    float GetHealthBarStartX(int screenWidth) const;

private:
    Renderer* m_pRenderer;
    Player* m_pPlayer;

    // Abyssal Essence
    Sprite* m_pEssenceTextSprite;
    Texture* m_pEssenceTextTexture;
    std::string m_lastEssenceStr;

    const char* m_uiFontPath = "assets/fonts/OptimusPrinceps.ttf";
    const int m_uiFontSize = 16;

    // UI Bar Configs
    const float BAR_HEIGHT = 20.0f;
    const float HEALTH_BAR_WIDTH = 300.0f;
    const float STAMINA_BAR_WIDTH = 200.0f;
    const float BAR_Y_OFFSET_FROM_BOTTOM = 40.0f;
    const float BAR_SPACING = 20.0f;
    const float BORDER_THICKNESS = 5.0f;

    // Colors
    const unsigned char HEALTH_FILL_R = 200, HEALTH_FILL_G = 0, HEALTH_FILL_B = 0, BAR_FILL_A = 220;
    const unsigned char STAMINA_FILL_R = 200, STAMINA_FILL_G = 200, STAMINA_FILL_B = 0;
    const unsigned char BAR_BG_R = 50, BAR_BG_G = 50, BAR_BG_B = 50, BAR_BG_A = 200;
    const unsigned char BAR_BORDER_R = 20, BAR_BORDER_G = 20, BAR_BORDER_B = 20, BAR_BORDER_A = 255;
    const unsigned char ESSENCE_TEXT_COLOR_R = 230, ESSENCE_TEXT_COLOR_G = 230, ESSENCE_TEXT_COLOR_B = 250;
};

#endif // __PLAYERHUD_H__