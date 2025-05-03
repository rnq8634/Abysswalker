// COMP710 GP Framework 2025

/* ---NOTES---
* Naming conventions to remember:
* m_ prefix: Indicates a member variable (class/instance variable)
* etc: m_pTextureManager, m_iWidth
* 
* p in m_p pointer type:
* m_pTextureManager = member pointer to TextureManager
* m_pWindow = member to pointer window
* 
* m_i type indicators:
* m_i = integer member
* m_f = float member
* m_b = boolean member
* 
* const = means constant or wont change.
meaning a function cannot be modified or a variable's value cannot be changed.
*/

// Library includes:
#include <SDL.h>

// Local includes:
#include "game.h"
#include "logmanager.h"

int main(int argc, char* argv[])
{
	Game& gameInstance = Game::GetInstance();
	if (!gameInstance.Initialise())
	{
		LogManager::GetInstance().Log("Game initialize failed!");

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