#include <stdio.h>
#include "game.h"




int main(int argc, char* args[])
{
	
	Game the_game;
	
	if (the_game.init()) {
		if (the_game.load_media(the_game.renderer))
		{
			the_game.start();
			the_game.close();
		}
	} 


	return 0;
}
