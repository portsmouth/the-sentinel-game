


#include "World.h"

#include "DisplayHandler.h"
#include "GameObject.h"
#include "terrain.h"
#include "sky.h"

#include "geometry.h"
#include "FileUtils.h"

#include <vector>
#include <stack>
#include <string>
#include <iostream>
#include <assert.h>
using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::stack;

//constructor
World::World(int level_num) 
{		
	level = level_num;
	
	//define terrain parameters, later these will be level specific
	pTerrain = new Terrain(level);
	
	pSky = new Sky;
}

void World::Init()
{
	//add landscape objects (sentinel, boulders, trees, etc..)
	pTerrain->distributeLandscapeOccupants();

	pSky->Init();		
}

//destructor
World::~World()
{
	//free up texture resources specific to a level
	TextureLibrary *texturelibrary = TextureLibrary::Instance();
	string resourcepath = FileUtils::GetApplicationResourcesFolderPath();

	delete pSky;
	delete pTerrain;
	
	DisplayHandler::Instance()->reset();
}


void World::draw()
{
	//backface culling for performance here, elsewhere it's just a nuisance
	glEnable(GL_CULL_FACE);
	
	//draw sky
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	pSky->draw();
	glPopAttrib();
	
	//draw terrain
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	pTerrain->draw();
	glPopAttrib();
	
	//draw game objects
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	DisplayHandler::Instance()->renderAll();
	glPopAttrib();
		
	glDisable(GL_CULL_FACE);
}


