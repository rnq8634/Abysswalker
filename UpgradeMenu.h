// UpgradeMenu.h
#ifndef __UPGRADEMENU_H__
#define __UPGRADEMENU_H__

// Local includes
#include "PlayerStats.h" 
#include "Vector2.h"

// Lib incldes
#include <vector>
#include <string>

// Forward Declarations
class Renderer;
class InputSystem;
class Sprite;
class Texture;
class Player; 
class SceneAbyssWalker; 

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

    UIButton() : textSprite(nullptr), textTexture(nullptr), isHovered(false) {}

    bool IsMouseOver(float mouseX, float mouseY) const
    {
        return mouseX >= rect.x && mouseX <= rect.x + rect.width &&
            mouseY >= rect.y && mouseY <= rect.y + rect.height;
    }

    ~UIButton() 
    {
    }
};


class UpgradeMenu
{
public:
    UpgradeMenu(Renderer* renderer, Player* player, SceneAbyssWalker* scene);
    ~UpgradeMenu();

    void SetupUI();
    void UpdateUI(InputSystem& inputSystem);
    void Draw(Renderer& renderer);
    void ClearUI();

    bool IsActive() const { return m_bIsActive; }
    void SetActive(bool active) { m_bIsActive = active; if (active) SetupUI(); else ClearUI(); }


private:
    void ActivateButtonAction(const std::string& identifier);
    void CreateUpgradeButton(const std::string& baseLabel, StatType type, const std::string& identifier, int currentVal, float currentValF = -1.0f);

    Renderer* m_pRenderer;
    Player* m_pPlayer;
    SceneAbyssWalker* m_pScene;

    std::vector<UIButton> m_upgradeButtons;
    Sprite* m_pUpgradeMenuTitleSprite;
    Texture* m_pUpgradeMenuTitleTexture;
    Sprite* m_pEssenceTextSprite;
    Texture* m_pEssenceTextTexture;

    int m_selectedUpgradeButtonIndex;
    bool m_bIsActive;

    // UI Layout constants (can be internal)
    const char* m_uiFontPath = "assets/fonts/OptimusPrinceps.ttf";
    const int m_uiFontSize = 18;
    const int m_uiTitleFontSize = 24;
    float m_panelX, m_panelY, m_panelWidth, m_panelHeight;
    float m_buttonWidth, m_buttonHeight, m_spacing, m_textPaddingX;
};

#endif // __UPGRADEMENU_H__