

#include "Game.h"
#include "terrain.h"
#include "GameObject.h"
#include "Maths.h"

#include <float.h>


/********************************************************************************/
/*                           Terrain raycasting                                 */
/********************************************************************************/

// some tolerance of intersection point inaccuracies, to prevent discarding rays
// emanating from points on the terrain surface
const float epsilonRaycast = 1.0e-5;

// whether or not the segment between two specified 3d points intersects the landscape, boulders or plinth
bool Terrain::isTargetVisibleFromOrigin( const Vec4& Origin, const Vec4& Target, HitData& data  )
{
	if ( Origin==Target ) return true;
	
	Vec4 ray = Target - Origin;
	float l = ray.length3();
	ray /= l;

	raycast(Origin, ray, data, TERRAIN_AND_BOULDERS_AND_PLINTH);
	
	if ( !data.m_hit ) return true;
	
	return ( data.m_t >= l - epsilonRaycast );
}


/*  Raycast against terrain walls and flat squares, and also if desired boulders and plinth. 
    Supply 3d ray origin P, and normalized 3d direction D. */
void Terrain::raycast(const Vec4& P, const Vec4& D, HitData& data, RaycastType casttype)
{
	Vec4 Pproj(P[0], P[1], 0.f);
	Vec4 Dproj(D[0], D[1], 0.f);
	Dproj.normalizeIfNotZero();
	Vec4 Eproj = Pproj + (1.5f*m_terrainEdgeSize) * Dproj;
	
	int grid_start[2], grid_end[2];
	getSquareOfCoords( Pproj[0], Pproj[1], grid_start );
	getSquareOfCoords( Eproj[0], Eproj[1], grid_end );
	
	// Use Bresenham algorithm to find a thin band of candidate grid squares for height testing 
	vector<int>& squaresHit = m_squaresHit[0];
	
	squaresHit.clear();
	int padSize = 1;
	Bresenham( grid_start, grid_end, squaresHit, padSize );

	m_hits.clear();
	
	// Raycast against terrain in each square in candidate band
	{
		HitData dataTerrain;
		raycast_terrain(P, D, dataTerrain, squaresHit);
		if (dataTerrain.m_hit) m_hits.push_back(dataTerrain);
	}

	if (casttype == TERRAIN_AND_BOULDERS || casttype == TERRAIN_AND_BOULDERS_AND_PLINTH)
	{
		// Raycast against boulder AABBs in each square in candidate band
		HitData dataBoulder;
		raycast_boulders(P, D, dataBoulder, squaresHit);
		if (dataBoulder.m_hit) m_hits.push_back(dataBoulder);
	}
		
	if (casttype == TERRAIN_AND_BOULDERS_AND_PLINTH)
	{
		// Raycast against plinth
		HitData dataPlinth;
		raycast_plinth(P, D, dataPlinth);
		if (dataPlinth.m_hit)  m_hits.push_back(dataPlinth);
	}
		
	// compare hit results
	if ( m_hits.size()==0 ) 
	{
		data.m_hit = false;
		return;
	}
	
	vector<HitData>::iterator it = m_hits.begin();
	vector<HitData>::iterator it_choose = it;
	float tmin = it_choose->m_t;
	it++;
	
	while (it != m_hits.end())
	{
		if ( it->m_t < tmin )
		{
			it_choose = it;
			tmin = it->m_t;
		}
		it++;
	}
	
	data = *it_choose;
}


void Terrain::raycast_terrain(const Vec4& P, const Vec4& D, HitData& data, const vector<int>& squaresHit)
{
	data.m_type = TERRAIN_HIT;
	data.m_hit = false;
	data.m_square = -1;
	
	vector<int>::const_iterator iter = squaresHit.begin();
	while (iter != squaresHit.end())
	{
		int squareIndx = *iter;
		Square& square = m_squares[squareIndx];
		float t;
		
		if ( square.intersect( P, D, &t, m_mesh ) )
		{
			if (!data.m_hit)
			{
				data.m_hit = true;
				data.m_t = t;
				data.m_square = squareIndx;
			}
			else
			{
				if (t<data.m_t)
				{
					data.m_t = t;
					data.m_square = squareIndx;
				}
			}
		}
		++iter;		
	}	
}

