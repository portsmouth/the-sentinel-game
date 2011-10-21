
#ifndef GAME_H
#define GAME_H

#ifdef DEBUG
#include <cassert>
/* #define NDEBUG */ //uncomment to strip assert() calls
#endif

#include "Game_enums.h"

class World;

class Game {
	
protected:
	
	// SINGLETON
	Game();
	
private:
	
	static Game* _instance;
	
public:	
	
	static Game* Instance();
	
	void mainLoop();
	int getLevel();
	void drawFrame();

	// static pointer to the current level's World
	static class World* pWorld;
	
private:
		
	enum UserAction update();
	enum UserAction pause();
	
	void drawInfo(enum Player_States playerState);
	void gameGLsettings();
	void placeLights();
	
	int enterCode();
	int levelEntry();
	void playLevel();
	
	enum level_status m_levelStatus;
	int m_level;
		
	class Timer* LFT; //LastFrameTimer
	class Timer* LET; //LevelEntryTimer
};

#endif //GAME_H
