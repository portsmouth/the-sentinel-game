/*
 *  SDLview.h
 *
 *  Created by Jamie Portsmouth on 14/01/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */



#ifndef SDLVIEW_H
#define SDLVIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include "SDL/SDL.h"
#include <GLUT/glut.h>

class SDLview {
	
protected:
	
	// SINGLETON
	SDLview();	
	
private:
	
	static SDLview* _instance;
	
public:	
	
	static SDLview* Instance();
		
	~SDLview(); //destructor
	void OpenGL_Reshape(int width, int height);
	void OpenGL_Init(void);	

	double mtime(void);
	double filter(double in, double *save);
	void DrawStr(const char *str);
	void printAttributes ();
	void createSurface ();

	void drawFPS(void);
	void getViewDims(int *dims);
	
private:
		
	SDL_Surface *gScreen;
	GLboolean Timing;
	int w_win;
	int h_win;
	GLint count;
	GLenum StrMode;
	GLboolean moving;
	
};

#endif //SDLVIEW_H
