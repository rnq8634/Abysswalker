// Local includes
#include "Scene.h"
#include "InputSystem.h"

class Player;
class Renderer;
class InputSystem;

class SceneTest : public Scene
{
	// Member methods
public:
	SceneTest();
	~SceneTest();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime, InputSystem& inputSystem);
	void Draw(Renderer& renderer);
	void DebugDraw();

protected:

private:
	SceneTest(const SceneTest& scene);
	SceneTest& operator=(const SceneTest& scene);

	// Member data
public:

protected:
	Player* m_pPlayer;

private:

};