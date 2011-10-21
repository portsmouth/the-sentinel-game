

#include "SDLview.h"
#include <OpenGL/OpenGL.h>

// uncomment for windowed mode
#define WINDOWED


SDLview* SDLview::_instance = NULL;

SDLview* SDLview::Instance()
{
	if (_instance == NULL)
	{
		_instance = new SDLview;
	}
	return _instance;
}

//constructor
SDLview::SDLview()
{
	// Init SDL video subsystem
	if ( SDL_Init (SDL_INIT_VIDEO) < 0 ) {
		
        fprintf(stderr, "Couldn't initialize SDL video: %s\n",
				SDL_GetError());
		exit(1);
	}
	
	// Init SDL audio subsystem
	if ( SDL_Init (SDL_INIT_AUDIO) < 0 )
	{
		
        fprintf(stderr, "Couldn't initialize SDL audio: %s\n",
				SDL_GetError());
		exit(1);
	}
	
	// Set GL context attributes
	OpenGL_Init();

	// Create GL context
	createSurface();
		        
    // Get GL context attributes
    //printAttributes ();
    
	Timing = GL_TRUE;
	count = 0;
	StrMode = GL_VENDOR;

	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableKeyRepeat(1,1);
	
	SDL_WM_GrabInput(SDL_GRAB_ON);					
	SDL_WM_SetCaption("The Sentinel", "The Sentinel");
	SDL_WarpMouse(int(w_win / 2.0), int(h_win / 2.0));

}

//destructor
SDLview::~SDLview()
{
}

void SDLview::createSurface()
{
	Uint32 flags = 0;
	
	int width, height;
    
	flags = SDL_OPENGL;
	flags |= SDL_HWSURFACE;
	flags |= SDL_GL_ACCELERATED_VISUAL;

#ifndef WINDOWED
	{
        flags |= SDL_FULLSCREEN;
		width = 1920;
		height = 1200;
    }
#else	
	{
		width = 1100;
		height = 750;
	}
#endif //WINDOWED
	
	gScreen = SDL_SetVideoMode (width, height, 0, flags);
	
    // Create window
    if (!gScreen) 
	{
        fprintf (stderr, "Unable to create video surface that supports AA;  trying without AA\n");
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
		gScreen = SDL_SetVideoMode (width, height, 0, flags);

		if (!gScreen)
        {
	    	fprintf (stderr, "Could not set SDL video mode: %s\n", SDL_GetError());
			exit(1);
        }		
	}	
	
	
	//sync vertical scan (Mac specific call)
#ifdef __APPLE__
	const GLint VBLState = 1;
	CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &VBLState);	
#endif
	
	OpenGL_Reshape (gScreen->w, gScreen->h);
	
}

void SDLview::OpenGL_Init(void)
{
	// Setup attributes we want for the OpenGL context
    int value;
    
    // Don't set color bit sizes (SDL_GL_RED_SIZE, etc)
    //    Mac OS X will always use 8-8-8-8 ARGB for 32-bit screens and
    //    5-5-5 RGB for 16-bit screens
    
    // Request a 16-bit depth buffer (without this, there is no depth buffer)
    value = 16;
    SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, value);
	SDL_GL_SetAttribute (SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute (SDL_GL_MULTISAMPLESAMPLES, 8);	
    
	// Request double-buffered OpenGL
    //     The fact that windows are double-buffered on Mac OS X has no effect
    //     on OpenGL double buffering.
    value = 1;
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, value);
}

void SDLview::OpenGL_Reshape(int width, int height)
{
	w_win = width;
	h_win = height;
    glViewport(0, 0, width, height);
}

void SDLview::getViewDims(int *dims)
{	
	dims[0] = w_win;
	dims[1] = h_win;
	return;
}

double SDLview::mtime(void) 
{
	struct timeval tk_time;
	struct timezone tz;
	
	gettimeofday(&tk_time, &tz);
	
	return 4294.967296 * tk_time.tv_sec + 0.000001 * tk_time.tv_usec;
}

double SDLview::filter(double in, double *save)
{
	static double k1 = 0.9;
	static double k2 = 0.05;
	
	save[3] = in;
	save[1] = save[0]*k1 + k2*(save[3] + save[2]);
	
	save[0]=save[1];
	save[2]=save[3];
	
	return(save[1]);
	
}

void SDLview::DrawStr(const char *str)
{
	GLint i = 0;
	
	if(!str) return;
	
	while(str[i])
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
		i++;
	}
	
}

void SDLview::printAttributes ()
{
	// Print out attributes of the context we created
    int nAttr;
    int i;
    
    int  attr[] = { SDL_GL_RED_SIZE,   SDL_GL_BLUE_SIZE,   SDL_GL_GREEN_SIZE,
		            SDL_GL_ALPHA_SIZE, SDL_GL_BUFFER_SIZE, SDL_GL_DEPTH_SIZE };
	
    char *desc[] = { "Red size: %d bits\n", "Blue size: %d bits\n", "Green size: %d bits\n",
		"Alpha size: %d bits\n", "Color buffer size: %d bits\n", 
		"Depth bufer size: %d bits\n" };
	
    nAttr = sizeof(attr) / sizeof(int);
    
    for (i = 0; i < nAttr; i++) {
		
        int value;
        SDL_GL_GetAttribute ((SDL_GLattr)attr[i], &value);
        printf (desc[i], value);
    } 	
}


void SDLview::drawFPS(void)
{
	static double th[4] = {0.0, 0.0, 0.0, 0.0};
	static double t1 = 0.0, t2 = 0.0, t;
	char num_str[128];
    
    t1 = t2;
	if(Timing)
    {
		t2 = mtime();
		t = t2 - t1;
		if(t > 0.0001) t = 1.0 / t;
		
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		
		glColor3f(1.0, 1.0, 1.0);
		
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, w_win, 0, h_win, -10.0, 10.0);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glRasterPos2f(w_win-100.0, 15.0);

		sprintf(num_str, "%0.2f FPS", filter(t, th));
		DrawStr(num_str);
		
		glMatrixMode(GL_MODELVIEW);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
	}
	
    count++;
	
}


