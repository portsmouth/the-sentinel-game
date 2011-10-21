

#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "Game_enums.h"

class EventManager {

protected:
	
	// SINGLETON
	EventManager();

private:
	
	static EventManager* _instance;
	
public:	
	
	static EventManager* Instance();	
		
	void setup();
	void doUpdate( UserAction requestedAction, level_status* ls );
	
	void killSentinel();
	void heardSentinelDeathCry();
	
private:
		
	void handlePlayer( UserAction requestedAction, level_status* ls );
	void handleLandscapeObjects();
};

#endif //EVENTMANAGER_H
