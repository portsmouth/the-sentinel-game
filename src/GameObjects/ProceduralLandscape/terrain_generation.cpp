
#include "Game.h"
#include "terrain.h"
#include "Maths.h"

// uncomment this for the "ultra-retro" look with creases even on planar grid squares
#define CREASED_LOOK


/*****************************************************************************/
/*                      Procedural terrain generation                        */     
/*****************************************************************************/


void Terrain::constructMesh() 
{
	m_terrainEdgeSize = 90.f;
	m_numSquaresPerEdge = 50;
	
	// allocate some memory now for use at runtime
	//m_squaresHit[0].reserve(6*m_numSquaresPerEdge);
	//m_squaresHit[1].reserve(6*m_numSquaresPerEdge);
	//m_squaresHit[2].reserve(6*m_numSquaresPerEdge);
	
	m_squareEdge = m_terrainEdgeSize / m_numSquaresPerEdge;
	float halfL = 0.5f * m_terrainEdgeSize;
	
	// create (N+1)^2 mesh vertices
	for (int i=0; i<=m_numSquaresPerEdge; ++i)
	{
		float x = -halfL + i * m_squareEdge;
		
		for (int j=0; j<=m_numSquaresPerEdge; ++j)
		{
			float y = -halfL + j * m_squareEdge;
			Vec4 v(x, y, 0.f);
			m_mesh.push_back(v);
		}
	}
	
	// create N^2 grid squares
	for (int i=0; i<m_numSquaresPerEdge; ++i)
	for (int j=0; j<m_numSquaresPerEdge; ++j)
	{
		Square square(i, j, m_squares.size(), m_numSquaresPerEdge);
		square.m_height = 0.f;
			
		m_squares.push_back(square);
	}
			
	mesh_buildLayers();
	mesh_makeCorners();
	mesh_findPlinths();
	mesh_makeTris();
	mesh_makeWalls();
	mesh_findLowestSquares();
	mesh_findHighestSquares();
	mesh_prettification();	
	mesh_setRadii();
}

inline static Vec4 computeNormal( const Square::Triangle t, const vector<Vec4>& m_mesh )
{
	const Vec4& v0 = m_mesh[ t.m_indx[0] ];
	const Vec4& v1 = m_mesh[ t.m_indx[1] ];
	const Vec4& v2 = m_mesh[ t.m_indx[2] ];
	
	// winding 0->1->2 gives normal by right hand rule
	Vec4 d10 = v1 - v0;
	Vec4 d20 = v2 - v0;
	Vec4 norm = d20 % d10;
	norm.normalizeIfNotZero();	
	return norm;
}

// store bounding radius of each grid square (about its triangles' centroid)
inline static void computeSquareRad( Square& square, const vector<Vec4>& m_mesh )
{
	float R = -1.f;
	for (int n=0; n<4; ++n)
	{
		const Vec4& corner = m_mesh[ square.m_corners[n] ];
		Vec4 diff = corner - square.m_center;
		float rSqr = diff*diff;
		if (rSqr>R) R = rSqr;
	}	
	
	square.m_boundRadius = R;
}

inline static bool _isFlat( Square::Triangle& t, const vector<Vec4>& m_mesh )
{
	const Vec4& v0 = m_mesh[ t.m_indx[0] ];
	const Vec4& v1 = m_mesh[ t.m_indx[1] ];
	const Vec4& v2 = m_mesh[ t.m_indx[2] ];
	
	float averageZ = (v0[2] + v1[2] + v2[2]) / 3.f;
	bool b0 = ( fabs(v0[2] - averageZ) < FLATNESS_TOL );
	bool b1 = ( fabs(v1[2] - averageZ) < FLATNESS_TOL );
	bool b2 = ( fabs(v2[2] - averageZ) < FLATNESS_TOL );
	
	return b0 && (b1 && b2);
}

void Terrain::mesh_buildLayers()
{
	const int NUM_LAYERS = 20;
	const int MIN_NUM_PLINTHS_PER_LAYER = 4;
	const int MAX_NUM_PLINTHS_PER_LAYER = 20;
	const int MAX_PLINTH_EDGE = 30; //must not exceed m_numSquaresPerEdge !
	const int MIN_PLINTH_EDGE = 4;
	
	
	//const int MIN_NUM_PLINTHS = 2;
	//const int MAX_NUM_PLINTHS = 28;
	//const int MAX_PLINTH_EDGE = 24; //must not exceed m_numSquaresPerEdge !
	//const int MIN_PLINTH_EDGE = 6;
	//const int NUM_LAYERS = 30;
	
	// build up landscape layers
	for (int layer=0; layer<NUM_LAYERS; ++layer)
	{
		int num_plinths = MIN_NUM_PLINTHS_PER_LAYER + getRand( MAX_NUM_PLINTHS_PER_LAYER - MIN_NUM_PLINTHS_PER_LAYER, &m_seed );
		int np=0;
		while (np<num_plinths)
		{
			// plinth dimensions
			int max_plinth_size = MIN_PLINTH_EDGE + MAX_PLINTH_EDGE * (int)(1 - layer/((float)NUM_LAYERS-1));
			int pX = MIN_PLINTH_EDGE + getRand( max_plinth_size - MIN_PLINTH_EDGE, &m_seed );
			int pY = MIN_PLINTH_EDGE + getRand( max_plinth_size - MIN_PLINTH_EDGE, &m_seed );
			
			// location of plinth corner
			int px = getRand( m_numSquaresPerEdge-pX, &m_seed );
			int py = getRand( m_numSquaresPerEdge-pY, &m_seed );
			
			float plinth_height = 1.1f * m_squareEdge;
			
			for (int i=px; i<px+pX; ++i)
			for (int j=py; j<py+pY; ++j)
			{
				int indx = i + (m_numSquaresPerEdge) * j;
				Square& square = m_squares[indx];
				square.m_height += plinth_height;
			}
				
			np++;
		}
	}
}

