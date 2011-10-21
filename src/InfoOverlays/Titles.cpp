

#include "Titles.h"

#include "Game.h"
#include "Font.h"
#include "SDLview.h"

#include "FileUtils.h"
#include <string>
#include <iostream>
using std::string;
using std::cout;
using std::endl;

Titles* Titles::_instance = NULL;

Titles* Titles::Instance()
{
	if (_instance == NULL)
	{
		_instance = new Titles;
	}
	return _instance;
}

Titles::Titles() //constructor
{					
	/* pause overlay buttons */
	float button_width = 0.3;
	float button_height = 0.055;
	float xflush = 0.5*(1.0-button_width);
	float button_top = 0.6;
	float button_space = 0.08;
		
	pause_resume.dx = button_width;
	pause_resume.dy = button_height;
	pause_resume.x = xflush;
	pause_resume.y = button_top - 0.0*button_space;
	pause_resume.state = false;
	
	pause_quit.dx = button_width;
	pause_quit.dy = button_height;
	pause_quit.x = xflush;
	pause_quit.y = button_top - 1.0*button_space;
	pause_quit.state = false;
		
	for (int i=0; i<4; ++i) m_levelChoice[i] = 0;
	for (int i=0; i<8; ++i) m_codeEntered[i] = 0;
	
	levelStringEntryTimer.start();
	codeStringEntryTimer.start();
}


void Titles::loadAssets()
{
	TextureLibrary *texturelibrary = TextureLibrary::Instance();
	string resourcepath = FileUtils::GetApplicationResourcesFolderPath();
	
	mTexture_roundboxEnd   = texturelibrary->GetTexture(resourcepath + kRoundBoxEnd, NO_MIPMAPS);
	mTexture_roundboxMid   = texturelibrary->GetTexture(resourcepath + kRoundBoxMid, NO_MIPMAPS);
	mTexture_roundboxEndOn = texturelibrary->GetTexture(resourcepath + kRoundBoxEndOn, NO_MIPMAPS);
	mTexture_roundboxMidOn = texturelibrary->GetTexture(resourcepath + kRoundBoxMidOn, NO_MIPMAPS);
	
	int dims[2];
	SDLview::Instance()->getViewDims(dims);	
	m_aspect = (float)dims[0]/dims[1];
}

Titles::~Titles() //destructor
{}


void Titles::drawTitleScreen()
{	
	static float bgcolor[] = {0.0, 0.0, 0.0, 1.0};
	glClearColor(bgcolor[0],bgcolor[1],bgcolor[2],bgcolor[3]);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	Font *font = Font::Instance();
	
	float Titlefont_height = 0.03;
	float Titlefont_width = 0.02;
	float titledx = 0.0;
	float titledy = 0.0;	
	font->SetDimensions(Titlefont_height, Titlefont_width, titledx, titledy);
	
	char title_string[20];
	float text_location[2];
		
	//title string
	text_location[1] = 0.6;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(title_string,  "LANDSCAPE NUMBER");
	font->PrintString_CenterHoriz(title_string);
	
	//level string
	char level_string[12];
	text_location[1] = 0.5;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(level_string,  "%d%d%d%d", m_levelChoice[0], m_levelChoice[1], m_levelChoice[2], m_levelChoice[3]);
	font->PrintString_CenterHoriz(level_string);
	
	SDL_GL_SwapBuffers ();
}


