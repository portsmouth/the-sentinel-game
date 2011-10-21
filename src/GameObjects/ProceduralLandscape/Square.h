
#ifndef SQUARE_H
#define SQUARE_H

#include "Vec4.h"
#include <vector>
using std::vector;

const float FLATNESS_TOL = 1.e-6f;
class GameObject;

class Square
{
	
public:
	
	Square( int i, int j, int indx, int numSquaresPerEdge );
	
	bool intersect(const Vec4& P, const Vec4& D, float* tInsersect, const vector<Vec4>& m_mesh);
	bool isPlanar(const vector<Vec4>& m_mesh);
	
	float m_height;      // height of grid square
	int m_i;             // (i,j) indices of this square in squares grid
	int m_j; 
	int m_indx;          // index of this square in squares array
	bool m_flat;         // whether this square is one of the flat ones
	Vec4 m_center;       // square center
	float m_boundRadius; // bounding radius (square) about m_center, for raycasting
	
	// indices of this square's corners in mesh vertex buffer,
	// in order 0 = BL, 1 = BR, 2 = TL, 3 = TR
	int m_corners[4];   
	
	struct Triangle
	{
		bool intersect(const Vec4& P, const Vec4& D, float* tInsersect, const vector<Vec4>& m_mesh);
		
		// index into m_mesh
		int m_indx[3];
		
		// texture coordinates
		float m_u[3], m_v[3];
		
		// normal
		Vec4 m_n;
	};
	
	Triangle m_t1; // triangle 1 indices
	Triangle m_t2; // triangle 2 indices
	
	/* triangle configuration
		
		2-----3
		| t2 /|
		|   / |
		|  /  |
		| /   |
		|/ t1 |
		0-----1
		
		*/
	
	// This square's occupants, maintained as a vector of GameObject* pointers.
	vector<GameObject*> m_occupants;
	
	// Various occupant related functions
	GameObject* getHighestOccupant();  // return NULL if no occupants
	GameObject* getSynthoidShell();    // return NULL if no synthoid shell occupant
	
	bool createTree();           // return true on successful creation
	bool createSynthoidShell();  // return true on successful creation
	bool addBoulder();           // return true on successful creation
	
};



struct SpaceTriangle
{
	
public:
	
	SpaceTriangle( const Vec4& v0, const Vec4& v1, const Vec4& v2 );
	
	bool intersect(const Vec4& P, const Vec4& D, float* tInsersect);
	
	Vec4 m_v[3];  // normal with winding 012 is defined to point outwards (from backside to frontside)
	
	Vec4 m_n;
};


#endif  //SQUARE_H







