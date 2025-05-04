// This inlcude
#include "SceneTest.h"

// Local Include
#include "Player.h"
#include "Renderer.h"

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
	if (m_pPlayer)
	{
		m_pPlayer->Process(deltaTime);
	}
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