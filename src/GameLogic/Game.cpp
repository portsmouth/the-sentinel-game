

#include "Game.h"

#include "World.h"
#include "EventManager.h"
#include "Player.h"
#include "SDLview.h"
#include "HUD.h"
#include "Font.h"
#include "Titles.h"
#include "Sound.h"
#include "TextureLibrary.h"
#include "ModelLibrary.h"
#include "DisplayHandler.h"
#include "Controls.h"
#include "Timer.h"

///DEBUG
#include "Sentinel.h"




Game* Game::_instance = NULL;

Game* Game::Instance()
{
	if (_instance == NULL)
	{
		_instance = new Game;
	}
	return _instance;
}

// static pointer to the current game level's world
World* Game::pWorld = NULL;


Game::Game() 
{
	// instantiate all the singletons on game startup
	SDLview         ::Instance();
	EventManager    ::Instance();
	Player          ::Instance();
	HUD             ::Instance();
	Font            ::Instance();
	Titles          ::Instance();
	Sound           ::Instance();
	TextureLibrary  ::Instance();
	ModelLibrary    ::Instance();
	DisplayHandler  ::Instance();
	Controls        ::Instance();
	TimerRegistry   ::Instance();
	
	LFT = new Timer;
	LET = new Timer;
}


int Game::getLevel() 
{
	return m_level;
}

//small delay to avoid multiple key detections
static void _keyDelay(float dt)
{
	Timer delay;
	delay.start();
	while ( delay.time() < dt )
	{
		continue;
	}
}

void Game::mainLoop()
{			
	Font::Instance()   ->loadAssets();
	HUD::Instance()    ->loadAssets();
	Titles::Instance() ->loadAssets();
	
	bool endGame = false;	
	
	while ( !endGame ) 
	{
		_keyDelay(0.2f);
		
		/* display title screen  */
		SDL_ShowCursor(SDL_ENABLE);
		
		enum title_events request = TITLE_SCREEN_NO_EVENT;
		Titles::Instance()->drawTitleScreen();
		
		while (!request)
		{
			Titles::Instance()->checkTitleEvents(&request);
		}
		
		switch (request) 
		{
			case TITLE_SCREEN_QUIT_GAME:  //quit
				endGame = true;
				break;
				
			case TITLE_SCREEN_ENTER_CODE: //code screen
			{
				_keyDelay(0.3f);
				int level = Titles::Instance()->levelChoice();
				
				if ( enterCode() == -1 )
				{
					request = TITLE_SCREEN_NO_EVENT;
					break;
				}

				m_level = level;
				
				if ( levelEntry() == -1 )
				{
					request = TITLE_SCREEN_ENTER_CODE;
					break;
				}
				
				playLevel();
				break;
			}
			
			case TITLE_SCREEN_PLAY_NOW: //level entry screen screen
			{
				m_level = 0;
				
				if ( levelEntry() == -1 )
				{
					_keyDelay(0.1f);
					request = TITLE_SCREEN_ENTER_CODE;
					break;
				}
			
				playLevel();
				break;
			}
			
			default:
				break;
		}	
		
		if ( m_levelStatus == LEVEL_ADVANCE )
		{
			Titles::Instance()->drawCodeRevealScreen(m_level, Player::Instance()->getEnergy());
			m_levelStatus = LEVEL_GAMEOVER;
		}
		
	}  //while ( !endGame )
		
	//quit!
		
}


const float LEVEL_LOADING_MIN_TIME = 0.5f;

void Game::playLevel()
{
	Timer levelloadingTimer, levelEntryTimer;
	levelloadingTimer.start();
	Titles::Instance()->drawLevelLoadingScreen(m_level);
	
	//create level
	pWorld = new World(m_level);
	pWorld->Init();

	Player::Instance()->InitObserver();
	EventManager::Instance()->setup();	
	HUD::Instance()->init();
	
	//level loading screen appears for a minimum of LEVEL_LOADING_MIN_TIME secs
	while (levelloadingTimer.time()<LEVEL_LOADING_MIN_TIME) 
		Titles::Instance()->drawLevelLoadingScreen(m_level);
	
	SDL_ShowCursor(SDL_DISABLE);
	gameGLsettings();
		
	//play sound indicating level entry
	Sound::Instance()->PlayGeneralSound(EnterGrid,1.0);
	LET->start(); //LevelEntryTimer 
	
	enum UserAction action = NOACTION;
	LFT->start(); //LastFrameTimer
	m_levelStatus = LEVEL_INPROGRESS;
	
	while (m_levelStatus == LEVEL_INPROGRESS)
	{	
		switch (action) 
		{						
			case NOACTION:				
				action = update();
				break;
				
			case PAUSE:				
				action = pause();	
				break;
				
			default:
				break;
		}						
	}
				
	//destroy this level's world and hyperspace
	delete pWorld;	
}
	