void Titles::drawCodeEntryScreen(enum codeEntry_events request)
{
	static float bgcolor[] = {0.0, 0.0, 0.0, 1.0};
	glClearColor(bgcolor[0],bgcolor[1],bgcolor[2],bgcolor[3]);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	Font *font = Font::Instance();
	
	float Titlefont_height = 0.03;
	float Titlefont_width = 0.666 * Titlefont_height;
	float titledx = 0.0;
	float titledy = 0.0;	
	font->SetDimensions(Titlefont_height, Titlefont_width, titledx, titledy);
	
	char title_string[20];
	char code_string[12];
	float text_location[2];
	
	//title string
	text_location[1] = 0.3;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(title_string,  "CODE FOR LEVEL %d", levelChoice());
	font->PrintString_CenterHoriz(title_string);
	
	//code string
	text_location[1] = 0.6;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(code_string,  "%d%d%d%d%d%d%d%d", m_codeEntered[0], m_codeEntered[1], m_codeEntered[2], m_codeEntered[3],
											  m_codeEntered[4], m_codeEntered[5], m_codeEntered[6], m_codeEntered[7]);
	font->PrintString_CenterHoriz(code_string);
		
	if (request == CODE_ENTRY_SCREEN_INVALID_CODE)
	{
		text_location[1] = 0.8;
		font->Home();
		font->SetCursorLocation(text_location);	
		font->PrintString_CenterHoriz("WRONG CODE");
	}
	
	SDL_GL_SwapBuffers ();	
}



void Titles::checkLevelEntryEvents( levelEntry_events *request )
{
	while ( SDL_PollEvent (&event) ) 
	{
		switch (event.type) 
		{			
			case SDL_KEYDOWN:	
			{
				// display entered digits
				int key = event.key.keysym.sym;
								
				if (key == SDLK_ESCAPE)
				{
					*request = LEVEL_ENTRY_SCREEN_QUIT;
					break;
				}
				
				if (key == SDLK_RETURN)
				{
					*request = LEVEL_ENTRY_SCREEN_CONTINUE;
					break;
				}
			}
				
			default:
				break;
		}
	}	
}


void Titles::drawLevelEntryScreen(int levelnum)
{
	static float bgcolor[] = {0.0, 0.0, 0.0, 1.0};
	glClearColor(bgcolor[0],bgcolor[1],bgcolor[2],bgcolor[3]);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	Font *font = Font::Instance();
	
	float Titlefont_height = 0.03;
	float Titlefont_width = 0.666 * Titlefont_height;
	float titledx = 0.0;
	float titledy = 0.0;	
	font->SetDimensions(Titlefont_height, Titlefont_width, titledx, titledy);
	
	char title_string[20];
	float text_location[2];
	
	//title string
	text_location[1] = 0.3;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(title_string,  "LEVEL %d", levelnum);
	font->PrintString_CenterHoriz(title_string);
		
	SDL_GL_SwapBuffers ();	
}


// (x,y) is top-left corner, (w,h) are the width and height

const float t0 = 0.01f; 
const float t1 = 0.99f; 

inline void drawButtonQuad( float x, float y, float w, float h, bool flipX )
{
	glBegin(GL_QUADS);	
	
	float u[4], v[4];
	
	if (!flipX)
	{
		float U[4] = { t0, t1, t1, t0 };
		float V[4] = { t1, t1, t0, t0 };
		for (int i=0; i<4; ++i) { u[i] = U[i]; v[i] = V[i]; }
	}
	else
	{
		float U[4] = { t1, t0, t0, t1 };
		float V[4] = { t1, t1, t0, t0 };
		for (int i=0; i<4; ++i) { u[i] = U[i]; v[i] = V[i]; }
	}
		
	glTexCoord2f(u[0], v[0]);
	glVertex2f(x,y);
	
	glTexCoord2f(u[1], v[1]);
	glVertex2f(x+w, y);
	
	glTexCoord2f(u[2], v[2]);
	glVertex2f(x+w, y-h);
	
	glTexCoord2f(u[3], v[3]);
	glVertex2f(x, y-h);
	
	glEnd();
}

const float crack_paste = 1.0e-2f;

