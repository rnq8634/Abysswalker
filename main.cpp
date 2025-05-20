// COMP710 GP Framework 2025

// Library includes:
#include <SDL.h>

// Local includes:
#include "Game.h"
#include "logmanager.h"

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(165);
#endif

	Game& gameInstance = Game::GetInstance();
	if (!gameInstance.Initialise())
	{
		LogManager::GetInstance().Log("Game initialize failed!");
		Game::DestroyInstance();
		LogManager::DestroyInstance();
		return 1;
	}

	while (gameInstance.DoGameLoop())
	{
		// No body.
	}

	Game::DestroyInstance();
	LogManager::DestroyInstance();

	return 0;
}