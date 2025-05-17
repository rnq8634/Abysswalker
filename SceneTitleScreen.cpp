// This include
#include "SceneTitleScreen.h"

// Local includes
#include "Game.h"
#include "Renderer.h"
#include "InputSystem.h"
#include "LogManager.h"
#include "Sprite.h"
#include "Texture.h"
#include "SoundSystem.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <glew.h>

// TITLE BGM
const char* TITLE_BGM_FILEPATH = "assets/sounds/titleScreenBGM.mp3";
const char* TITLE_BGM_ID = "dmcTitle_bgm";

// TITLE BUTTON SOUND WHEN PRESSED
const char* TITLE_BUTTONPRESS_SOUND = "assets/sounds/titleScreenButton.mp3";
const char* TITLE_BUTTONPRESS_ID = "titleButton";

// Fonts
const char* DEFAULT_FONT_FILEPATH = "assets/fonts/OptimusPrincepsSemiBold.ttf";
const int DEFAULT_FONT_SIZE = 20;

// Title image
const char* TITLESCREEN_IMAGE_FILEPATH = "assets/titleScreen/ABYSSWALKER.png";

SceneTitleScreen::SceneTitleScreen()
    : m_pNewGameTextSprite(nullptr)
    , m_pControlsTextSprite(nullptr)
    , m_pQuitTextSprite(nullptr)
    , m_pNewGameTextTexture(nullptr)
    , m_pControlsTextTexture(nullptr)
    , m_pQuitTextTexture(nullptr)
    , m_pTitleScreenImageSprite(nullptr)
    , m_pTitleScreenImageTexture(nullptr)
    , m_pBGMChannel(nullptr)
    , m_fontPath(DEFAULT_FONT_FILEPATH)
    , m_fontSize(DEFAULT_FONT_SIZE)
{
    // Initialize button properties
    m_newGameButton = { Vector2(0, 0), Vector2(200, 50), "Start Game", false };
    m_controlsButton = { Vector2(0, 0), Vector2(200, 50), "Controls", false };
    m_quitButton = { Vector2(0, 0), Vector2(200, 50), "Quit Game", false };
}

SceneTitleScreen::~SceneTitleScreen()
{
    if (m_pBGMChannel)
    {
        SoundSystem::GetInstance().StopChannel(m_pBGMChannel);
        m_pBGMChannel = nullptr;
    }

    delete m_pNewGameTextSprite;
    m_pNewGameTextSprite = nullptr;
    delete m_pControlsTextSprite;
    m_pControlsTextSprite = nullptr;
    delete m_pQuitTextSprite;
    m_pQuitTextSprite = nullptr;
    delete m_pTitleScreenImageSprite;
    m_pTitleScreenImageSprite = nullptr;

    delete m_pTitleScreenImageTexture;
    m_pTitleScreenImageTexture = nullptr;
    delete m_pNewGameTextTexture;
    m_pNewGameTextTexture = nullptr;
    delete m_pControlsTextTexture;
    m_pControlsTextTexture = nullptr;
    delete m_pQuitTextTexture;
    m_pQuitTextTexture = nullptr;
}

