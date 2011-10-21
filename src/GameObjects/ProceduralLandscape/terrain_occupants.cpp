

#include "Game.h"
#include "World.h"
#include "terrain.h"
#include "GameObject.h"
#include "Sentinel.h"
#include "Sentry.h"
#include "Maths.h"


/********************************************************************************/
/*                        Landscape occupant handling                           */
/********************************************************************************/

void Terrain::distributeLandscapeOccupants()
{
	// LANDSCAPE OBJECT MODELS
	GameObject::loadAssets();
	
	placeSentinel();
	placeSentries();
	
	distributeTrees();
	distributeBoulders();
}

void Terrain::placeSentries()
{
	Square& sentinelSquare = getSentinelSquare();
	
	int nSentries = 1 + static_cast<int>(m_level)/100.0f;
	
	const float MIN_SENTRY_SPACING = 25 * getSquareEdgeSize();
	const float MIN_SENTRY_SENTINEL_DISTANCE = 5 * getSquareEdgeSize(); //30 * getSquareEdgeSize();
	const float MIN_SENTRY_HEIGHT = 0.0f * sentinelSquare.m_center[2];// 0.25f * sentinelSquare.m_center[2];
	
	vector<int> currentSentrySquares;
	
	for (int n=0; n<nSentries; ++n)
	{
		// choose a random unoccupied flat square
		vector<Square*>::size_type N = m_flat.size();
		
		int sIndx;
		int nTries = 0;
		bool abort = false;
		bool occupied, tooclose, toolow;
	
		Vec4& sentinelPos = sentinelSquare.m_center;
		
		do
		{
			int fIndx = getRand( N-1, &m_seed );
			sIndx = m_flat[fIndx];
			occupied = ( m_squares[sIndx].m_occupants.size() > 0 );
			if (occupied) continue;
			
			// reject this position if too close to an existing sentry or the sentinel itself, or too low
			Vec4& center = m_squares[sIndx].m_center;
			
			toolow = ( center[2] < MIN_SENTRY_HEIGHT );
			if (toolow) continue;
			
			Vec4 diff;
			diff = center - sentinelPos;
			float distSentinel2 = diff*diff;
			
			if (distSentinel2 < MIN_SENTRY_SENTINEL_DISTANCE*MIN_SENTRY_SENTINEL_DISTANCE)
			{
				tooclose = true;
				continue;
			}
			
			vector<int>::iterator iter_sentry_squares = currentSentrySquares.begin();
			float mindist2 = MAXFLOAT;
			while (iter_sentry_squares != currentSentrySquares.end())
			{
				int sIndx2 = *iter_sentry_squares;
				Square& square = m_squares[sIndx2];
				
				diff = square.m_center - center;
				float d2 = diff*diff;
				if (d2 < mindist2) mindist2 = d2;
				
				iter_sentry_squares++;
			}	
			
			tooclose = ( mindist2 < MIN_SENTRY_SPACING*MIN_SENTRY_SPACING );
			nTries++;
				
			if (nTries>1e4) 
			{
				printf("Disturbingly many sentry placement trials, skipping this sentry\n");
				abort = true;
				break;
			}
		}
		while ( occupied || tooclose || toolow );
		
		if (abort) continue;
		
		Square& square = m_squares[sIndx];
		
		// put a sentry there
		Vec4 translation = square.m_center;
		float rotation = 360.0f * ran2(&m_seed);
		
		Sentry* sentry = new Sentry( square.m_indx, translation, rotation );
		square.m_occupants.push_back( (GameObject*)sentry );
		currentSentrySquares.push_back(sIndx);
	}
}

