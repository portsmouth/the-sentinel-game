

#ifndef FONT_H
#define FONT_H

#include "SDL/SDL.h"

class Texture;

class Font {
	
protected:
	
	// SINGLETON
	Font();	
	
private:
	
	static Font* _instance;
	
public:	
	
	static Font* Instance();

	void loadAssets();
	void SetDimensions(float height, float width, float dx, float dy);
	void SetCursorLocation(float *location);
	void PrintString(const char *inCString);
	void PrintString_CenterHoriz(const char *inCString);
	void Home();
	
private:

	void loadTextures();
	Texture *mTexture;
	
	float cursor_location[2]; // (0,0) = lower left screen corner, (1,1) = upper right
	int nColumns_fonttexture;
	int nRows_fonttexture;
	float charHeight, charWidth, charSpacing_x, charSpacing_y;
	
	int column, line;
	int linewidth;
	
	void newLine();
	void PrintChar(char inChar);
	void DrawChar(float *location, int inChar);
	void BeginDrawing();
	void EndDrawing();
	
};

#endif //FONT_H
