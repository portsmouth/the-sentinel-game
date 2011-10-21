
#include "EventManager.h"

#include "Player.h"
#include "SDLview.h"
#include "Game.h"
#include "World.h"
#include "terrain.h"
#include "Sentinel.h"

#include <set>
using std::set;


EventManager* EventManager::_instance = NULL;

EventManager* EventManager::Instance()
{
	if (_instance == NULL)
	{
		_instance = new EventManager;
	}
	return _instance;
}


EventManager::EventManager() 
{}


/* setup game state at the beginning of each level */
void EventManager::setup()
{
}

/* do a game update and return the updated level status */
void EventManager::doUpdate( UserAction requestedAction, level_status* ls )
{	
	handlePlayer(requestedAction, ls);
	handleLandscapeObjects();
}

void EventManager::handlePlayer( UserAction requestedAction, level_status* ls )
{	
	switch ( Player::Instance()->GetState() ) 
	{
		//normal in-level play
		case PLAYER_ACTIVE: 
		
			Player::Instance()->handleCrosshairs();
			Player::Instance()->raycastTerrain();
			Player::Instance()->performRequestedUserAction(requestedAction);
			break;
		
		//sentinel killed
		case PLAYER_DEFEATED_SENTINEL: 
			
			Player::Instance()->raycastTerrain();
			Player::Instance()->performRequestedUserAction(requestedAction);
			break;	
			
		//player has hyperspaced after killing sentinel, thus winning the level
		case PLAYER_WINS_LEVEL:
			
			if (Player::Instance()->finishedWinning())
			{
				*ls = LEVEL_ADVANCE;
			}
			break;
		
			
		//player has been killed
		case PLAYER_KILLED: 
			
			if (Player::Instance()->finishedDying()) 
			{
				*ls = LEVEL_GAMEOVER;
			}
			break;
			
		case PLAYER_TRANSFERRING:
			
			Player::Instance()->continueTransfer();
			break;
	}
}

void EventManager::handleLandscapeObjects()
{
	// removes deleted objects, does AI update for enemies, etc.
	Game::pWorld->pTerrain->handleOccupants();
}

void EventManager::killSentinel()
{
	Sentinel* S = Game::pWorld->pSentinel;
	if (S)
	{
		S->kill();
	}
}

void EventManager::heardSentinelDeathCry()
{
	Player::Instance()->SetState( (Player_States)PLAYER_DEFEATED_SENTINEL );
	Game::pWorld->pSentinel = NULL;
}