void Titles::drawButton(button *aButton)
{
	float W = aButton->dx;
	float H = aButton->dy;
	float X = aButton->x;
	float Y = aButton->y;
	
	// adjacent quads seen to have a pixel gap for some reason.  Kludge to hide it:
	float Hp = H * (1.f + crack_paste) / m_aspect;
	
	if (aButton->state) 
	{
		mTexture_roundboxEndOn->Apply();
	}
	else
	{
		mTexture_roundboxEnd->Apply();
	}
	
	drawButtonQuad( X, Y, Hp, H, false );
	drawButtonQuad( X+W-Hp, Y, Hp, H, true );
	
	if (aButton->state) 
	{
		mTexture_roundboxMidOn->Apply();
	}
	else
	{
		mTexture_roundboxMid->Apply();
	}
	
	drawButtonQuad( X+Hp, Y, W-2.f*Hp, H, false );
}


int Titles::levelChoice()
{
	int level = 0;
	
	level += m_levelChoice[3];
	level += m_levelChoice[2] * 10;
	level += m_levelChoice[1] * 100;
	level += m_levelChoice[0] * 1000;
	
	return level;
}

int Titles::codeFromDigits(int* codeDigits)
{
	int code=0;
	
	code += codeDigits[7];
	code += codeDigits[6] * 10;
	code += codeDigits[5] * 100;
	code += codeDigits[4] * 1000;
	code += codeDigits[3] * 10000;
	code += codeDigits[2] * 100000;
	code += codeDigits[1] * 1000000;
	code += codeDigits[0] * 10000000;
	
	return code;
}


void Titles::updateLevelChoice( int newDigit )
{
	int n=0;
	while (n<3)
	{
		m_levelChoice[n] = m_levelChoice[n+1];
		n++;
	}
	
	m_levelChoice[3] = newDigit;
}


void Titles::backspaceLevelChoice()
{
	int n=3;
	
	while (n>0)
	{
		m_levelChoice[n] = m_levelChoice[n-1];
		n--;
	}
	
	m_levelChoice[0] = 0;
}


const float TEXT_ENTRY_DELAY = 0.15f;

void Titles::checkTitleEvents( title_events *request )
{	
	while ( SDL_PollEvent (&event) ) 
	{
		switch (event.type)
		{			
			case SDL_KEYDOWN:	
			{
				if (levelStringEntryTimer.time() > TEXT_ENTRY_DELAY)
				{
					// display entered digits
					int key = event.key.keysym.sym;
					
					if (key >= SDLK_0 && key <= SDLK_9)
					{
						updateLevelChoice(key-SDLK_0);
						levelStringEntryTimer.start();
						drawTitleScreen();
						break;
					}
					
					else if (key == SDLK_BACKSPACE)
					{
						backspaceLevelChoice();
						levelStringEntryTimer.start();
						drawTitleScreen();
						break;
					}
					
					else if (key == SDLK_ESCAPE)
					{
						*request = TITLE_SCREEN_QUIT_GAME;
						break;
					}
					
					else if (key == SDLK_RETURN)
					{
						// if level selected was 0000, play the first level
						if (levelChoice() == 0)
						{
							*request = TITLE_SCREEN_PLAY_NOW;
						}
						
						//otherwise, go to the code entry screen
						else
						{
							*request = TITLE_SCREEN_ENTER_CODE;
						}
						break;
					}
				}
			}
			break;
				
			case SDL_QUIT:
				*request = TITLE_SCREEN_QUIT_GAME;
				break;
				
			default:
				break;
		}
	}	
}

void Titles::generateCode(int level, int* codeDigits)
{
	// TO DO: good random number method for int -> 8 digit code of digits [0-9]
	
	level *= 89714837;  // makes level negative for level > about 20, crap
	level += 13472810;
	level %= 100000000;

	int f = 10000000;
	for (int n=0; n<8; ++n)
	{
		codeDigits[n] = level/f;
		level -= codeDigits[n] * f;
		f /= 10;
	}
	
	const int pad[8] = {7,6,6,4,1,0,9,5};
	
	for (int n=0; n<8; ++n)
	{
		codeDigits[n] += pad[n];
		codeDigits[n] %= 10;
	}
}