// update mesh vertices according to square heights
// (each vertex gets height of the highest square which has that vertex as a corner)
void Terrain::mesh_makeCorners()
{
	for (int i=0; i<m_numSquaresPerEdge; ++i)
	for (int j=0; j<m_numSquaresPerEdge; ++j)
	{
		int indx = i + m_numSquaresPerEdge*j;
		Square& square = m_squares[indx];
		float H = square.m_height;
			
		for (int n=0; n<4; ++n)
		{
			Vec4& corner = m_mesh[ square.m_corners[n] ];
			if (H > corner[2]) corner[2] = H;
		}
	}
}

// find flat squares
void Terrain::mesh_findPlinths()
{
	vector<Square>::iterator iter = m_squares.begin();
	float H, minH, maxH;
	
	while (iter != m_squares.end())
	{
		Square& square = (*iter);
		Vec4& corner0 = m_mesh[ square.m_corners[0] ];
		minH = maxH = corner0[2];
		
		for (int n=1; n<4; ++n)
		{
			Vec4& corner = m_mesh[ square.m_corners[n] ];
			H = corner[2];
			if (H>maxH) maxH = H;
			if (H<minH) minH = H;
		}	
		
		// store coords of center of square
		Vec4& C = square.m_center;
		C.setZero3();
		for (int n=0; n<4; ++n)
		{
			Vec4& corner = m_mesh[ square.m_corners[n] ];
			C += corner;
		}
		C /= 4.f;
		
		if (fabs(maxH-minH) < FLATNESS_TOL)
		{
			m_flat.push_back(square.m_indx);
			square.m_flat = true;
		}
		else
		{
			square.m_flat = false;
		}
		
		++iter;		
	}	
}

void Terrain::mesh_findLowestSquares()
{
	//find lowest flat-square height
	bool first = true;
	float minH = MAXFLOAT;
	
	vector<int>::iterator iter = m_flat.begin();
	while (iter != m_flat.end())
	{
		int squareIndx = (*iter);
		Square& square = m_squares[squareIndx];
		float H = square.m_center[2];
		
		if (first || H<=minH)
		{
			minH = H;
			first = false;
		}
		iter++;		
	}
	
	m_lowSquares.clear();
	iter = m_flat.begin();
	while (iter != m_flat.end())
	{
		int squareIndx = (*iter);
		Square& square = m_squares[squareIndx];
		if (square.m_center[2] == minH)
		{
			m_lowSquares.push_back(squareIndx);
		}
		iter++;	
	}
}


void Terrain::mesh_findHighestSquares()
{
	//find highest flat-square height
	bool first = true;
	float maxH = MAXFLOAT;
	
	vector<int>::iterator iter = m_flat.begin();
	while (iter != m_flat.end())
	{
		int squareIndx = (*iter);
		Square& square = m_squares[squareIndx];
		float H = square.m_center[2];
		
		if (first || H>maxH)
		{
			maxH = H;
			first = false;
		}
		iter++;		
	}
	
	m_highSquares.clear();
	iter = m_flat.begin();
	while (iter != m_flat.end())
	{
		int squareIndx = (*iter);
		Square& square = m_squares[squareIndx];
		if (square.m_center[2] == maxH)
		{
			m_highSquares.push_back(squareIndx);
		}
		iter++;	
	}	
}

 
const float Ta = 0.004f;
const float Tb = 1.0f - Ta;
const float UVs[4][2] = {{Ta,Ta}, {Tb,Ta}, {Ta,Tb}, {Tb,Tb}};

static void _setTriData( Square::Triangle& t, const int* indx, Square& square, const vector<Vec4>& m_mesh )
{
	for (int i=0; i<3; ++i)
	{
		t.m_indx[i] = square.m_corners[indx[i]];
		t.m_u[i] = UVs[indx[i]][0]; 
		t.m_v[i] = UVs[indx[i]][1];
	}
	
	t.m_n = computeNormal( t, m_mesh );
}

