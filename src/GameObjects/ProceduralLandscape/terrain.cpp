

#include "Game.h"
#include "World.h"
#include "Sentinel.h"
#include "GameObject.h"
#include "terrain.h"
#include "Maths.h"


Terrain::Terrain(int level) :

m_level(level)

{	
	m_seed = -level-2;
	ran2(&m_seed);
	
	constructMesh();
	createDisplayLists();
	
	// terrain assets
	loadAssets();
}

Terrain::~Terrain()  //destructor
{
	// call destructors of all the landscape occupants
	vector<Square>::iterator iter_square = m_squares.begin();	
	while (iter_square != m_squares.end())
	{
		Square& square = (*iter_square);
		vector<GameObject*>::iterator iter_occupants = square.m_occupants.begin();
		
		while (iter_occupants != square.m_occupants.end())
		{
			delete *iter_occupants;
			iter_occupants++;
		}
		
		square.m_occupants.clear();
		iter_square++;
	}
	
	// clear arrays
	m_squares.clear();
	m_lowSquares.clear();
	m_highSquares.clear();
	m_mesh.clear();	
	m_flat.clear();
	m_walls.clear();
	m_planeWalls.clear();
	m_creasedWalls.clear();
	
	for (int i=0; i<N_HIT_VECS; ++i)
	{
		m_squaresHit[i].clear();
	}
	
	delete m_plinthTopTriangles[0];
	delete m_plinthTopTriangles[1];
	
	// free display lists
	glDeleteLists(m_planeWallList,1);
	glDeleteLists(m_creasedWallList,1);
	glDeleteLists(m_flatList,1);
}


// sets the initial square of the player - 
// henceforth, it is Player's responsibility to keep track of the player eye-point location in space
void Terrain::setInitPlayerSquare()
{
	Square* square;
	Vec4 d;
	
	do 
	{
		int n = getRand( m_lowSquares.size()-1, &m_seed );
		square = &m_squares[ m_lowSquares[n] ];
		d = square->m_center - Game::pWorld->pSentinel->getTranslation();

	} while ( (square->m_occupants.size()>0) || (d.length3()<m_squareEdge*30.f));
	
	m_player = square->m_indx;
}

// Find a random empty (flat) square on the landscape with a height lower than that of the
// given square (used in choosing a square to hyperspace to)
int Terrain::findRandomEmptySquareLowerThan(Square& inSquare)
{
	static long seed = -3;
	vector<int> m_hyperspaceSquares; 
	
	vector<int>::iterator iter = m_flat.begin();
	while (iter != m_flat.end())
	{
		int squareIndx = (*iter);

		Square& square = m_squares[squareIndx];
		
		if ( (square.m_center[2]<=inSquare.m_center[2]) && (square.m_occupants.size() == 0) )
		{
			m_hyperspaceSquares.push_back(square.m_indx);
		}
		
		iter++;		
	}
	
	if (m_hyperspaceSquares.size() == 0) return -1;
	
	int n = getRand( m_hyperspaceSquares.size()-1, &seed );
	
	return m_hyperspaceSquares[n];
}