bool Titles::validateCode()
{
	int trueCodeDigits[8];
	generateCode(levelChoice(), trueCodeDigits);
	int trueCode = codeFromDigits(trueCodeDigits);
	
	int enteredCode = codeFromDigits(m_codeEntered);
	return ( enteredCode == trueCode );
}

void Titles::updateCodeChoice( int newDigit )
{
	int n=0;
	while (n<7)
	{
		m_codeEntered[n] = m_codeEntered[n+1];
		n++;
	}
	
	m_codeEntered[7] = newDigit;
}

void Titles::backspaceCodeEntry()
{
	int n=7;
	
	while (n>0)
	{
		m_codeEntered[n] = m_codeEntered[n-1];
		n--;
	}
	
	m_codeEntered[0] = 0;
}

const float CODE_INVALID_WARNING_DISPLAY_TIME = 2.0f;

void Titles::checkCodeEntryEvents( codeEntry_events *request )
{	
	static bool warningDisplayed = false;
	
	if ( *request == CODE_ENTRY_SCREEN_INVALID_CODE )
	{
		if (codeInvalidTimer.time() < CODE_INVALID_WARNING_DISPLAY_TIME)
		{
			if (!warningDisplayed)
			{
				drawCodeEntryScreen(*request);
				warningDisplayed = true;
			}
		}
		else
		{
			warningDisplayed = false;
			*request = CODE_ENTRY_SCREEN_QUIT;
		}
		
		return;
	}
	
	while ( SDL_PollEvent (&event) ) 
	{
		switch (event.type) 
		{			
			case SDL_KEYDOWN:	
			{
				if (codeStringEntryTimer.time() > TEXT_ENTRY_DELAY)
				{
					// display entered digits
					int key = event.key.keysym.sym;
					
					if (key >= SDLK_0 && key <= SDLK_9)
					{
						updateCodeChoice(key-SDLK_0);
						codeStringEntryTimer.start();
						drawCodeEntryScreen(*request);
					}
					
					else if (key == SDLK_BACKSPACE)
					{
						backspaceCodeEntry();
						codeStringEntryTimer.start();
						drawCodeEntryScreen(*request);
						break;
					}
					
					else if (key == SDLK_RETURN)
					{
						if ( true ) // ( validateCode() )
						{
							// TO DO: allow player to re-enter level without code entry
							// after the first successful code entry (later, perhaps valid
							// codes can be stored in the user preferences on disk).
							
							*request = CODE_ENTRY_SCREEN_VALID_CODE;
						}
						else
						{
							*request = CODE_ENTRY_SCREEN_INVALID_CODE;
							codeInvalidTimer.start();
						}
						break;
					}
			
					else if (key == SDLK_ESCAPE)
					{
						*request = CODE_ENTRY_SCREEN_QUIT;
						
						// introduce a delay so escape key does not get pressed in title screen also
						Timer delay;
						delay.start();
						while (delay.time() < 0.2f)
						{
							continue;
						}
						break;
					}
				}
			}
				
			default:
				break;
		}
	}		
}

button_names Titles::Mouse_button(title_state ts)
{
	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	
	float mx, my;
	mx = (float)mouse_x/viewdims[0];
	my = 1.0-(float)mouse_y/viewdims[1];
			
	switch (ts) 
	{			
		case PAUSE_OVERLAY:
			
			if (checkbutton(&pause_resume, mx, my)) 
				return bPause_Resume;
			 
			else if (checkbutton(&pause_quit, mx, my)) 
				return bPause_Quit;
			
			break;
			
		default:
			break;
	}
				
	return bNULL;
}


void Titles::Mouse_motion(title_state ts)
{
	int mouse_x, mouse_y;
	(Uint8)SDL_GetMouseState(&mouse_x, &mouse_y);	
	
	float mx, my;
	mx = (float)mouse_x/viewdims[0];
	my = 1.0-(float)mouse_y/viewdims[1];
		
	switch (ts) 
	{			
		case PAUSE_OVERLAY:
			
			pause_resume.state = checkbutton(&pause_resume, mx, my);
			pause_quit.state = checkbutton(&pause_quit, mx, my);
			drawPauseOverlay();		
			break;
			
		default:
			break;
	}
}