// make mesh triangles
void Terrain::mesh_makeTris()
{
	int indx1[3]  = {0,1,3}; //normal config
	int indx2[3]  = {0,3,2};
	
	int indx1p[3] = {0,1,2}; //switched config
	int indx2p[3] = {1,3,2};
	
	for (int i=0; i<m_numSquaresPerEdge; ++i)
	for (int j=0; j<m_numSquaresPerEdge; ++j)
	{
		int indx = i + m_numSquaresPerEdge*j;
		Square& square = m_squares[indx];
		
		_setTriData(square.m_t1, indx1, square, m_mesh );
		_setTriData(square.m_t2, indx2, square, m_mesh );
		
		// need to patch up mesh so that wall squares with 1 flat triangle
		// are changed to other configuration, to get rid of flat triangles adjacent to flat squares
		if (!square.m_flat)
		{
			if ( _isFlat( square.m_t1, m_mesh ) || _isFlat( square.m_t2, m_mesh ) )
			{
				_setTriData(square.m_t1, indx1p, square, m_mesh );
				_setTriData(square.m_t2, indx2p, square, m_mesh );
			}
		}
	}		
}

// make walls
void Terrain::mesh_makeWalls()
{
	for (int i=0; i<m_numSquaresPerEdge; ++i)
	for (int j=0; j<m_numSquaresPerEdge; ++j)
	{
		int indx = i + m_numSquaresPerEdge*j;
		Square& square = m_squares[indx];
		
		if (!square.m_flat)
		{
			
#ifndef CREASED_LOOK
			if ( square.isPlanar(m_mesh) )
			{
				m_planeWalls.push_back(square.m_indx);
			}
			else
#endif
				
			{
				m_creasedWalls.push_back(square.m_indx);
			}
		}
	}
}

static void _rotateUVs(Square& square)
{
	/* reassign UVs to vertices so that texture is rotated to align slash with 03
	2 (01) 3 (11)  -> 3 (01) 1 (11)
	0 (00) 1 (10)     2 (00) 0 (10)
	*/
	
	// triangle 013
	square.m_t1.m_u[0] = Tb; square.m_t1.m_v[0] = Ta;
	square.m_t1.m_u[1] = Tb; square.m_t1.m_v[1] = Tb;
	square.m_t1.m_u[2] = Ta; square.m_t1.m_v[2] = Tb;
	
	// triangle 032
	square.m_t2.m_u[0] = Tb; square.m_t2.m_v[0] = Ta;
	square.m_t2.m_u[1] = Ta; square.m_t2.m_v[1] = Tb;
	square.m_t2.m_u[2] = Ta; square.m_t2.m_v[2] = Ta;				
}


// alter texture coords of walls so 'slashes' look nice 
void Terrain::mesh_prettification()
{
	// make plane walls have horizontal slash for 'contour' look
	vector<int>::iterator iterPlaneWalls = m_planeWalls.begin();
	
	while (iterPlaneWalls != m_planeWalls.end())
	{
		int squareIndx = *iterPlaneWalls;
		Square& square = m_squares[squareIndx];
		
		//if one of the diagonals is horizontal (both can't be, otherwise this would be a flat square..),
		//put slash along it. Since slash is along 12 by default, switch if find 03 is horizontal.
		Vec4& v0 = m_mesh[ square.m_corners[0] ];
		Vec4& v3 = m_mesh[ square.m_corners[3] ];
		
		if ( fabs(v0[2]-v3[2]) < FLATNESS_TOL )
		{
			_rotateUVs(square);
		}
		
		++iterPlaneWalls;
	}
		
	// make non-plane walls have slash along the crease
	vector<int>::iterator iterCreasedWalls = m_creasedWalls.begin();
	
	while (iterCreasedWalls != m_creasedWalls.end())
	{
		int squareIndx = *iterCreasedWalls;
		Square& square = m_squares[squareIndx];
		
		//put slash along triangle crease, which is 03 edge or 12 edge, determined by whether 
		//triangle 1 contains 03 edge. Slash is along 12 by default, so switch if find 03.
		Square::Triangle& t1 = square.m_t1;
		
		int m0 =  square.m_corners[0];
		int m3 =  square.m_corners[3];
		bool contains0 = ( (t1.m_indx[0] == m0) || (t1.m_indx[1] == m0) || (t1.m_indx[2] == m0) );
		bool contains3 = ( (t1.m_indx[0] == m3) || (t1.m_indx[1] == m3) || (t1.m_indx[2] == m3) );
		
		if ( contains0 && contains3 )
		{
			_rotateUVs(square);
		}
		++iterCreasedWalls;
	}
}

void Terrain::mesh_setRadii()
{
	// store bounding radius of each square
	for (int i=0; i<m_numSquaresPerEdge; ++i)
	for (int j=0; j<m_numSquaresPerEdge; ++j)
	{
		int indx = i + m_numSquaresPerEdge*j;
		Square& square = m_squares[indx];	
		
		computeSquareRad( square, m_mesh );
	}			
}


