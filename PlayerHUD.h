#ifndef __PLAYERHUD_H__
#define __PLAYERHUD_H__

// Forward declarations
class Renderer;
class Player;

class PlayerHUD
{
public:
    PlayerHUD(Renderer* renderer, Player* player);
    ~PlayerHUD();

    void Draw();

private:
    Renderer* m_pRenderer;
    Player* m_pPlayer;

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
};

#endif // __PLAYERHUD_H__