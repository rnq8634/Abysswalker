// This include
#include "SceneTitleScreen.h"

// Local includes
#include "Game.h"
#include "Renderer.h"
#include "InputSystem.h"
#include "LogManager.h"
#include "Sprite.h"
#include "Texture.h"

// IMGUI
#include "imgui/imgui.h"

const char* DEFAULT_FONT_PATH = "assets/fonts/OptimusPrinceps.ttf";
const int DEFAULT_FONT_SIZE = 20;

SceneTitleScreen::SceneTitleScreen()
    : m_pNewGameTextSprite(nullptr)
    , m_pControlsTextSprite(nullptr)
    , m_pQuitTextSprite(nullptr)
    , m_pNewGameTextTexture(nullptr)   // Initialize texture pointers
    , m_pControlsTextTexture(nullptr)
    , m_pQuitTextTexture(nullptr)
    , m_fontPath(DEFAULT_FONT_PATH)
    , m_fontSize(DEFAULT_FONT_SIZE)
{
    // Initialize button properties
    m_newGameButton = { Vector2(0, 0), Vector2(200, 50), "New Game", false };
    m_controlsButton = { Vector2(0, 0), Vector2(200, 50), "Controls", false };
    m_quitButton = { Vector2(0, 0), Vector2(200, 50), "Quit Game", false };
}

SceneTitleScreen::~SceneTitleScreen()
{
    delete m_pNewGameTextSprite;
    m_pNewGameTextSprite = nullptr;
    delete m_pControlsTextSprite;
    m_pControlsTextSprite = nullptr;
    delete m_pQuitTextSprite;
    m_pQuitTextSprite = nullptr;

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

    const int screenWidth = renderer.GetWidth();
    const int screenHeight = renderer.GetHeight();

    // Position buttons
    m_newGameButton.position = Vector2(screenWidth / 2.0f, screenHeight / 2.0f);
    m_controlsButton.position = Vector2(screenWidth / 2.0f, screenHeight / 2.0f + 70);
    m_quitButton.position = Vector2(screenWidth / 2.0f, screenHeight / 2.0f + 140);

    // Text for New Game
    m_pNewGameTextTexture = new Texture();
    m_pNewGameTextSprite = new Sprite();
    if (!m_pNewGameTextTexture || !m_pNewGameTextSprite) { return false; }

    if (!m_pNewGameTextSprite->InitialiseWithText(*m_pNewGameTextTexture, m_newGameButton.text, m_fontPath, m_fontSize))
    {
        LogManager::GetInstance().Log("Failed to initialise New Game Text Sprite!!");
    }
    else
    {
        float textX = m_newGameButton.position.x - m_pNewGameTextSprite->GetOriginalWidth() / 2.0f;
        float textY = m_newGameButton.position.y - m_pNewGameTextSprite->GetOriginalHeight() / 2.0f;

        m_pNewGameTextSprite->SetX(static_cast<int>(textX));
        m_pNewGameTextSprite->SetY(static_cast<int>(textY));
    }

    // Text for Controls
    m_pControlsTextTexture = new Texture();
    m_pControlsTextSprite = new Sprite();
    if (!m_pControlsTextTexture || !m_pControlsTextSprite) { return false; }
    if (!m_pControlsTextSprite->InitialiseWithText(*m_pControlsTextTexture, m_controlsButton.text, m_fontPath, m_fontSize))
    {
        LogManager::GetInstance().Log("Failed to initialize Controls text sprite.");
    }
    else
    {
        float textX = m_controlsButton.position.x - m_pControlsTextSprite->GetOriginalWidth() / 2.0f;
        float textY = m_controlsButton.position.y - m_pControlsTextSprite->GetOriginalHeight() / 2.0f;
        m_pControlsTextSprite->SetX(static_cast<int>(textX));
        m_pControlsTextSprite->SetY(static_cast<int>(textY));
    }

    // Quit Game Text
    m_pQuitTextTexture = new Texture();
    m_pQuitTextSprite = new Sprite();
    if (!m_pQuitTextTexture || !m_pQuitTextSprite) { return false; }
    if (!m_pQuitTextSprite->InitialiseWithText(*m_pQuitTextTexture, m_quitButton.text, m_fontPath, m_fontSize))
    {
        LogManager::GetInstance().Log("Failed to initialize Quit Game text sprite.");
    }
    else
    {
        float textX = m_quitButton.position.x - m_pQuitTextSprite->GetOriginalWidth() / 2.0f;
        float textY = m_quitButton.position.y - m_pQuitTextSprite->GetOriginalHeight() / 2.0f;
        m_pQuitTextSprite->SetX(static_cast<int>(textX));
        m_pQuitTextSprite->SetY(static_cast<int>(textY));
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

    // Check clicks
    if (inputSystem.GetMouseButtonState(SDL_BUTTON_LEFT) == BS_PRESSED)
    {
        if (m_newGameButton.isHovered)
        {
            Game::GetInstance().SetCurrentScene(1); // Switch to game scene
        }
        else if (m_controlsButton.isHovered)
        {
            // Switch to Controls (Controls should have a button that goes back to title screen)
        }
        else if (m_quitButton.isHovered)
        {
            Game::GetInstance().Quit();
        }
    }
}

void SceneTitleScreen::Draw(Renderer& renderer)
{
    // Helper lambda to draw a button
    auto DrawButton = [&renderer](const Button& button) 
    {
        // Button background (color changes on hover)
        unsigned char r = button.isHovered ? 120 : 80;
        unsigned char g = button.isHovered ? 120 : 80;
        unsigned char b = button.isHovered ? 120 : 80;

        renderer.DrawDebugRect(
            button.position.x - button.size.x / 2.0f,
            button.position.y - button.size.y / 2.0f,
            button.position.x + button.size.x / 2.0f,
            button.position.y + button.size.y / 2.0f,
            r, g, b, 255
        );  
    };

    // Draw all buttons
    DrawButton(m_newGameButton);
    if (m_pNewGameTextSprite)
    {
        m_pNewGameTextSprite->Draw(renderer);
    }
    DrawButton(m_controlsButton);
    if (m_pControlsTextSprite)
    {
        m_pControlsTextSprite->Draw(renderer);
    }

    DrawButton(m_quitButton);
    if (m_pQuitTextSprite)
    {
        m_pQuitTextSprite->Draw(renderer);
    }
}

bool SceneTitleScreen::IsMouseOverButton(const Button& button, const Vector2& mousePos)
{
    return (mousePos.x >= button.position.x - button.size.x / 2.0f &&
        mousePos.x <= button.position.x + button.size.x / 2.0f &&
        mousePos.y >= button.position.y - button.size.y / 2.0f &&
        mousePos.y <= button.position.y + button.size.y / 2.0f);
}

void SceneTitleScreen::DebugDraw()
{
    ImGui::Text("TITLE SCREEN");
}