bool Titles::checkbutton(button *aButton, float mouse_x, float mouse_y)
{
	float x  = aButton ->x;
	float dx = aButton ->dx;
	float y  = aButton ->y;
	float dy = aButton ->dy;
	
	bool left  = ( mouse_x > x    );
	bool right = ( mouse_x < x+dx );
	bool up    = ( mouse_y < y    );
	bool down  = ( mouse_y > y-dy );
	
	if (left && right && up && down) 
		return true;
	
	return false;
}

void Titles::drawLevelLoadingScreen(int levelnum)
{
	static float bgcolor[] = {0.0, 0.0, 0.0, 1.0};
	glClearColor(bgcolor[0],bgcolor[1],bgcolor[2],bgcolor[3]);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	Font *font = Font::Instance();
	
	float Titlefont_height = 0.04;
	float Titlefont_width = 0.03;
	float titledx = 0.0;
	float titledy = 0.0;	
	font->SetDimensions(Titlefont_height, Titlefont_width, titledx, titledy);
	
	char title_string[16];
	float text_location[2];
	
	font->Home();
	text_location[1] = 0.5;
	font->SetCursorLocation(text_location);	
	sprintf(title_string,"LEVEL %d LOADING",levelnum);
	font->PrintString_CenterHoriz(title_string);
	
	SDL_GL_SwapBuffers ();

}

void Titles::drawLevelFadeInOverlay(float alpha)
{
	if (alpha<0.0) return;
	
	glColor4f(0.0,0.0,0.0,alpha);
	
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glBegin(GL_QUADS);		
	glVertex2f(0.0,0.0);
	glVertex2f(1.0,0.0);
	glVertex2f(1.0,1.0);
	glVertex2f(0.0,1.0);
	glEnd();
	
	glPopMatrix();	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();		
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);	
	glDisable(GL_BLEND);
	
	glColor4f(1.0,1.0,1.0,1.0);
}

void Titles::drawPauseOverlay()
{
	SDLview::Instance()->getViewDims(viewdims);	
	Game::Instance()->drawFrame();

	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glColor4f(1.0,1.0,1.0,0.6);
	drawButton(&pause_resume);
	drawButton(&pause_quit);
	glColor4f(1.0,1.0,1.0,1.0);
	
	char title_string[12];
	float text_location[2];
	Font *font = Font::Instance();
	
	float buttonborder = 0.5;
	float glyphfactor = 1.0-buttonborder;
	font->SetDimensions(glyphfactor*pause_resume.dy, 
						glyphfactor*0.666*pause_resume.dy, 
						0.0, 0.0);
		
	text_location[1] = pause_resume.y-(1.0-0.5*buttonborder)*pause_resume.dy;
	font->SetCursorLocation(text_location);		
	
	font->Home();
	sprintf(title_string,"RESUME");
	font->PrintString_CenterHoriz(title_string);
	
	text_location[1] = pause_quit.y-(1.0-0.5*buttonborder)*pause_quit.dy;
	font->Home();
	font->SetCursorLocation(text_location);
	sprintf(title_string,"QUIT");
	font->PrintString_CenterHoriz(title_string);
			
	SDL_GL_SwapBuffers ();
}


