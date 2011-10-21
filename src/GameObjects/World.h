
#ifndef WORLD_H
#define WORLD_H

#define COLLISION_GRID 32

#include <vector>
using std::vector;

class World 
{
	
/* contains and handles objects populating the game world
   (terrain, cylinders, bullets, enemies) */	
	
public:	
	
	World(int level_num); //constructor
	void Init();
	~World(); //destructor
	
	void draw();
	
	class Sky* pSky; 
	class Terrain* pTerrain;	
	class Sentinel* pSentinel;
	
	friend class EventManager;

private:

	int level;
	long seed;

};

#endif //WORLD_H