/* implements "fixed interval" time-based animation (i.e. variable number of game updates per frame) */

const float MAXIMUM_FRAME_RATE = 100.f;
const float MINIMUM_FRAME_RATE = 10.f;

UserAction Game::update()
{
	enum UserAction action = NOACTION;
	
	//UINT = Update Interval
	const float UINT = 0.5f / MAXIMUM_FRAME_RATE;
	
	//MCPF = MAX_CYCLES_PER_FRAME
	const float MCPF = (2.0f*MAXIMUM_FRAME_RATE / MINIMUM_FRAME_RATE);
	
	//Max update interval
	const float MAXI = MCPF*UINT;
	
	static double timeLeftOver = 0.0; 
	
	double iup = LFT->time() + timeLeftOver; //updateIterations
	if (iup > MAXI) iup = MAXI;
		
	bool updated = false;
	
	Player* pPlayer = Player::Instance();
	Player_States playerState;
	
	while (iup > UINT) 
	{		
		iup -= UINT;																					
		
		//if player is respawning or dead we don't want to check input
		playerState = pPlayer->GetState();
		
		if (playerState != PLAYER_KILLED) 
		{
			action = Controls::Instance()->checkUserInput( pPlayer );
		}
	
		//break out of update cycle if user requests a pause
		if (action==PAUSE) break;
		
		//main update of game world
		EventManager::Instance()->doUpdate( action, &m_levelStatus );
		updated = true;
	}
							
	//keep track of time left over
	timeLeftOver = iup;
	LFT->start();
	
	if (m_levelStatus == LEVEL_GAMEOVER) return action;
	
	if (action==PAUSE) return action;
	
	action = NOACTION;
	
	//draw the game world and HUD
	if (updated)
	{
		drawFrame();
		
		//SDLview::Instance()->drawFPS();
		
		//information overlays on top of action
		drawInfo(playerState);
		
		//draw the console. always call it last so it is drawn on top of everything
		pPlayer->theConsole.RenderConsole();
				
		SDL_GL_SwapBuffers ();
		
#ifdef DEBUG		
		// report any GL errors:	(REMOVE FOR RELEASE)		
		GLenum gl_error = ::glGetError();
		if(gl_error != GL_NO_ERROR)
			fprintf(stderr, "The Sentinel: OpenGL error: %d\n", (int)gl_error);
		
		char *sdl_error = ::SDL_GetError();
		if(sdl_error[0] != '\0')
		{
			fprintf(stderr, "The Sentinel: SDL error '%s'\n", sdl_error);
			::SDL_ClearError();
		}
#endif		
	}
	
	return action;
}				

const float fog_color_LEVEL[]      = {0.02, 0.02, 0.02, 1.0};
const float fog_color_HYPERSPACE[] = {0.0,  0.0,  0.0,  1.0};
const float LEVEL_FADEIN_TIME = 3.0;

void Game::drawFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
	/* initialize observer */
	Player::Instance()->SetObserverProjection();
	placeLights();
	
	/* draw game world */
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	pWorld->draw();		
	glPopAttrib();
	
	//DEBUG
	//if (pWorld->pSentinel)
	//	pWorld->pSentinel->_drawLine_DEBUG();
	
	//draw fade-in overlay if level just started
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	Titles::Instance()->drawLevelFadeInOverlay( 1.0-LET->time()/LEVEL_FADEIN_TIME );
	glPopAttrib();
	
	/* draw game info display (HUD) */
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	bool isPaused = ( m_levelStatus == LEVEL_PAUSED );
	HUD::Instance()->display(isPaused);
	glPopAttrib();
	
}

void Game::drawInfo(Player_States playerState)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	switch (playerState) 
	{																
		case PLAYER_KILLED:
			Titles::Instance()->drawGameOverOverlay();
			break;	
			
		case PLAYER_WINS_LEVEL:
			Titles::Instance()->drawLevelWonOverlay();
			break;
			
		default:
			break;
	}	
	
	glPopAttrib();	
}

