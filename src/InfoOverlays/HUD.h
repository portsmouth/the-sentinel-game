
#ifndef HUD_H
#define HUD_H

#include "SDL/SDL.h"
#include "Timer.h"
#include "TextureLibrary.h"


class HUD 
{
	
protected:
	
	// SINGLETON
	HUD();

private:
	
	static HUD* _instance;
	
public:	
	
	static HUD* Instance();

	~HUD(); //destructor
	
	void init();
	void loadAssets();
	
	void display( bool paused );
	
private:
	/* texturing */
	Texture *mTexture_banner;
			
};

#endif //HUD_H
