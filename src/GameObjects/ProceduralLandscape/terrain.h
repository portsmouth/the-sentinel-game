

#ifndef TERRAIN_H
#define TERRAIN_H

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#include "Vec4.h"
#include "Square.h"
#include "Game_enums.h"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::set;
using std::map;


#define N_HIT_VECS 6

class Texture;
class Model;

class Terrain
{
	
public:	
	
	// Procedural landscape generation, seeded by level number
	Terrain(int level);
	~Terrain(); 
	
	// Generation of initial occupants	
	void distributeLandscapeOccupants();
		
	// handing of occupant updates
	void handleOccupants();
	void registerSquareAsOccupied(int indx);
		
	// Raycasting:
	void raycast(const Vec4& P, const Vec4& D, HitData& data, RaycastType casttype);             
	bool isTargetVisibleFromOrigin( const Vec4& Origin, const Vec4& Target, HitData& data );      
		
	// Rendering
	void loadAssets();
	void drawOccupants();
	void draw();
	
	// Accessors
	Square& getSquare( int indx );
	Square& getPlayerSquare();
	Square& getSentinelSquare();
	int getNumSquaresPerEdge();
	float getTerrainEdgeSize();
	float getSquareEdgeSize();
	const set<int>& getOccupiedSquares();
	const vector<int>& getFlatSquares();
	
	void setPlayerSquare( int indx );
	void setInitPlayerSquare();
	
	// utility function for hyperspacing
	int findRandomEmptySquareLowerThan(Square& inSquare); 
	
	// debug
	void highlightSquare( int indx );
	void drawPlinthTop_DEBUG();
	void drawBoulderAABBs_DEBUG();
	void drawBresenhamBand_DEBUG();
	void drawOccupiedSquares_DEBUG();
	void drawNormals_DEBUG();
	
	long& getSeed();
	
private:
	
	// procedural landscape construction
	void constructMesh();	
	void mesh_buildLayers();
	void mesh_makeCorners();
	void mesh_findPlinths();	
	void mesh_findLowestSquares();
	void mesh_findHighestSquares();
	void mesh_makeWalls();
	void mesh_makeTris();
	void mesh_prettification();	
	void mesh_setRadii();

	// occupants
	void distributeTrees();
	void distributeBoulders();
	void placeSentinel();
	void placeSentries();
	
	// raycasting
	void Bresenham( int* start, int* end, vector<int>& squaresHit, int padSize );
	void raycast_terrain(const Vec4& P, const Vec4& D, HitData& data, const vector<int>& squaresHit);
	void raycast_boulders(const Vec4& P, const Vec4& D, HitData& data, const vector<int>& squaresHit);
	void raycast_plinth(const Vec4& P, const Vec4& D, HitData& data);

	vector<int> m_squaresHit[N_HIT_VECS];   // storage for runtime raycasting
	vector<HitData> m_hits;
	map<int, int> m_sectorEdge; // map to improve sector raycasting performance
	set<int> m_edgeCrossingSet; // set to improve sector raycasting performance
	
	// internal access
	Square& getGridSquare(int i, int j);
	int getSquareIndx(int i, int j);
	void getSquareOfCoords( float x, float y, int* grid );
	
	// DATA
	vector<Square> m_squares;   // array of Squares in 2d grid
	vector<Vec4> m_mesh;        // array of mesh vertices which give square corners
	vector<int> m_flat;         // indices into m_squares of the squares which are flat (accessible to player) 
	vector<int> m_walls;        // indices into m_square of the squares which are non-flat (not accessable to player)
	vector<int> m_planeWalls;   // planar walls
	vector<int> m_creasedWalls; // walls with a crease
	vector<int> m_lowSquares;   // indices into m_square of the lowest rung of flat squares
	vector<int> m_highSquares;  // indices into m_square of the highest rung of flat squares
	set<int> m_occupiedSquares; // set containing the indices of the squares with occupants
	
	SpaceTriangle* m_plinthTopTriangles[2];
	
	// dimensions
	int m_numSquaresPerEdge;
	float m_terrainEdgeSize;
	float m_squareEdge;
	
	// index of square currently occupied by player
	int m_player;
	// index of square currently occupied by Sentinel
	int m_sentinel;
		
	// level
	int m_level;
	
	// random seed
	long m_seed;
	
	/* rendering */
	void createDisplayLists();
	GLuint m_planeWallList;
	GLuint m_creasedWallList;
	GLuint m_flatList;
	
	Texture *m_planeWallTexture;
	Texture *m_creasedWallTexture;
	Texture *m_flatTexture;
	
	Model *m_boulderModel;
	
	float terrain_texture_cover;
	float no_emission[4];
	
	// DEBUG
	void drawNormals();
	
};


/********************************************************************************/
/*                              inline Accessors                                */
/********************************************************************************/

inline int Terrain::getNumSquaresPerEdge()
{
	return m_numSquaresPerEdge;
}

inline float Terrain::getSquareEdgeSize()
{
	return m_squareEdge;
}

inline float Terrain::getTerrainEdgeSize()
{
	return m_terrainEdgeSize;
}

inline Square& Terrain::getPlayerSquare()
{
	return m_squares[m_player];
}

inline Square& Terrain::getSentinelSquare()
{
	return m_squares[m_sentinel];
}

inline Square& Terrain::getSquare( int indx )
{
	return m_squares[indx];
}


inline void Terrain::setPlayerSquare( int indx )
{
	m_player = indx;
}

inline void Terrain::getSquareOfCoords( float x, float y, int* grid )
{
	int i = static_cast<int>( ( .5f + x/m_terrainEdgeSize ) * m_numSquaresPerEdge );
	int j = static_cast<int>( ( .5f + y/m_terrainEdgeSize ) * m_numSquaresPerEdge );
	grid[0] = i;
	grid[1] = j;
}

inline int Terrain::getSquareIndx(int i, int j)
{
	if ( i<0 || i>m_numSquaresPerEdge-1) return -1;
	if ( j<0 || j>m_numSquaresPerEdge-1) return -1;
	
	int indx = j + m_numSquaresPerEdge*i;
	return indx;
}

inline Square& Terrain::getGridSquare(int i, int j)
{
	return m_squares[ getSquareIndx(i,j) ];
}

inline const set<int>& Terrain::getOccupiedSquares()
{
	return m_occupiedSquares;
}

inline const vector<int>& Terrain::getFlatSquares()
{
	return m_flat;
}

inline long& Terrain::getSeed()
{
	return m_seed;
}



#endif //TERRAIN_H

