// Local includes
#include "Scene.h"
#include <vector>

class Player;
class Renderer;
class InputSystem;
class Sprite;

class SceneAbyssWalker : public Scene
{
	// Member methods
public:
	SceneAbyssWalker();
	~SceneAbyssWalker();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime, InputSystem& inputSystem);
	void Draw(Renderer& renderer);
	void DebugDraw();

	void fullBackground(Renderer& renderer);

protected:

private:
	SceneAbyssWalker(const SceneAbyssWalker& scene);
	SceneAbyssWalker& operator=(const SceneAbyssWalker& scene);

	// Member data
public:

protected:
	Player* m_pPlayer;
	Sprite* m_pmoonBackground;
	Sprite* m_ptree5Background;
	Sprite* m_ptree4Background;
	Sprite* m_ptree3Background;
	Sprite* m_ptree2Background;
	Sprite* m_ptree1Background;
	
private:

};