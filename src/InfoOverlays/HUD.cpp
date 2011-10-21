


#include "HUD.h"

#include "Game.h"
#include "World.h"
#include "SDLview.h"
#include "Font.h"
#include "Player.h"
#include "Sentinel.h"

#include "Maths.h"

//headers for texture handling
#include "FileUtils.h"
#include "stdio.h"
#include "stdlib.h"
#include <string>
#include <iostream>
using std::string;
using std::cout;
using std::endl;

using std::string;
using std::cout;
using std::endl;


HUD* HUD::_instance = NULL;

HUD* HUD::Instance()
{
	if (_instance == NULL)
	{
		_instance = new HUD;
	}
	return _instance;
}

HUD::HUD(void) //constructor
{
}


void HUD::init()
{
}


void HUD::display( bool paused )
{	
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	
	int dims[2];
	SDLview::Instance()->getViewDims(dims);	
	float aspect = (float)dims[0]/dims[1];	
	
	//banner
	float dbx = 0.016;
	float HUDfont_height = 0.04;
	float HUDfont_width = HUDfont_height * 0.75 / (aspect / (1152.0/768.0) );
	float lp_y0 = 1.0 - HUDfont_height - dbx*aspect;
	float dx = -0.008;
	float dy = 0.0;	
	float banner_alpha = 0.5;
	float banmarg = 0.25*HUDfont_height;
	float ban_x0 = dbx - banmarg;
	float ban_x1 = 1.0-ban_x0;
	float ban_y0 = lp_y0 - banmarg;
	float ban_y1 = lp_y0 + HUDfont_height + banmarg;
	
	
	glColor4f(1.0,1.0,1.0,banner_alpha);	

	mTexture_banner->Apply();
	glBegin(GL_QUADS);		
	glTexCoord2f(0.0,1.0);
	glVertex3f(ban_x0,ban_y0,0.0);
	glTexCoord2f(1.0,1.0);
	glVertex3f(ban_x1,ban_y0,0.0);
	glTexCoord2f(1.0,0.0);
	glVertex3f(ban_x1,ban_y1,0.0);
	glTexCoord2f(0.0,0.0);
	glVertex3f(ban_x0,ban_y1,0.0);
	glEnd();	
	 
	 
	// font stuff
	glColor4f(1.0,1.0,1.0,1.0);	
	Font *font = Font::Instance();	
	font->SetDimensions(HUDfont_height, HUDfont_width, dx, dy);

	float lives_text_location[2] = {dbx,lp_y0};
	font->Home();
	font->SetCursorLocation(lives_text_location);
	
	Player *player = Player::Instance();
	float energy = player->getEnergy();
	char energy_string[5];
	sprintf(energy_string,"E%3u", (int)energy);
	font->PrintString(energy_string);
	
	//level indicator
	float level_text_location[2] = {0.333-HUDfont_width,lp_y0};
	font->Home();
	font->SetCursorLocation(level_text_location);
	
	int level = Game::Instance()->getLevel();
	char level_string[5];
	sprintf(level_string,"L%d",level);
	font->PrintString(level_string);
		
	
	
	// sentinel draining indicator
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glColor3f(0.9f, 0.9f, 0.9f);
	glLineWidth(2.0f);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);		
	glVertex3f( 0.6*ban_x1, ban_y0 + banmarg, 0.0);
	glVertex3f( ban_x1-banmarg/aspect, ban_y0 + banmarg, 0.0);
	glVertex3f( ban_x1-banmarg/aspect, ban_y1 - banmarg, 0.0);
	glVertex3f( 0.6*ban_x1, ban_y1 - banmarg, 0.0);
	glEnd();	
	
	Sentinel* S = Game::pWorld->pSentinel;
	if ( S && S->getEnemyBehaviourState() == ENEMY_ABSORBING_PLAYER )
	{
		glColor3f(1.0f,0.3f,0.2f);
	}
	else
	{
		glColor3f(0.1f, 0.1f, 0.1f);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);		
	glVertex3f( 0.6*ban_x1, ban_y0 + banmarg, 0.0);
	glVertex3f( ban_x1-banmarg/aspect, ban_y0 + banmarg, 0.0);
	glVertex3f( ban_x1-banmarg/aspect, ban_y1 - banmarg, 0.0);
	glVertex3f( 0.6*ban_x1, ban_y1 - banmarg, 0.0);
	glEnd();	
	
	
	if (!paused)
	{
		float crossX = player->m_crosshairs.m_x;
		float crossY = player->m_crosshairs.m_y;
		
		//draw view crosshairs
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glColor3f(0.0f,0.0f,0.0f);
		glLineWidth(1.0f);
		float hX = 0.02f;
		float hY = hX * aspect;
		
		glBegin(GL_LINES);
		glVertex2f(crossX - hX, crossY);
		glVertex2f(crossX + hX, crossY);
		glVertex2f(crossX, crossY - hY);
		glVertex2f(crossX, crossY + hY);
		glEnd();
	}

}


void HUD::loadAssets()
{
	//at the moment, these are loaded once on running the first game level and used until the app is quit
	TextureLibrary *texturelibrary = TextureLibrary::Instance();
	string resourcepath = FileUtils::GetApplicationResourcesFolderPath();
	string texturepath;

	texturepath = resourcepath + kBanner;
	mTexture_banner = texturelibrary->GetTexture(texturepath, NO_MIPMAPS);
}