const float position0[] = { 100.0, 100.0, 50.0, 0.0 };
const float position1[] = { -100.0,  -100.0, 50.0, 0.0 };

inline void Game::placeLights()
{
	 glLightfv(GL_LIGHT0, GL_POSITION, position0);
	 glLightfv(GL_LIGHT1, GL_POSITION, position1);
}

void Game::gameGLsettings()
{
	//glFogfv(GL_FOG_COLOR, fog_color_LEVEL);
	glClearColor(fog_color_LEVEL[0], fog_color_LEVEL[1], fog_color_LEVEL[2], 0.0);
	
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glShadeModel(GL_SMOOTH);	
	glEnable(GL_POLYGON_SMOOTH);
	glDisable(GL_BLEND);
	
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	
	float ambient[]   = { 0.5f, 0.5f, 0.5f, 1.0 };
	float diffuse0[]  = { 1.0, 1.0, 1.0, 1.0 };
	float specular0[] = { 0.6, 0.6, 0.6, 1.0 };
	float diffuse1[]  = { 0.5, 0.5, 0.5, 1.0 };
	float specular1[] = { 0.5, 0.5, 0.5, 1.0 };
		
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
	
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
		
	glFogi(GL_FOG_MODE, GL_EXP);
	glFogf(GL_FOG_DENSITY, 0.05f);
	//glEnable(GL_FOG);
	
	glLightModelf( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE );
	glLightModelf( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
}


UserAction Game::pause()
{
	enum UserAction action = PAUSE;
	m_levelStatus = LEVEL_PAUSED;
	
	TimerRegistry::Instance()->pause();
				
	/* display pause overlay and current frame */
	SDL_ShowCursor(SDL_ENABLE);
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	Titles::Instance()->drawPauseOverlay();
	glPopAttrib();			
	
	enum pause_events pause_choice = PAUSE_OVERLAY_NO_EVENT;
	while (!pause_choice) 
	{
		Titles::Instance()->checkPauseEvents(&pause_choice);
	}
				
	switch (pause_choice) 
	{			
		case PAUSE_OVERLAY_RESUME_GAME:  //resume
			
			action = NOACTION;
			m_levelStatus = LEVEL_INPROGRESS;
			
			TimerRegistry::Instance()->unpause();
	
			SDL_ShowCursor(SDL_DISABLE);
			break;
			
		case PAUSE_OVERLAY_QUIT_GAME: //quit
			
			m_levelStatus = LEVEL_GAMEOVER;
			break;
			
		default:
			break;
	}		
	
	return action;
}


int Game::enterCode()
{
	enum codeEntry_events request = CODE_ENTRY_SCREEN_NO_EVENT;
	Titles::Instance()->drawCodeEntryScreen(request);
	
	while ( request == CODE_ENTRY_SCREEN_NO_EVENT || request == CODE_ENTRY_SCREEN_INVALID_CODE )
	{
		Titles::Instance()->checkCodeEntryEvents(&request);
	}
	
	switch (request) 
	{
		case CODE_ENTRY_SCREEN_QUIT:  //quit code entry screen back to main titles
			_keyDelay(0.05f);
			return -1;
			
		case CODE_ENTRY_SCREEN_VALID_CODE: //continue to level entry screen
		{
			return 1;
		}
			
		default:
			break;
	}	
	
	return 0;
}

int Game::levelEntry()
{
	Titles::Instance()->drawLevelEntryScreen(m_level);
	enum levelEntry_events request = LEVEL_ENTRY_SCREEN_NO_EVENT;
	
	//small delay to avoid multiple return key detections
	Timer delay;
	delay.start();
	while (delay.time() < 0.5f)
	{
		continue;
	}
	
	while (!request)
	{
		Titles::Instance()->checkLevelEntryEvents(&request);
	}
	
	switch (request) 
	{
		case LEVEL_ENTRY_SCREEN_QUIT:  //quit level entry screen back to code entry screen
			_keyDelay(0.05f);
			return -1;
			
		case LEVEL_ENTRY_SCREEN_CONTINUE: //continue to level
		{
			return 1;
			break;
		}
		
		default:
			break;
	}	
	
	return 0;
}