bool SceneTitleScreen::Initialise(Renderer& renderer)
{
    renderer.SetClearColor(0, 0, 0); // Black background

    // To load and play the BGM
    SoundSystem& soundSys = SoundSystem::GetInstance();
    if (!soundSys.LoadSound(TITLE_BGM_FILEPATH, TITLE_BGM_ID, true, true))
    {
        LogManager::GetInstance().Log("Failed to load title screen BGM!");
    }
    else
    {
        m_pBGMChannel = soundSys.PlaySound(TITLE_BGM_ID);
        if (m_pBGMChannel)
        {
            soundSys.SetChannelVolume(m_pBGMChannel, 0.8f); // NOTE: Volume can be set here!
        }
        else
        {
            LogManager::GetInstance().Log("Failed to play title screen BGM");
        }
    }

    if (!soundSys.LoadSound(TITLE_BUTTONPRESS_SOUND, TITLE_BUTTONPRESS_ID, false, false))
    {
        LogManager::GetInstance().Log("Failed to load button sound!");
    }

    m_pTitleScreenImageTexture = new Texture();
    m_pTitleScreenImageSprite = new Sprite();

    if (!m_pTitleScreenImageTexture || !m_pTitleScreenImageSprite)
    {
        LogManager::GetInstance().Log("Failed to allocate memory for game title");
        delete m_pTitleScreenImageTexture; 
        m_pTitleScreenImageTexture = nullptr;
        delete m_pTitleScreenImageSprite; 
        m_pTitleScreenImageSprite = nullptr;
        return false;
    }

    if (!m_pTitleScreenImageTexture->Initialise(TITLESCREEN_IMAGE_FILEPATH))
    {
        LogManager::GetInstance().Log("Failed to initialise game title image texture from file.");
        delete m_pTitleScreenImageTexture; 
        m_pTitleScreenImageTexture = nullptr;
        delete m_pTitleScreenImageSprite;
        m_pTitleScreenImageSprite = nullptr;
        return false;
    }

    if (!m_pTitleScreenImageSprite->Initialise(*m_pTitleScreenImageTexture))
    {
        LogManager::GetInstance().Log("Failed to initialise game title image texture from file.");
        delete m_pTitleScreenImageTexture; 
        m_pTitleScreenImageTexture = nullptr;
        delete m_pTitleScreenImageSprite; 
        m_pTitleScreenImageSprite = nullptr;
        return false;
    }

    const int screenWidth = renderer.GetWidth();
    const int screenHeight = renderer.GetHeight();

    // Position the Title screen image
    m_pTitleScreenImageSprite->SetX(screenWidth / 2);
    m_pTitleScreenImageSprite->SetY(static_cast<int>(screenHeight * 0.30f));
    m_pTitleScreenImageSprite->SetFlipHorizontal(true);
    m_pTitleScreenImageSprite->SetAngle(180.0f);

    // Position buttons
    m_newGameButton.position = Vector2(screenWidth / 2.0f, screenHeight / 2.0f);
    m_controlsButton.position = Vector2(screenWidth / 2.0f, screenHeight / 2.0f + 70);
    m_quitButton.position = Vector2(screenWidth / 2.0f, screenHeight / 2.0f + 140);

    // Text for New Game
    m_pNewGameTextTexture = new Texture();
    m_pNewGameTextSprite = new Sprite();
    if (!m_pNewGameTextTexture || !m_pNewGameTextSprite) { return false; }
    if (m_pNewGameTextSprite->InitialiseWithText(*m_pNewGameTextTexture, m_newGameButton.text, m_fontPath, m_fontSize))
    {
        if (m_pNewGameTextTexture->GetWidth() > 0 && m_pNewGameTextTexture->GetHeight() > 0)
        {
            m_pNewGameTextSprite->SetX(static_cast<int>(m_newGameButton.position.x));
            m_pNewGameTextSprite->SetY(static_cast<int>(m_newGameButton.position.y));
        }
    }

    // Text for Controls
    m_pControlsTextTexture = new Texture();
    m_pControlsTextSprite = new Sprite();
    if (!m_pControlsTextTexture || !m_pControlsTextSprite) { return false; }
    if (m_pControlsTextSprite->InitialiseWithText(*m_pControlsTextTexture, m_controlsButton.text, m_fontPath, m_fontSize))
    {
        if (m_pControlsTextTexture->GetWidth() > 0 && m_pControlsTextTexture->GetHeight() > 0)
        {
            m_pControlsTextSprite->SetX(static_cast<int>(m_controlsButton.position.x));
            m_pControlsTextSprite->SetY(static_cast<int>(m_controlsButton.position.y));
        }
    }

    // Quit Game Text
    m_pQuitTextTexture = new Texture();
    m_pQuitTextSprite = new Sprite();
    if (!m_pQuitTextTexture || !m_pQuitTextSprite) { return false; }
    if (m_pQuitTextSprite->InitialiseWithText(*m_pQuitTextTexture, m_quitButton.text, m_fontPath, m_fontSize))
    {
        if (m_pQuitTextTexture->GetWidth() > 0 && m_pQuitTextTexture->GetHeight() > 0)
        {
            m_pQuitTextSprite->SetX(static_cast<int>(m_quitButton.position.x));
            m_pQuitTextSprite->SetY(static_cast<int>(m_quitButton.position.y));
        }
    }

    return true;
}

