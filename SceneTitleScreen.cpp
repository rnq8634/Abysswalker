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
#include "XboxController.h"

// IMGUI
#include "imgui/imgui.h"

// Lib includes
#include <glew.h>
#include <vector>

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
    , m_pQuitTextSprite(nullptr)
    , m_pNewGameTextTexture(nullptr)
    , m_pQuitTextTexture(nullptr)
    , m_pTitleScreenImageSprite(nullptr)
    , m_pTitleScreenImageTexture(nullptr)
    , m_pBGMChannel(nullptr)
    , m_fontPath(DEFAULT_FONT_FILEPATH)
    , m_fontSize(DEFAULT_FONT_SIZE)
    , m_selectedButtonIndex(0)
{
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
    delete m_pQuitTextSprite;
    m_pQuitTextSprite = nullptr;
    delete m_pTitleScreenImageSprite;
    m_pTitleScreenImageSprite = nullptr;

    delete m_pTitleScreenImageTexture;
    m_pTitleScreenImageTexture = nullptr;
    delete m_pNewGameTextTexture;
    m_pNewGameTextTexture = nullptr;
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

    Vector2 newGamePos = Vector2(screenWidth / 2.0f, screenHeight / 2.0f);
    Vector2 quitPos = Vector2(screenWidth / 2.0f, screenHeight / 2.0f + 70);
    Vector2 buttonSize = Vector2(200, 50);

    // Position the Title screen image
    m_pTitleScreenImageSprite->SetX(screenWidth / 2);
    m_pTitleScreenImageSprite->SetY(static_cast<int>(screenHeight * 0.30f));
    m_pTitleScreenImageSprite->SetFlipHorizontal(true);
    m_pTitleScreenImageSprite->SetAngle(180.0f);

    // Text for New Game
    m_pNewGameTextTexture = new Texture();
    m_pNewGameTextSprite = new Sprite();
    if (!m_pNewGameTextTexture || !m_pNewGameTextSprite) { return false; }
    if (m_pNewGameTextSprite->InitialiseWithText(*m_pNewGameTextTexture, "Start Game", m_fontPath, m_fontSize))
    {
        if (m_pNewGameTextTexture->GetWidth() > 0 && m_pNewGameTextTexture->GetHeight() > 0)
        {
            m_pNewGameTextSprite->SetX(static_cast<int>(newGamePos.x));
            m_pNewGameTextSprite->SetY(static_cast<int>(newGamePos.y));
            m_allButtons.push_back({ newGamePos, buttonSize, "Start Game", false, m_pNewGameTextSprite, ButtonAction::NEW_GAME });
        }
    }

    // Quit Game Text
    m_pQuitTextTexture = new Texture();
    m_pQuitTextSprite = new Sprite();
    if (!m_pQuitTextTexture || !m_pQuitTextSprite) { return false; }
    if (m_pQuitTextSprite->InitialiseWithText(*m_pQuitTextTexture, "Quit Game", m_fontPath, m_fontSize))
    {
        if (m_pQuitTextTexture->GetWidth() > 0 && m_pQuitTextTexture->GetHeight() > 0)
        {
            m_pQuitTextSprite->SetX(static_cast<int>(quitPos.x));
            m_pQuitTextSprite->SetY(static_cast<int>(quitPos.y));
            m_allButtons.push_back({ quitPos, buttonSize, "Quit Game", false, m_pQuitTextSprite, ButtonAction::QUIT });
        }
    }

    m_selectedButtonIndex = 1;

    return true;
}

void SceneTitleScreen::Process(float deltaTime, InputSystem& inputSystem)
{
    XboxController* pController = nullptr;
    if (inputSystem.GetNumberOfControllersAttached() > 0)
    {
        pController = inputSystem.GetController(0);
    }

    //bool controllerNavigated = false;
    if (pController && !m_allButtons.empty())
    {
        if (pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_DOWN) == BS_PRESSED)
        {
            m_selectedButtonIndex = (m_selectedButtonIndex + 1) % m_allButtons.size();
            //controllerNavigated = true;
        }

        if (pController->GetButtonState(SDL_CONTROLLER_BUTTON_DPAD_UP) == BS_PRESSED)
        {
            m_selectedButtonIndex = (m_selectedButtonIndex - 1 + m_allButtons.size()) % m_allButtons.size();
            //controllerNavigated = true;
        }
    }

    Vector2 mousePos = inputSystem.GetMousePosition();

    for (size_t i = 0; i < m_allButtons.size(); ++i)
    {
        Button& btn = m_allButtons[i];
        btn.isHovered = IsMouseOverButton(btn, mousePos);

        bool isActive = btn.isHovered || (pController && static_cast<int>(i) == m_selectedButtonIndex);

        if (btn.textSprite)
        {
            if (isActive)
            {
                // Orange color when hovered
                btn.textSprite->SetRedTint(1.0f);
                btn.textSprite->SetGreenTint(0.647f);
                btn.textSprite->SetBlueTint(0.0f);
            }
            else
            {
                // White when default
                btn.textSprite->SetRedTint(1.0f);
                btn.textSprite->SetGreenTint(1.0f);
                btn.textSprite->SetBlueTint(1.0f);
            }
        }
    }

    // CHeck for response (mouse or controller)
    bool buttonActionTriggered = false;
    ButtonAction actionToPerform = ButtonAction::NEW_GAME;

    if (inputSystem.GetMouseButtonState(SDL_BUTTON_LEFT) == BS_PRESSED)
    {
        for (const auto& btn : m_allButtons)
        {
            if (btn.isHovered)
            {
                actionToPerform = btn.action;
                buttonActionTriggered = true;
                break;
            }
        }
    }

    if (!buttonActionTriggered && pController && pController->GetButtonState(SDL_CONTROLLER_BUTTON_A) == BS_PRESSED)
    {
        if (m_selectedButtonIndex >= 0 && m_selectedButtonIndex < static_cast<int>(m_allButtons.size()))
        {
            actionToPerform = m_allButtons[m_selectedButtonIndex].action;
            buttonActionTriggered = true;
        }
    }

    if (buttonActionTriggered)
    {
        SoundSystem::GetInstance().PlaySound(TITLE_BUTTONPRESS_ID);
        switch (actionToPerform)
        {
        case ButtonAction::NEW_GAME:
            if (m_pBGMChannel)
            {
                SoundSystem::GetInstance().StopChannel(m_pBGMChannel);
                m_pBGMChannel = nullptr;
            }
            Game::GetInstance().SetCurrentScene(SCENE_INDEX_ABYSSWALKER);
            break;

        case ButtonAction::QUIT:
            Game::GetInstance().Quit();
            break;
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
    
    for (const auto& btn : m_allButtons)
    {
        if (btn.textSprite)
        {
            btn.textSprite->Draw(renderer);
        }
    }
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