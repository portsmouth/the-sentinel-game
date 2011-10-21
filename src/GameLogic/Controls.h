
#ifndef CONTROLS_H
#define CONTROLS_H

#include "Game_enums.h"

enum MouseButtonState {UP, DOWN};

class Player;

class Controls {
	
protected:
	
	// SINGLETON
	Controls();	

private:
	
	static Controls* _instance;
	
public:	
	
	static Controls* Instance();	
		
	UserAction checkUserInput(Player *pPlayer);
	
	void Look(Player *pPlayer);

};

#endif //CONTROLS_H