void Titles::checkPauseEvents( pause_events *request )
{
	SDL_PumpEvents();
	
	while ( SDL_PollEvent (&event) ) 
	{
		switch (event.type) 
		{			
			case SDL_MOUSEMOTION:
				Mouse_motion(PAUSE_OVERLAY);
				break;
				
			case SDL_MOUSEBUTTONDOWN:
			{
				if (event.button.button != SDL_BUTTON(1))
				{
					*request = PAUSE_OVERLAY_NO_EVENT;
					break;
				}
				
				switch (Mouse_button(PAUSE_OVERLAY)) 
				{
					case bNULL:						
						*request = PAUSE_OVERLAY_NO_EVENT;
						break;
						
					case bPause_Resume:
						*request = PAUSE_OVERLAY_RESUME_GAME;
						break;
						
					case bPause_Quit:
						*request = PAUSE_OVERLAY_QUIT_GAME;
						break;
						
					default:
						break;
				}				
				break;
			}
				
			default:
				break;
		}
	}		
	
}


void Titles::drawGameOverOverlay()
{
	SDLview::Instance()->getViewDims(viewdims);	
	
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	float height = 0.06;
	float top = 0.5;
	
	char title_string[12];
	float text_location[2];
	Font *font = Font::Instance();
	font->SetDimensions(height, 0.666*height, 0.0, 0.0);
	
	text_location[1] = top;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(title_string,"GAME OVER");
	font->PrintString_CenterHoriz(title_string);
}


void Titles::drawLevelWonOverlay()
{
	SDLview::Instance()->getViewDims(viewdims);	
	
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	
	float height = 0.06;
	float top = 0.5;
	
	char title_string[12];
	float text_location[2];
	Font *font = Font::Instance();
	font->SetDimensions(height, 0.666*height, 0.0, 0.0);
	
	text_location[1] = top;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(title_string,"WELL DONE");
	font->PrintString_CenterHoriz(title_string);	
}


static void _revealCode(Font *font, int level, int* codeDigits)
{
	char title_string[20];
	char code_string[12];
	float text_location[2];
	
	//title string
	text_location[1] = 0.3;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(title_string,  "CODE FOR LEVEL %d", level);
	font->PrintString_CenterHoriz(title_string);
	
	//code string
	text_location[1] = 0.6;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(code_string,  "%d%d%d%d%d%d%d%d", codeDigits[0], codeDigits[1], codeDigits[2], codeDigits[3],
			codeDigits[4], codeDigits[5], codeDigits[6], codeDigits[7]);
	font->PrintString_CenterHoriz(code_string);	
}

void Titles::drawCodeRevealScreen(int level, float energy)
{
	static float bgcolor[] = {0.0, 0.0, 0.0, 1.0};
	glClearColor(bgcolor[0],bgcolor[1],bgcolor[2],bgcolor[3]);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	Font *font = Font::Instance();
	
	float Titlefont_height = 0.06;
	float Titlefont_width = 0.666 * Titlefont_height;
	float titledx = 0.0;
	float titledy = 0.0;	
	font->SetDimensions(Titlefont_height, Titlefont_width, titledx, titledy);
	
	int newLevel = level + energy;
	int codeDigits[8];
	generateCode(newLevel, codeDigits);
	
	_revealCode(font, newLevel, codeDigits);
	SDL_GL_SwapBuffers ();	
	
	// wait a mandatory amount of time so the code is seen	
	Timer delay;
	delay.start();
	while ( delay.time() < 5.0f )
	{
		_revealCode(font, newLevel, codeDigits);
		SDL_GL_SwapBuffers ();	
	}
	
	//indication that the mandatory waiting period is over
	_revealCode(font, newLevel, codeDigits);
	
	char msg_string[20];
	float text_location[2];
	text_location[1] = 0.8;
	font->Home();
	font->SetCursorLocation(text_location);	
	sprintf(msg_string,  "...");
	font->PrintString_CenterHoriz(msg_string);
	SDL_GL_SwapBuffers ();		
	
	// check for enter key, after which we go back to titles
	SDL_PumpEvents();
	SDL_PollEvent (&event);
	
	while (1) 
	{
		SDL_PollEvent (&event);
		
		if (event.type == SDL_KEYDOWN)	
		{
			// display entered digits
			int key = event.key.keysym.sym;
			
			if (key == SDLK_RETURN && delay.time() > 6.0f)
			{
				return;
			}
		}
	}
}





