// COMP710 GP Framework
#ifndef __GAME_H_
#define __GAME_H_

// Forward delcarations:
class Renderer;
class Sprite;
class Scene;
class InputSystem;

// Lib includes
#include <vector>

class Game
{
	// Member methods:
public:
	static Game& GetInstance();
	static void DestroyInstance();
	bool Initialise();
	bool DoGameLoop();
	void Quit();
	//IMGUI
	void ToggleDebugWindow();

protected:
	void Process(float deltaTime);
	void Draw(Renderer& renderer);
	void ProcessFrameCounting(float deltaTime);
	
	//IMGUI
	void DebugDraw();

private:
	Game();
	~Game();
	Game(const Game& game);
	Game& operator=(const Game& game);

	// Member data:
public:
	Sprite* m_pCheckerboard;

protected:
	static Game* sm_pInstance;
	Renderer* m_pRenderer;
	InputSystem* m_pInputSystem;

	bool m_bShowDebugWindow;

	__int64 m_iLastTime;
	float m_fExecutionTime;
	float m_fElaspedSeconds;
	int m_iFrameCount;
	int m_iFPS;

	std::vector<Scene*> m_scenes;
	int m_iCurrentScene;

#ifdef USE_LAG
	float m_mfLag;
	int m_iUpdateCount;
#endif // USE_LAG

	bool m_bLooping;

private:

};

#endif // __GAME_H_