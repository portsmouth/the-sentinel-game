

#ifndef TITLES_H
#define TITLES_H

#include "SDL/SDL.h"

#include "TextureLibrary.h"

#include "Timer.h"



typedef struct button 
{
	float x,y,  //top-left corner (screen fractions)
	      dx,dy;//width and height (screen fractions)
	
	bool state; //on or off
} button;

enum button_names {bNULL, bPause_Resume, bPause_Quit};

enum title_state {TITLE_SCREEN, CODE_ENTRY_SCREEN, LEVEL_ENTRY_SCREEN, PAUSE_OVERLAY};

enum title_events { TITLE_SCREEN_NO_EVENT,  
					TITLE_SCREEN_ENTER_CODE,
	                TITLE_SCREEN_PLAY_NOW,
					TITLE_SCREEN_QUIT_GAME };

enum pause_events { PAUSE_OVERLAY_NO_EVENT, 
					PAUSE_OVERLAY_RESUME_GAME, 
					PAUSE_OVERLAY_QUIT_GAME };

enum codeEntry_events { CODE_ENTRY_SCREEN_NO_EVENT,
	                    CODE_ENTRY_SCREEN_VALID_CODE,
	                    CODE_ENTRY_SCREEN_INVALID_CODE,
	                    CODE_ENTRY_SCREEN_QUIT };

enum levelEntry_events { LEVEL_ENTRY_SCREEN_NO_EVENT,
	                     LEVEL_ENTRY_SCREEN_CONTINUE,
                         LEVEL_ENTRY_SCREEN_QUIT };

class Titles 
{

protected:
	
	// SINGLETON
	Titles();	

private:
	
	static Titles* _instance;
	
public:	
	
	static Titles* Instance();

	~Titles(); //destructor
	
	void checkTitleEvents( title_events *request );
	void checkCodeEntryEvents( codeEntry_events *request );
	void checkLevelEntryEvents( levelEntry_events *request );
	void checkPauseEvents( pause_events *request );
	
	int levelChoice();
	
	void drawTitleScreen();
	void drawCodeEntryScreen(enum codeEntry_events request);
	void drawLevelEntryScreen(int levelnum);
	void drawLevelLoadingScreen(int levelnum);
	
	void drawLevelFadeInOverlay(float alpha);
	void drawPauseOverlay();
	void drawGameOverOverlay();
	void drawLevelWonOverlay();
	void drawCodeRevealScreen(int level, float energy);
	void loadAssets();
	
private:
	
	//pause overlay buttons
	button pause_resume;
	button pause_quit;
	
	enum button_names selected_button;
	void Mouse_motion(title_state ts);
	button_names Mouse_button(title_state ts);
	bool checkbutton(button *aButton, float mouse_x, float mouse_y);
	void drawButton(button *aButton);

	void updateLevelChoice( int newDigit );
	void backspaceLevelChoice();
	void updateCodeChoice( int newDigit );
	void backspaceCodeEntry();
	
	void generateCode(int level, int* codeDigits);
	bool validateCode();
	int codeFromDigits(int* codeDigits);
	
	int m_levelChoice[4];
	int m_codeEntered[8];
	
	Timer levelStringEntryTimer;
	Timer codeStringEntryTimer;
	Timer codeInvalidTimer;
	
	int viewdims[2];
	SDL_Surface *image;
	GLuint texture;
	SDL_Event event;
	float m_aspect;

	//textures	
	Texture *mTexture_roundboxEnd;
	Texture *mTexture_roundboxMid;
	Texture *mTexture_roundboxEndOn;
	Texture *mTexture_roundboxMidOn;

};

#endif //TITLES_H


