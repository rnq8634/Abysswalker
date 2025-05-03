#ifndef __SCENEBALLGAME_H
#define __SCENEBALLGAME_H

// Local includes
#include "scene.h"

// Library includes:
#include <vector>

// Forward Declaration
class Ball;

class SceneBallGame : public Scene
{
	// Member methods
public:
	SceneBallGame();
	virtual ~SceneBallGame();

	virtual bool Initialise(Renderer& renderer);
	virtual void Process(float deltaTime, InputSystem& inputSystem);
	virtual void Draw(Renderer& renderer);

	virtual void DebugDraw();

protected:
	void SpawnGoodBalls(int number);
	void SpawnBadBalls(int number);

	void CheckCollisions();
	bool BallVsBall(Ball* p1, Ball* p2);

private:
	SceneBallGame(const SceneBallGame& sceneballgame);
	SceneBallGame& operator=(const SceneBallGame& sceneballgame);

	// Member data
public:

protected:
	Renderer* m_pRenderer;

	std::vector<Ball*> m_pGoodBalls;
	std::vector<Ball*> m_pBadBalls;

	Ball* m_pPlayerBall;

private:

};

#endif // !__SCENEBALLGAME_H