void SceneTitleScreen::Process(float deltaTime, InputSystem& inputSystem)
{
    Vector2 mousePos = inputSystem.GetMousePosition();

    // Check hover states
    m_newGameButton.isHovered = IsMouseOverButton(m_newGameButton, mousePos);
    m_controlsButton.isHovered = IsMouseOverButton(m_controlsButton, mousePos);
    m_quitButton.isHovered = IsMouseOverButton(m_quitButton, mousePos);

    // Text color change when hovering over text
    if (m_pNewGameTextSprite)
    {
        if (m_newGameButton.isHovered)
        {
            // Orange color when text is hovered
            m_pNewGameTextSprite->SetRedTint(1.0f);
            m_pNewGameTextSprite->SetGreenTint(0.647f);
            m_pNewGameTextSprite->SetBlueTint(0.0f);
        }
        else
        {
            // Text goes back to white
            m_pNewGameTextSprite->SetRedTint(1.0f);
            m_pNewGameTextSprite->SetGreenTint(1.0f);
            m_pNewGameTextSprite->SetBlueTint(1.0f);
        }
    }

    if (m_pControlsTextSprite)
    {
        if (m_controlsButton.isHovered)
        {
            // Orange color when text is hovered
            m_pControlsTextSprite->SetRedTint(1.0f);
            m_pControlsTextSprite->SetGreenTint(0.647f);
            m_pControlsTextSprite->SetBlueTint(0.0f);
        }
        else
        {
            // Text goes back to white
            m_pControlsTextSprite->SetRedTint(1.0f);
            m_pControlsTextSprite->SetGreenTint(1.0f);
            m_pControlsTextSprite->SetBlueTint(1.0f);
        }
    }

    if (m_pQuitTextSprite)
    {
        if (m_quitButton.isHovered)
        {
            // Orange color when text is hovered
            m_pQuitTextSprite->SetRedTint(1.0f);
            m_pQuitTextSprite->SetGreenTint(0.647f);
            m_pQuitTextSprite->SetBlueTint(0.0f);
        }
        else
        {
            // Text goes back to white
            m_pQuitTextSprite->SetRedTint(1.0f);
            m_pQuitTextSprite->SetGreenTint(1.0f);
            m_pQuitTextSprite->SetBlueTint(1.0f);
        }
    }

    // Check clicks
    if (inputSystem.GetMouseButtonState(SDL_BUTTON_LEFT) == BS_PRESSED)
    {
        SoundSystem& soundSys = SoundSystem::GetInstance();
        if (m_newGameButton.isHovered)
        {
            soundSys.PlaySound(TITLE_BUTTONPRESS_ID);
            if (m_pBGMChannel)
            {
                SoundSystem::GetInstance().StopChannel(m_pBGMChannel);
                m_pBGMChannel = nullptr;
            }

            Game::GetInstance().SetCurrentScene(3); // Switch to game scene
        }
        else if (m_controlsButton.isHovered)
        {
            soundSys.PlaySound(TITLE_BUTTONPRESS_ID);
            // Switch to Controls (Controls should have a button that goes back to title screen)
            // Game::GetInstance().SetCurrentScene(2); // Will switch to controls and player goals
        }
        else if (m_quitButton.isHovered)
        {
            soundSys.PlaySound(TITLE_BUTTONPRESS_ID);
            Game::GetInstance().Quit();
        }
    }
}

void SceneTitleScreen::Draw(Renderer& renderer)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_pTitleScreenImageSprite && m_pTitleScreenImageTexture && m_pTitleScreenImageTexture->GetWidth() > 0)
    {
        m_pTitleScreenImageSprite->Draw(renderer);
    }
    
    m_pNewGameTextSprite->Draw(renderer);
    m_pControlsTextSprite->Draw(renderer);
    m_pQuitTextSprite->Draw(renderer);
}

bool SceneTitleScreen::IsMouseOverButton(const Button& button, const Vector2& mousePos)
{
    float halfWidth = button.size.x / 2.0f;
    float halfHeight = button.size.y / 2.0f;

    return (mousePos.x >= button.position.x - halfWidth &&
        mousePos.x <= button.position.x + halfWidth &&
        mousePos.y >= button.position.y - halfHeight &&
        mousePos.y <= button.position.y + halfHeight);
}

void SceneTitleScreen::DebugDraw()
{
    ImGui::Text("TITLE SCREEN");
}