void Terrain::placeSentinel()
{
	// put the sentinel on one of the highest squares
	vector<Square*>::size_type N = m_highSquares.size();
	int n = getRand( N-1, &m_seed );	
	int indx = m_squares[ m_highSquares[n] ].m_indx;
	Square& square = m_squares[indx];

	//  if already occupied, remove the old occupant(s)
	vector<GameObject*>::iterator iter_occupants = square.m_occupants.begin();
	while (iter_occupants != square.m_occupants.end())
	{
		delete *iter_occupants;
		square.m_occupants.erase(iter_occupants);
	}
	
	// create sentinel plinth
	Vec4 translation = square.m_center;
	translation[2] -= 0.6f*Plinth::getHeight();
	
	Plinth* plinth = new Plinth( square.m_indx, translation );
	square.m_occupants.push_back( (GameObject*)plinth );
	
	// put sentinel on top of plinth
	translation[2] += Plinth::getHeight();
	
	Game::pWorld->pSentinel = new Sentinel( square.m_indx, translation );
	square.m_occupants.push_back( (GameObject*) Game::pWorld->pSentinel );

	m_sentinel = square.m_indx;
	
	// make triangles which cover plinth top surface, so it can be raycasted against
	float s = 0.6f; // adjust to fit plinth top
	float X0 = square.m_center[0] - 0.5f * s * m_squareEdge;
	float X1 = square.m_center[0] + 0.5f * s * m_squareEdge;
	float Y0 = square.m_center[1] - 0.5f * s * m_squareEdge;
	float Y1 = square.m_center[1] + 0.5f * s * m_squareEdge;
	
	Vec4 plinthVerts[4];
	plinthVerts[0][0] = X0; plinthVerts[0][1] = Y0;
	plinthVerts[0][2] = translation[2];
	
	plinthVerts[1][0] = X1; plinthVerts[1][1] = Y0;
	plinthVerts[1][2] = translation[2];
	
	plinthVerts[2][0] = X1; plinthVerts[2][1] = Y1;
	plinthVerts[2][2] = translation[2];
	
	plinthVerts[3][0] = X0; plinthVerts[3][1] = Y1;
	plinthVerts[3][2] = translation[2];
	
	m_plinthTopTriangles[0] = new SpaceTriangle(plinthVerts[0], plinthVerts[2], plinthVerts[3]); 
	m_plinthTopTriangles[1] = new SpaceTriangle(plinthVerts[0], plinthVerts[1], plinthVerts[2]); 	
}

void Terrain::distributeTrees()
{
	int MIN_NUM_TREES = m_flat.size() / 50;
	int MAX_NUM_TREES = m_flat.size() / 25;
	
	int numTrees = MIN_NUM_TREES + getRand( MAX_NUM_TREES - MIN_NUM_TREES, &m_seed );
	
	for (int i=0; i<numTrees; ++i)
	{
		Square* square;
		
		do 
		{
			int n = getRand( m_flat.size()-1, &m_seed );
			square = &m_squares[ m_flat[n] ];
		} while (square->m_occupants.size() > 0);
		
		Vec4 translation = square->m_center;
		
		Tree* tree = new Tree( square->m_indx, translation );
		square->m_occupants.push_back( (GameObject*)tree );
	}
}


void Terrain::distributeBoulders()
{
	int MIN_NUM_BOULDERS = m_flat.size() / 100;
	int MAX_NUM_BOULDERS = m_flat.size() / 50;
	
	int numBoulders = MIN_NUM_BOULDERS + getRand( MAX_NUM_BOULDERS - MIN_NUM_BOULDERS, &m_seed );
	
	for (int i=0; i<numBoulders; ++i)
	{
		Square* square;
		int indx;
		
		do 
		{
			indx = getRand( m_flat.size()-1, &m_seed );
			square = &m_squares[ m_flat[indx] ];
		} while (square->m_occupants.size() > 0);
		
		Vec4 translation = square->m_center;
		
		Boulder* boulder = new Boulder( square->m_indx, translation );
		square->m_occupants.push_back( (GameObject*)boulder );
	}
}

void Terrain::registerSquareAsOccupied(int indx)
{
	m_occupiedSquares.insert(indx);
}




/***********************************************************************************************************/
/*                                                                                                         */
/*                                Runtime update of terrain occupants                                      */
/*                                                                                                         */
/***********************************************************************************************************/


void Terrain::handleOccupants()
{
	set<int>::iterator iter = m_occupiedSquares.begin();
	
	while (iter != m_occupiedSquares.end())
	{
		int indx = *iter;
		Square& square = m_squares[indx];
		
		if (square.m_occupants.size() == 0)
		{
			iter++;
			continue;
		}
		
		vector<GameObject*>::iterator iter_occupants = square.m_occupants.begin();
		while (iter_occupants != square.m_occupants.end())
		{
			GameObject* object = *iter_occupants;
			
			// if object is an enemy, do the AI update (not if it's being absorbed, or dead, though)
			if ( object->getType() == OBJECT_ENEMY && object->getState() == (Object_States)OBJECT_NORMAL )
			{
				Enemy* E = static_cast<Enemy*> (object);
				E->think();
			}
			
			// if object has been flagged as dead (completely absorbed), remove it
			if (object->getState() == OBJECT_DEAD)
			{
				// call destructor on the GameObject (automatically removes corresponding DisplayObject)
				delete object;
				
				square.m_occupants.erase(iter_occupants);
				
				// If the square is now empty, remove it from the occupied list
				if (square.m_occupants.size() == 0)
				{
					m_occupiedSquares.erase(square.m_indx);
				}
				
				continue;
			}
			
			// auto (player) absorption
			else if ( object->getState() == (Object_States) OBJECT_ABSORBING )
			{
				object->updateAbsorptionByPlayer();
			}
			
			// object materialization
			else if ( object->getState() == (Object_States) OBJECT_MATERIALIZING )
			{
				object->updateMaterialization();
			}
			
			iter_occupants++;
		}
		
		iter++;
	}
}







