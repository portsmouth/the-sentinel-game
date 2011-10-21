

#include "Controls.h"
#include "SDL/SDL.h"
#include "SDLview.h"
#include "Player.h"
#include "Maths.h"

SDL_Event event;

Controls* Controls::_instance = NULL;

Controls* Controls::Instance()
{
	if (_instance == NULL)
	{
		_instance = new Controls;
	}
	return _instance;
}

Controls::Controls() {}

UserAction Controls::checkUserInput(Player *pPlayer)
{		
	enum UserAction action;
	int num_keys;
	Uint8 *key_states = SDL_GetKeyState(&num_keys);
	
	SDL_PumpEvents();
	
	int ls = 0;
	
	if (key_states)
	{
		switch (ls) 
		{			
			case 0:
				{
					// Do not allow keyboard viewpoint control if currently scrolling via drag zones
					if ( !pPlayer->crosshairsInDragzone() )
					{
						float dragAngle = pPlayer->perFrameDragAngle();
						if ( key_states[SDLK_UP] )    pPlayer->RotateViewAboutHorizontal(dragAngle);	
						if ( key_states[SDLK_DOWN] )  pPlayer->RotateViewAboutHorizontal(-dragAngle);	
						if ( key_states[SDLK_LEFT] )  pPlayer->RotateViewAboutVertical(-dragAngle);
						if ( key_states[SDLK_RIGHT] ) pPlayer->RotateViewAboutVertical(dragAngle);
					}
				}
				break;
				
			default:
				printf("error: incorrect level status in Controls, %d\n",ls);
				exit(1);
				break;
		}
	}

	action = NOACTION;
			
	while ( SDL_PollEvent (&event) ) 
	{
		switch (event.type) {			
			
			case SDL_MOUSEMOTION:
				Look(pPlayer);
				break;
				
			case SDL_MOUSEBUTTONDOWN:
				//leftbutton = DOWN;
				break;
				
			case SDL_MOUSEBUTTONUP:
				//leftbutton = UP;
				break;
				
			case SDL_KEYDOWN:	
					
				if ( pPlayer->theConsole.IsOpen() )
				{
					printf("console is open\n");
					//pass all key strokes to the console
					pPlayer->theConsole.KeyboardFunc( event.key.keysym.sym );
				}
				
				/*
				if ( event.key.keysym.sym>=SDLK_a && event.key.keysym.sym<=SDLK_z )
				{
					
				}
				*/
				
				else
				{
					switch( event.key.keysym.sym )
					{
						case SDLK_ESCAPE:
							action = PAUSE;
							break;
							
						case SDLK_c:
							pPlayer->theConsole.ToggleConsole();
							break;
							
						case SDLK_a:
							action = ABSORB;
							break;	
							
						case SDLK_r:
							action = CREATE_SYNTHOID_SHELL;
							break;
							
						case SDLK_q:
							action = TRANSFER_TO_SYNTHOID;
							break;
							
						case SDLK_t:
							action = CREATE_TREE;
							break;	
							
						case SDLK_b:
							action = CREATE_BOULDER;
							break;	
							
						case SDLK_h:
							action = HYPERSPACE;
							break;	
							
						default:
							break;
					}
				}

				
			default:
			break;
		}
	}	
	
	return action;
}


void Controls::Look(Player *pPlayer)
{
	int mouse_dx, mouse_dy;
	//float dAngle, dAzimuth;
	
	int MAX_MOUSE_MOTION = 40;
	
	(Uint8)SDL_GetRelativeMouseState(&mouse_dx, &mouse_dy);	
	
	if (mouse_dx>MAX_MOUSE_MOTION)
		mouse_dx = MAX_MOUSE_MOTION;
	else if (mouse_dx<-MAX_MOUSE_MOTION) 
		mouse_dx = -MAX_MOUSE_MOTION;
	
	if (mouse_dy>MAX_MOUSE_MOTION) 
		mouse_dy = MAX_MOUSE_MOTION;
	else if (mouse_dy<-MAX_MOUSE_MOTION) 
		mouse_dy = -MAX_MOUSE_MOTION;
	
	//mouse sensitivity is currently hard-coded..
	// TO DO: ensure that the sensitivity is such that, given the screen size,
	// the cross-hairs can move by less than a pixel relative to the furthest squares
	//dAngle   = 0.5e-3f * 2.0*M_PI*(float)mouse_dx;	
	//dAzimuth = -0.5e-3f * 2.0*M_PI*(float)mouse_dy;	
	
	int dims[2];
	SDLview::Instance()->getViewDims(dims);
	
	pPlayer->MoveCrossHairs(  static_cast<float>(mouse_dx) / static_cast<float>(dims[0]),
							 -static_cast<float>(mouse_dy) / static_cast<float>(dims[1]) );

}




