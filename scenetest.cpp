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
	if (!m_pPlayer) return;

	const float moveSpeed = 80.0f;
	const float jumpSpeed = 80.0f;
	const float rollSpeed = 85.0f;
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
		m_pPlayer->Jump(jumpSpeed);
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