void Terrain::raycast_boulders(const Vec4& P, const Vec4& D, HitData& data, const vector<int>& squaresHit)
{
	data.m_type = BOULDER_AABB_HIT;
	data.m_hit = false;
	
	vector<int>::const_iterator iter = squaresHit.begin();
	while (iter != squaresHit.end())
	{
		int squareIndx = *iter;
		Square& square = m_squares[squareIndx];
		
		if (square.m_occupants.size() == 0)
		{
			iter++;
			continue;
		}
		
		vector<GameObject*>::iterator iter_occupants = square.m_occupants.begin();
		while (iter_occupants != square.m_occupants.end())
		{
			GameObject* object = *iter_occupants;
			
			if ( object->getType() == OBJECT_BOULDER )
			{
				Boulder* boulder = static_cast<Boulder*> ( object );
				
				const AABB& aabb = boulder->getWorldAABB();
				float t;
				if ( aabb.raycast( P, D, &t ) )
				{
					if (!data.m_hit)
					{
						data.m_hit = true;
						data.m_t = t;
						data.m_boulder = boulder;
					}
					else
					{
						if (t<data.m_t)
						{
							data.m_t = t;
							data.m_boulder = boulder;
						}
					}
				}
			}
			
			iter_occupants++;
		}
		++iter;
	}	
}

void Terrain::raycast_plinth(const Vec4& P, const Vec4& D, HitData& data)
{
	data.m_type = SENTINEL_PLINTH_HIT;
	data.m_t = FLT_MAX;
	data.m_hit = false;
	
	float t1_plinth, t2_plinth;
	bool i1_plinth = m_plinthTopTriangles[0]->intersect(P, D, &t1_plinth);
	bool i2_plinth = m_plinthTopTriangles[1]->intersect(P, D, &t2_plinth);
	
	if ( i1_plinth ) 
	{
		Vec4& N = m_plinthTopTriangles[0]->m_n;
		if (D*N < 0.0f) // only count intersections through top side of plinth
		{
			data.m_hit = true;
			data.m_t = t1_plinth;
		}
	}
	if ( i2_plinth)
	{
		Vec4& N = m_plinthTopTriangles[1]->m_n;
		if (D*N < 0.0f)
		{
			data.m_hit = true;
			data.m_t = fminf(data.m_t, t2_plinth);
		}
	}		
}


static inline void Swap( int& a, int& b )
{
	int c = a;
    a = b;
    b = c;
}


void Terrain::Bresenham( int* start, int* end, vector<int>& squaresHit, int padSize )
{
	int x0 = start[0];
	int y0 = start[1];
	int x1 = end[0];
	int y1 = end[1];
	
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) 
	{
        Swap(x0, y0);
        Swap(x1, y1);
    }
    if (x0 > x1) 
	{
        Swap(x0, x1);
        Swap(y0, y1);
    }
	
    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = 0, ystep;
    int y = y0;
    if (y0 < y1) ystep = 1; else ystep = -1;
    
	for (int x = x0; x <= x1; x++) 
	{
		bool nohit  = (y<0 || y>m_numSquaresPerEdge-1);
		     nohit |= (x<0 || x>m_numSquaresPerEdge-1);
		
		if (!nohit)
		{
			if (steep) 
			{
				squaresHit.push_back( getSquareIndx(y,x) );
			}
			else 
			{
				squaresHit.push_back( getSquareIndx(x,y) );
			}
		}
        
		error += deltay;
        if (2 * error >= deltax) 
		{
            y += ystep;
            error -= deltax;
        }
    }
	
	//pad line
	if (padSize < 1) return;
	
	int nSize = squaresHit.size();
	
	if (steep)
	{
		for (int n=0; n<nSize; ++n) 
		{
			int squareIndx = squaresHit[n];
			Square& square = m_squares[squareIndx];
			
			int x = square.m_i; 
			int y = square.m_j; 
			
			for (int p=1; p<=padSize; ++p)
			{
				if (x+p < m_numSquaresPerEdge ) squaresHit.push_back( getSquareIndx(x+p, y) );
				if (x-p >= 0)                   squaresHit.push_back( getSquareIndx(x-p, y) );
			}
		}
	}
	
	else
	{
		for (int n=0; n<nSize; ++n) 
		{
			int squareIndx = squaresHit[n];
			Square& square = m_squares[squareIndx];
			
			int x = square.m_i; 
			int y = square.m_j; 

			for (int p=1; p<=padSize; ++p)
			{
				if (y+p < m_numSquaresPerEdge ) squaresHit.push_back( getSquareIndx(x, y+p) );
				if (y-p >= 0)                   squaresHit.push_back( getSquareIndx(x, y-p) );
			}			
		}
	}
}


