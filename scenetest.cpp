// This inlcude
#include "SceneTest.h"

// Local Include
#include "Player.h"
#include "Renderer.h"
#include "InputSystem.h"

// IMGUI
#include "imgui/imgui.h"

SceneTest::SceneTest()
	: m_pPlayer(0)
{

}

SceneTest::~SceneTest()
{
	delete m_pPlayer;
	m_pPlayer = 0;
}

bool
SceneTest::Initialise(Renderer& renderer)
{
	// Create and initialise player
	m_pPlayer = new Player();
	if (!m_pPlayer->Initialise(renderer))
	{
		return false;
	}

	return true;
}

void
SceneTest::Process(float deltaTime, InputSystem& inputSystem)
{
	/*
	* if (m_pPlayer)
	{
		m_pPlayer->Process(deltaTime);
	}
	*/

	if (!m_pPlayer) return;

	const float moveSpeed = 100.0f;
	bool isMoving = false;

	// Switch


	// COntrols for the player
	// Move Right = 'D'
	// Move Left = 'A'
	// Crouch = 'S'
	// Dodge/Roll = 'Q'
	// Interact = 'F'
	// Jump = 'Spacebar'
	// Attack = 'J'
	// Move Left 'A' Key
	if (inputSystem.GetKeyState(SDL_SCANCODE_A) == BS_HELD ||
		inputSystem.GetKeyState(SDL_SCANCODE_LEFT) == BS_HELD)
	{
		m_pPlayer->MoveLeft(moveSpeed);
		isMoving = true;
	}
	else if (inputSystem.GetKeyState(SDL_SCANCODE_D) == BS_HELD ||
		inputSystem.GetKeyState(SDL_SCANCODE_RIGHT) == BS_HELD)
	{
		m_pPlayer->MoveRight(moveSpeed);
		isMoving = true;
	}

	if (!isMoving)
	{
		m_pPlayer->StopMoving();
	}

	m_pPlayer->Process(deltaTime);
}

void
SceneTest::Draw(Renderer& renderer)
{
	// Draw the player
	if (m_pPlayer)
	{
		m_pPlayer->Draw(renderer);
	}
}

void
SceneTest::DebugDraw()
{
	if (ImGui::CollapsingHeader("Scene Test"))
	{
		ImGui::Text("Scene Debug Information");
	}

	if (m_pPlayer)
	{
		m_pPlayer->DebugDraw();
	}
}