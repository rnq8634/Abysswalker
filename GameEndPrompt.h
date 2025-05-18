#ifndef __GAMEENDPROMPT_H__
#define __GAMEENDPROMPT_H__

// Local includes
#include "UITypes.h"

// Lib includes
#include <vector>
#include <string>

// Forward Declarations
class Renderer;
class InputSystem;
class Player;
class SceneAbyssWalker;

class GameEndPrompt
{
public:
    GameEndPrompt(Renderer* renderer, Player* player, SceneAbyssWalker* scene);
    ~GameEndPrompt();

    void SetupUI(const std::string& titleMessage);
    void UpdateUI(InputSystem& inputSystem);
    void Draw(Renderer& renderer);
    void ClearUI();

    bool IsActive() const { return m_bIsActive; }
    // When setting active, a title message is usually required
    void SetActive(bool active, const std::string& titleMessage = "");


private:
    void ActivateButtonAction(const std::string& identifier);

    Renderer* m_pRenderer;
    Player* m_pPlayer;
    SceneAbyssWalker* m_pScene; 

    std::vector<UIButton> m_gameEndButtons;
    Sprite* m_pGameEndTitleSprite;
    Texture* m_pGameEndTitleTexture;
    Sprite* m_pGameEndReviveCostSprite;
    Texture* m_pGameEndReviveCostTexture;

    int m_selectedGameEndButtonIndex;
    bool m_bIsActive;
    std::string m_currentTitleMessage;

    // UI Layout constants
    const char* m_uiFontPath = "assets/fonts/OptimusPrinceps.ttf";
    const int m_uiFontSize = 18;
    const int m_uiTitleFontSize = 24; 
};

#endif // __GAMEENDPROMPT_H__