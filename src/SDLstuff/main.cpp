
   
#include "Game.h"
#include "SDL/SDL.h"

int main(int argc, char *argv[])
{
	//create Game object, which will instantiate everything else too
	Game::Instance()->mainLoop();

	//exit game
	SDL_Quit();

    return 0;
}


/************************************************************************/