#if 0
// find all the squares within a sector defined by two directions and an origin

// Find which squares are potentially visible in the acute sector between two
// given normalized directions in the plane pointing from the same origin.
// This works by doing a horizontal fill between the two edge rays and the boundary,
// so it will only fill the entire sector so long as the projected 2d angle between the rays
// is less than or equal to 2 * acos(2/sqrt(5)) = 53.13 degrees.

// note - fill broken when sector aligned with scan direction currently, misses a line - need to
// to scan along direction most orthogonal to sector, which hasn't been implemented correctly yet.

/*  - example of prior use of in ::Sentinel

 float angleA = m_rotation - SENTINEL_VIEWRANGE_HALF;
 float angleB = m_rotation + SENTINEL_VIEWRANGE_HALF;
 
 Vec4 sectorA( cos(angleA * degToRad), sin(angleA * degToRad), 0.f );
 Vec4 sectorB( cos(angleB * degToRad), sin(angleB * degToRad), 0.f );
 
 m_sectorSquares.clear();
 Game::pWorld->pTerrain->squaresInSector(sectorA, sectorB, m_eyePosition, m_sectorSquares);
 */

/*
	vector<int>::const_iterator iter = m_sectorSquares.begin();
	while (iter != m_sectorSquares.end())
	{
		Square& square = Game::pWorld->pTerrain->getSquare( *iter );
		
		if ( square.m_occupants.size() > 0 
			 
			 // sentinel should not try to absorb itself ..
			 && square.m_indx != m_squareIndx ) 
		{	
			GameObject* object = square.getHighestOccupant();
			const Vec4& T = object->getTranslation();
			
			//if ( Game::pWorld->pTerrain->isTargetVisibleFromOrigin( m_eyePosition, T ) )
			//{
			//	m_seenObjects.push_back(object);
			//}
		}
		
		iter++;
	}
*/

/*
	 +------+------+     a = acos( 2/sqrt(5) )
     |     /|\     | 
     |    / | \    |
 2L  |   /  |a \   |
     |  /   |   \  |
     | /    |    \ |
     |/     |     \|
     +------+------+
         L     L 
 */

inline int _sign(float x)
{
	if (x==0.0f) return 0;
	return x > 0.f ? 1 : -1;
}

// v1 and v2 are assumed to be normalized, with zero z component

void Terrain::squaresInSector(const Vec4& v1, const Vec4& v2, const Vec4& O, vector<int>& sectorSquares)
{
	Vec4 Oproj(O[0], O[1], 0.f);
	int grid_start[2];
	getSquareOfCoords( Oproj[0], Oproj[1], grid_start );
	
	// Find squares at the sector edges using Bresenham
	int grid_end1[2], grid_end2[2];
	Vec4 Eproj1 = Oproj + (1.5f * m_terrainEdgeSize) * v1;
	Vec4 Eproj2 = Oproj + (1.5f * m_terrainEdgeSize) * v2;
	getSquareOfCoords( Eproj1[0], Eproj1[1], grid_end1 );
	getSquareOfCoords( Eproj2[0], Eproj2[1], grid_end2 );
	
	vector<int>& sectorEdge1 = m_squaresHit[1];
	vector<int>& sectorEdge2 = m_squaresHit[2];
	sectorEdge1.clear();
	sectorEdge2.clear();
	
	int padSize = 0;
	Bresenham( grid_start, grid_end1, sectorEdge1, padSize );
	Bresenham( grid_start, grid_end2, sectorEdge2, padSize );
	
	// decide whether to scan along x or along y
	Vec4 plusX(1.f, 0.f, 0.f); Vec4 plusY(0.f, 1.f, 0.f);
	//Vec4 bisection = 0.5f * (v1 + v2);
	//bisection.normalizeIfNotZero();
	
	//float horiz = fabs( bisection * plusX );
	//float verti = fabs( bisection * plusY );
	bool scanAlongX; //= (horiz > verti);
	
	// Each edge vector defines a bisection of the plane into 2 half-planes.
	// We want to walk, from each pixel on the edge, along the scan direction 
	// which takes us into the half-plane which contains the other edge...
	Vec4 z12 = v1 % v2;
	int sign12 = _sign( z12[2] );
	
	Vec4 z1, z2;
	
	scanAlongX = true;
	if (scanAlongX)
	{
		z1 = v1 % plusX; z2 = plusX % v2;
	}
	else
	{
		z1 = v1 % plusY; z2 = plusY % v2;
	}
	
	int walkDirection1 = ( _sign( z1[2] ) == sign12 ) ? 1 : -1;
	int walkDirection2 = ( _sign( z2[2] ) == sign12 ) ? 1 : -1;
	
	// Walk from each square of edge1 in the determined direction, until either we hit
	// the grid boundary or a square contained in edge2 is hit. Repeat for edge 2.
	walkSectorEdge( sectorEdge1, sectorEdge2, walkDirection1, sectorSquares, scanAlongX );
	walkSectorEdge( sectorEdge2, sectorEdge1, walkDirection2, sectorSquares, scanAlongX );
}


