


#ifndef AABB_H
#define AABB_H

#include "Vec4.h"

#include <vector>
using namespace std;

struct AABB
{
	AABB();
	
	AABB( const vector<Vec4>& vList );

	void findAABB( const vector<Vec4>& vList );
	
	void addVertexToAABB( Vec4& v );
	
	void draw() const;
	
	void expand( float factor );
	
	// return a new AABB by transforming the points of this AABB (rotation in degrees)
	AABB transform( const Vec4& translation, float rotation ) const;
	
	bool raycast( const Vec4& P, const Vec4& D, float* t ) const;
	
	Vec4 m_max;
	Vec4 m_min;
};
	
	
#endif //AABB_H