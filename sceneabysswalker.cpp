// This inlcude
#include "SceneAbyssWalker.h"

// Local Include
#include "Player.h"
#include "Renderer.h"
#include "InputSystem.h"
#include "Sprite.h"

// IMGUI
#include "imgui/imgui.h"

SceneAbyssWalker::SceneAbyssWalker()
	: m_pPlayer(0)
	, m_pmoonBackground(nullptr)
	, m_ptree5Background(nullptr)
	, m_ptree4Background(nullptr)
	, m_ptree3Background(nullptr)
	, m_ptree2Background(nullptr)
	, m_ptree1Background(nullptr)
{

}

SceneAbyssWalker::~SceneAbyssWalker()
{
	delete m_pPlayer;

	// Background
	delete m_pmoonBackground;
	delete m_ptree5Background;
	delete m_ptree4Background;
	delete m_ptree3Background;
	delete m_ptree2Background;
	delete m_ptree1Background;

	m_pPlayer = nullptr;
	m_pmoonBackground = nullptr;
	m_ptree5Background = nullptr;
	m_ptree4Background = nullptr;
	m_ptree3Background = nullptr;
	m_ptree2Background = nullptr;
	m_ptree1Background = nullptr;
}

void SceneAbyssWalker::fullBackground(Renderer& renderer)
{
	m_pmoonBackground = renderer.CreateSprite("assets/backgrounds/main_background.png");
	m_ptree5Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree5.png");
	m_ptree4Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree4.png");
	m_ptree3Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree3.png");
	m_ptree2Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree2.png");
	m_ptree1Background = renderer.CreateSprite("assets/backgrounds/bgrd_tree1.png");

	// screen dimensions
	const int screenWidth = renderer.GetWidth();
	const int screenHeight = renderer.GetHeight();
	const int screenCenterX = screenWidth / 2;
	const int screenBottomY = screenHeight;

	if (m_pmoonBackground)
	{
		float scaleX = static_cast<float>(screenWidth) / m_pmoonBackground->GetOriginalWidth();
		float scaleY = static_cast<float>(screenHeight) / m_pmoonBackground->GetOriginalHeight();

		m_pmoonBackground->SetX(screenCenterX);
		m_pmoonBackground->SetY(screenHeight / 2);
		m_pmoonBackground->SetScale(scaleX, scaleY);
		m_pmoonBackground->SetFlipHorizontal(true);
		m_pmoonBackground->SetAngle(180.0f);
	}

	// helper lambda for tree layers
	auto setupTreeLayer = [&](Sprite* treeSprite, float desiredRelativeHeight)
	{
		if (!treeSprite) return;

		//scale to fit screen width
		float scaleToScreenWidthX = static_cast<float>(screenWidth) / treeSprite->GetOriginalWidth();

		float scaleToScreenWidthY = scaleToScreenWidthX;

		treeSprite->SetScale(scaleToScreenWidthX, scaleToScreenWidthY);

		// Position the tree layer
		treeSprite->SetX(screenCenterX);

		float scaledTreeHeight = treeSprite->GetOriginalHeight() * scaleToScreenWidthY;
		treeSprite->SetY(screenBottomY - (scaledTreeHeight / 2));

	};

	setupTreeLayer(m_ptree5Background, 1.0f);
	m_ptree5Background->SetFlipHorizontal(true);
	m_ptree5Background->SetAngle(180.0f);
	setupTreeLayer(m_ptree4Background, 1.0f);
	m_ptree4Background->SetFlipHorizontal(true);
	m_ptree4Background->SetAngle(180.0f);
	setupTreeLayer(m_ptree3Background, 1.0f);
	m_ptree3Background->SetFlipHorizontal(true);
	m_ptree3Background->SetAngle(180.0f);
	setupTreeLayer(m_ptree2Background, 1.0f);
	m_ptree2Background->SetFlipHorizontal(true);
	m_ptree2Background->SetAngle(180.0f);
	setupTreeLayer(m_ptree1Background, 1.0f);
	m_ptree1Background->SetFlipHorizontal(true);
	m_ptree1Background->SetAngle(180.0f);
}

bool SceneAbyssWalker::Initialise(Renderer& renderer)
{
	fullBackground(renderer);

	// Create and initialise player
	m_pPlayer = new Player();
	if (!m_pPlayer->Initialise(renderer))
	{
		return false;
	}

	return true;
}

void SceneAbyssWalker::Process(float deltaTime, InputSystem& inputSystem)
{
	if (!m_pPlayer) return;

	const float moveSpeed = 125.0f;
	const float rollSpeed = 105.0f;
	bool isMoving = false;

	// Movement keys left and right
	if (inputSystem.GetKeyState(SDL_SCANCODE_A) == BS_HELD)
	{
		m_pPlayer->MoveLeft(moveSpeed);
		isMoving = true;
	}
	else if (inputSystem.GetKeyState(SDL_SCANCODE_D) == BS_HELD)
	{
		m_pPlayer->MoveRight(moveSpeed);
		isMoving = true;
	}

	// Other player actions
	if (inputSystem.GetKeyState(SDL_SCANCODE_SPACE) == BS_PRESSED)
	{
		m_pPlayer->Jump();
	}

	if (inputSystem.GetKeyState(SDL_SCANCODE_J) == BS_PRESSED)
	{
		m_pPlayer->Attack();
	}
	if (inputSystem.GetKeyState(SDL_SCANCODE_Q) == BS_PRESSED)
	{
		m_pPlayer->Roll(rollSpeed);
	
	}

	if (!isMoving &&
		m_pPlayer->GetCurrentState() != PlayerState::JUMPING &&
		m_pPlayer->GetCurrentState() != PlayerState::FALLING &&
		m_pPlayer->GetCurrentState() != PlayerState::TURNING &&
		m_pPlayer->GetCurrentState() != PlayerState::ROLLING &&
		m_pPlayer->GetCurrentState() != PlayerState::ATTACKING)
	{
		m_pPlayer->StopMoving();
	}

	m_pPlayer->Process(deltaTime);
}

void SceneAbyssWalker::Draw(Renderer& renderer)
{
	// draw the background
	if (m_pmoonBackground)
	{
		m_pmoonBackground->Draw(renderer);
	}

	if (m_ptree5Background)
	{
		m_ptree5Background->Draw(renderer);
	}

	if (m_ptree4Background)
	{
		m_ptree4Background->Draw(renderer);
	}

	if (m_ptree3Background)
	{
		m_ptree3Background->Draw(renderer);
	}

	if (m_ptree2Background)
	{
		m_ptree2Background->Draw(renderer);
	}

	if (m_ptree1Background)
	{
		m_ptree1Background->Draw(renderer);
	}

	// Draw the player
	if (m_pPlayer)
	{
		m_pPlayer->Draw(renderer);
	}
}

void SceneAbyssWalker::DebugDraw()
{
	if (m_pPlayer)
	{
		m_pPlayer->DebugDraw();
	}
}