// TO DO: solve issue of missing line in sector: easiest to switch scan direction from x to y depending on quadrant

inline void Terrain::_getIJ( int& i, int& j, const Square& square, bool scanAlongX )
{
	//if (scanAlongX)
	{
		i = square.m_i;
		j = square.m_j;
	}
	//else
	//{
	//	i = square.m_j;
	//	j = square.m_i;
	//}
}

inline int Terrain::_getIndx( int i, int j, bool scanAlongX )
{
	//if (scanAlongX)
	{
		return getSquareIndx(i, j); 
	}
	//else
	//{
	//	return getSquareIndx(j, i); 
	//}	
}


void Terrain::walkSectorEdge( const vector<int>& edge, 
							  const vector<int>& otherEdge, 
							  int walkDirection, vector<int>& sectorSquares, 
							  bool scanAlongX )
{
	vector<int>::const_iterator iter; 
	
	// build a map between the y coord (j) of the 'other edge' squares and the index of the square
	// closest along the walk direction at that j level.
	m_sectorEdge.clear();
	
	iter = otherEdge.begin();
	
	do
	{
		bool first = true;
		int currentJ = -1;
		int endI = -1;
		int endIndx = -1;
		
		// go through a line of constant j
		while (iter != otherEdge.end())
		{
			int squareIndx = *iter;
			const Square& square = m_squares[squareIndx];
			
			int i, j;
			//_getIJ(i,j,square,scanAlongX);
			i = square.m_i;
			j = square.m_j;

			if (first)
			{
				currentJ = j;
				endI = i;
				endIndx = squareIndx;
				first = false;
			}
			
			else if (j != currentJ) break; // next j line encountered
			
			else if ( i * walkDirection < endI * walkDirection )
			{
				endI = i;   // find the closest square along the walk direction at this j
				endIndx = squareIndx;
			}
			
			++iter;
		}
		
		m_sectorEdge.insert( map<int, int>::value_type(currentJ, endIndx) );
	}
	while ( iter != otherEdge.end() );
	
	// advance along x-direction from edge to other edge
	iter = edge.begin();
	while (iter != edge.end())
	{
		int squareIndx = *iter;
		const Square& square = m_squares[squareIndx];
		
		int i, j;	
		//_getIJ(i,j,square,scanAlongX);
		i = square.m_i;
		j = square.m_j;
		
		int currIndx = square.m_indx;
		
		if ( m_sectorEdge.count(j) > 0 )
		{
			// find the corresponding square on the other edge
			int io = m_squares[ m_sectorEdge[j] ].m_i;
			
			if ( io * walkDirection >= i * walkDirection )
			{
				int iop = io + walkDirection;
				
				// add all the squares between i and io with this j
				do
				{
					sectorSquares.push_back(currIndx);
					i += walkDirection;
					currIndx = getSquareIndx(i, j); //_getIndx(i,j,scanAlongX);
				} while (i != iop);
			}
		}
		else
		{
			// walk all the way to domain boundary
			do
			{
				sectorSquares.push_back(currIndx);
				i += walkDirection;
				currIndx = getSquareIndx(i, j); //_getIndx(i,j,scanAlongX);
			} while ( currIndx != -1 );
		}
		
		++iter;		
	}
}


#endif




