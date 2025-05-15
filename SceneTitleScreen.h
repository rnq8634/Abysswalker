#ifndef __SCENETITLESCREEN_H__
#define __SCENETITLESCREEN_H__

// Local includes
#include "Scene.h"
#include "Vector2.h"
#include "Sprite.h"

// Forward Declarations
class Texture;

class SceneTitleScreen : public Scene
{
    // Member methods
public:
    SceneTitleScreen();
    ~SceneTitleScreen();

    bool Initialise(Renderer& renderer) override;
    void Process(float deltaTime, InputSystem& inputSystem) override;
    void Draw(Renderer& renderer) override;
    void DebugDraw() override;

    // Member data
private:
    struct Button {
        Vector2 position;
        Vector2 size;
        const char* text;
        bool isHovered;
    };

    Button m_newGameButton;
    Button m_controlsButton;
    Button m_quitButton;

    Sprite* m_pTitleScreenTextSprite;
    Sprite* m_pNewGameTextSprite;
    Sprite* m_pControlsTextSprite;
    Sprite* m_pQuitTextSprite;

    Texture* m_pTitleScreenTextTexture;
    Texture* m_pNewGameTextTexture;
    Texture* m_pControlsTextTexture;
    Texture* m_pQuitTextTexture;

    // FOnt settings
    const char* m_fontPath;
    int m_fontSize;

    bool IsMouseOverButton(const Button& button, const Vector2& mousePos);
};

#endif // __SCENETITLESCREEN_H__