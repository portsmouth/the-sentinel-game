

#include "Font.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "Maths.h"

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#include "FileUtils.h"
#include "TextureLibrary.h"
#include <string>
#include <iostream>
using std::string;
using std::cout;
using std::endl;

Font* Font::_instance = NULL;

Font* Font::Instance()
{
	if (_instance == NULL)
	{
		_instance = new Font;
	}
	return _instance;
}


Font::Font()//constructor
{			
	nColumns_fonttexture = 8;
	nRows_fonttexture = 8;
	charHeight = 0.06; //in fractions of a screen height
	charWidth = 0.04;
	charSpacing_x = 0.00;  //additional (or reduced, if negative) spacing between characters
	charSpacing_y = 0.00;
	
	linewidth = 18; //max number of characters per line before line break
}



void Font::SetDimensions(float height, float width, float dx, float dy)
{
	charHeight = height;
	charWidth = width;
	charSpacing_x = dx;
	charSpacing_y = dy;
}

void Font::loadAssets()
{
	loadTextures();
} 

void Font::loadTextures()
{
	TextureLibrary *texturelibrary = TextureLibrary::Instance();
	string resourcepath = FileUtils::GetApplicationResourcesFolderPath();
	string font_texture;
	
	font_texture = kFontGrid;		
	mTexture = texturelibrary->GetTexture(resourcepath + font_texture, NO_MIPMAPS);
}

// (0,0) = lower left screen corner, (1,1) = upper right
void Font::SetCursorLocation(float *location)
{
	for (int a=0;a<2;a++)
		cursor_location[a] = location[a];
}

/* draw text centred horizontally on screen. Vertical position and dimensions must have 
   been set prior to calling this. */
void Font::PrintString_CenterHoriz(const char *inCString)
{
	size_t StringLen = strlen(inCString);	
	float location[2];
	location[0] = 0.5 - 0.5*charWidth*(float)StringLen;
	location[1] = cursor_location[1];
	SetCursorLocation(location);
	
	BeginDrawing();
	while(*inCString != 0) 
		PrintChar(*inCString++);
	
	EndDrawing();	
}

/*	Uses PrintChar() to print a complete C-string */
void Font::PrintString(const char *inCString)
{
	BeginDrawing();
	while(*inCString != 0) 
		PrintChar(*inCString++);

	EndDrawing();
}

void Font::PrintChar(char inChar)
{	
	// Work out where the character goes:	
	float char_location[2];				
	char_location[0] = cursor_location[0] + (charWidth + charSpacing_x)*(float)column;
	char_location[1] = cursor_location[1] - (charHeight - charSpacing_y)*(float)line;
			
	int character = int(inChar);
	if(character >= 0 and character <= 255)
	{
		DrawChar(char_location, character);
	}
	
	// Advance to the next column, and if necessary, start a new line:	
	column++;
	if(linewidth > 0 and column > linewidth) newLine();	
}

void Font::newLine()
{
	column = 0;
	line++;
}

void Font::DrawChar(float *location, int inChar)
{	
	inChar -= 32; //we begin at ASCII 32
	
	float edge = 0.05; //fraction of a character width or height to exclude around edge of identified grid square
	
	float gridx = 1.0 / (float)nColumns_fonttexture;
	float gridy = 1.0 / (float)nRows_fonttexture;

	float dchar_u = edge*gridx;
	float dchar_v = edge*gridy;	
	float char_u = gridx - 2.0*dchar_u;
	float char_v = gridy - 2.0*dchar_v;

	float u = float((unsigned char)(inChar) % nColumns_fonttexture) * gridx + dchar_u;
	float v = float((unsigned char)(inChar) / nRows_fonttexture)    * gridy + dchar_v;
			
	glTexCoord2f(u, v + char_v);
	glVertex3f(location[0], location[1], 0.0);
	
	glTexCoord2f(u + char_u, v + char_v);
	glVertex3f(location[0] + charWidth, location[1], 0.0);
	
	glTexCoord2f(u + char_u, v);
	glVertex3f(location[0] + charWidth, location[1] + charHeight, 0.0);
	
	glTexCoord2f(u, v);
	glVertex3f(location[0], location[1] + charHeight, 0.0);
	 
}

void Font::BeginDrawing()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
	mTexture->Apply();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glBegin(GL_QUADS);	
}

void Font::EndDrawing()
{
	glEnd();
	
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);	
	glDisable(GL_BLEND);
}

/*	Moves the cursor back to its initial position. */
void Font::Home()
{
	column = 0;
	line = 0;
}

