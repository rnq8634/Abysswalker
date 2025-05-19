#ifndef __SCENETITLESCREEN_H__
#define __SCENETITLESCREEN_H__

// Local includes
#include "Scene.h"
#include "Vector2.h"
#include "Sprite.h"
#include "fmod.hpp"

// Lib Includes
#include <vector>

// Forward Declarations
class Texture;
class SoundSystem;
class InputSystem;

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

    enum class ButtonAction { NEW_GAME, CONTROLS, QUIT };
    struct Button
    {
        Vector2 position;
        Vector2 size;
        const char* text;
        bool isHovered;
        Sprite* textSprite;
        ButtonAction action;
    };

    // Member data
private:
    std::vector<Button> m_allButtons;
    int m_selectedButtonIndex;

    FMOD::Channel* m_pBGMChannel;

    // Title screen related stuff
    Sprite* m_pTitleScreenImageSprite;
    Texture* m_pTitleScreenImageTexture;

    Sprite* m_pNewGameTextSprite;
    Sprite* m_pQuitTextSprite;

    Texture* m_pNewGameTextTexture;
    Texture* m_pQuitTextTexture;

    // FOnt settings
    const char* m_fontPath;
    int m_fontSize;

    bool IsMouseOverButton(const Button& button, const Vector2& mousePos);
};

#endif // __SCENETITLESCREEN_H__