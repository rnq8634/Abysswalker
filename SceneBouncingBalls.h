#ifndef __SCENEBOUNCINGBALLS_H__
#define __SCENEBOUNCINGBALLS_H__

// Local includes
#include "scene.h"

// Forward declarations
class Renderer;
class Ball;

// Class declaration
class SceneBouncingBalls : public Scene
{
	// Member methods
public:
	SceneBouncingBalls();
	virtual ~SceneBouncingBalls();

	virtual bool Initialise(Renderer& renderer);
	virtual void Process(float deltaTime, InputSystem& inputSystem);
	virtual void Draw(Renderer& renderer);

	virtual void DebugDraw();

protected:

private:
	SceneBouncingBalls(const SceneBouncingBalls& sceneBoucningBalls);
	SceneBouncingBalls& operator=(const SceneBouncingBalls& sceneBouncingBalls);

	// Member data:
public:

protected:
	Ball* m_pBalls[100];

	int m_iShowCount;

private:

};

#endif // __SCENEBOUNCINGBALLS_H__