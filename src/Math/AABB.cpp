

#include "AABB.h"
#include "geometry.h"


AABB::AABB()
{
	
}

AABB::AABB( const vector<Vec4>& vList )
{
	findAABB(vList);	
}


void AABB::findAABB( const vector<Vec4>& vList )
{
	if (vList.size()<1) return;
	
	m_max = vList[0];
	m_min = vList[0];
	
	for (unsigned int n=0; n<vList.size(); ++n)
	{
		const Vec4& v = vList[n];
		for (int a=0; a<3; ++a)
		{
			if ( v[a] > m_max[a] ) m_max[a] = v[a];
			if ( v[a] < m_min[a] ) m_min[a] = v[a];
		}
	}	
}

void AABB::addVertexToAABB( Vec4& v )
{
	for (int a=0; a<3; ++a)
	{
		if ( v[a] > m_max[a] ) m_max[a] = v[a];
		if ( v[a] < m_min[a] ) m_min[a] = v[a];
	}
}

void AABB::expand( float factor )
{
	Vec4 diff = m_max - m_min;
	diff *= 0.5f * factor;
	
	Vec4 center = 0.5f * (m_max + m_min);
	m_max = center + diff;
	m_min = center - diff;
}

AABB AABB::transform( const Vec4& translation, float rotation ) const
{
	AABB transformedAABB;
	
	Matrix4 M;
	Vec4 Z(0,0,1);
	M.setRotation( Z, degToRad * rotation );
	
	transformedAABB.m_max = M * m_max;
	transformedAABB.m_min = M * m_min;
	
	transformedAABB.m_max += translation;
	transformedAABB.m_min += translation;
	
	return transformedAABB;
}


// for debugging purposes only 
void AABB::draw() const
{
	Vec4 center = 0.5f * (m_max + m_min);
	Vec4 halfExtents = 0.5f * (m_max - m_min);

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();
	
	glTranslatef(center[0], center[1], center[2]);
	
	glLineStipple(1,0xF000);
	glEnable(GL_LINE_STIPPLE);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	
	drawBox(&halfExtents[0]);
	
	glDisable(GL_LINE_STIPPLE);
	
	glPopMatrix();
	glPopAttrib();
}


static inline bool _computeFaceIntersection( int face, const float *planes, 
											  const Vec4& o, const Vec4& n, float *t )
{
	int i = face/2;
	
	if ( n[i] == 0.0f ) return 0;
	
	float lambda = ( planes[face] - o[i] ) / n[i];
	
	int a_indx = (i+1) % 3;
	int b_indx = (i+2) % 3;
	
	float a = lambda * n[a_indx] + o[a_indx];
	float b = lambda * n[b_indx] + o[b_indx];
	
	bool aHit = ( a > planes[a_indx*2] ) && ( a < planes[a_indx*2+1] );
	bool bHit = ( b > planes[b_indx*2] ) && ( b < planes[b_indx*2+1] );
	
	if ( aHit && bHit )
	{	
		*t = lambda;
		return 1;
	}
	
	return 0;
}

bool AABB::raycast( const Vec4& P, const Vec4& D, float* t ) const
{
	float planes[6];
	int i;
	for (i=0; i<3; ++i)
	{
		planes[2*i]   = m_min[i];
		planes[2*i+1] = m_max[i];
	}
	
	// 6 faces, indexed as: -X = 0, +X = 1, -Y = 2, +Y = 3, -Z = 4, +Z = 5 
	// note the index of the normal axis is given by face/2 (face = 0,..,5)
	bool hitface[6];
	
	// if there was a hit on a particular face, record the ray distance to that hit from the origin
	float hit_dist[6];
	
	int face;
	for (face=0; face<6; ++face) 
	{
		hitface[face] = _computeFaceIntersection( face, planes, P, D, &hit_dist[face] );
	}
	
	// If the box was hit, there must have been two face hits. Find which faces
	int hits = 0;
	int hit_faces[2];
	for (face = 0; face<6; face++) 
	{
		if ( hitface[face] ) 
		{
			hit_faces[hits++] = face;
			if (hits>1) break;
		}
	}
	
	if (hits != 2)
	{
		//no intersection
		return false;
	}
	
	//given intersection distances, determine what happened
	float t0 = hit_dist[ hit_faces[0] ];
	float t1 = hit_dist[ hit_faces[1] ];
	
	if ( ( t0 < 0.0f ) && ( t1 < 0.0f ) )
	{
		//intersections behind start of ray
		return false;
	}
	
	if ( ( t0 > 0.0f ) && ( t1 > 0.0f ) )
	{
		*t = fminf(t0, t1);
		return true;
	}
		
	return false;
}



#if 0
 
 typedef unsigned int udword;
 
 //! Integer representation of a floating-point value.
#define IR(x)	((udword&)x)
 
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 /**
 *	A method to compute a ray-AABB intersection.
  *	Original code by Andrew Woo, from "Graphics Gems", Academic Press, 1990
  *	Optimized code by Pierre Terdiman, 2000 (~20-30% faster on my Celeron 500)
  *	Epsilon value added by Klaus Hartmann. (discarding it saves a few cycles only)
  *
  *	Hence this version is faster as well as more robust than the original one.
  *
  *	Should work provided:
  *	1) the integer representation of 0.0f is 0x00000000
  *	2) the sign bit of the float is the most significant one
  *
  *	Report bugs: p.terdiman@codercorner.com
  *
  *	\param		aabb		[in] the axis-aligned bounding box
  *	\param		origin		[in] ray origin
  *	\param		dir			[in] ray direction
  *	\param		coord		[out] impact coordinates
  *	\return		true if ray intersects AABB
  */
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define RAYAABB_EPSILON 0.00001f
 bool Meshmerizer::RayAABB(const AABB& aabb, const Point& origin, const Point& dir, Point& coord)
 {
	 BOOL Inside = TRUE;
	 Point MinB = aabb.mCenter - aabb.mExtents;
	 Point MaxB = aabb.mCenter + aabb.mExtents;
	 Point MaxT;
	 MaxT.x=MaxT.y=MaxT.z=-1.0f;
	 
	 // Find candidate planes.
	 for(udword i=0;i<3;i++)
	 {
		 if(origin.m[i] < MinB.m[i])
		 {
			 coord.m[i]	= MinB.m[i];
			 Inside		= FALSE;
			 
			 // Calculate T distances to candidate planes
			 if(IR(dir.m[i]))	MaxT.m[i] = (MinB.m[i] - origin.m[i]) / dir.m[i];
		 }
		 else if(origin.m[i] > MaxB.m[i])
		 {
			 coord.m[i]	= MaxB.m[i];
			 Inside		= FALSE;
			 
			 // Calculate T distances to candidate planes
			 if(IR(dir.m[i]))	MaxT.m[i] = (MaxB.m[i] - origin.m[i]) / dir.m[i];
		 }
	 }
	 
	 // Ray origin inside bounding box
	 if(Inside)
	 {
		 coord = origin;
		 return true;
	 }
	 
	 // Get largest of the maxT's for final choice of intersection
	 udword WhichPlane = 0;
	 if(MaxT.m[1] > MaxT.m[WhichPlane])	WhichPlane = 1;
	 if(MaxT.m[2] > MaxT.m[WhichPlane])	WhichPlane = 2;
	 
	 // Check final candidate actually inside box
	 if(IR(MaxT.m[WhichPlane])&0x80000000) return false;
	 
	 for(i=0;i<3;i++)
	 {
		 if(i!=WhichPlane)
		 {
			 coord.m[i] = origin.m[i] + MaxT.m[WhichPlane] * dir.m[i];
#ifdef RAYAABB_EPSILON
			 if(coord.m[i] < MinB.m[i] - RAYAABB_EPSILON || coord.m[i] > MaxB.m[i] + RAYAABB_EPSILON)	return false;
#else
			 if(coord.m[i] < MinB.m[i] || coord.m[i] > MaxB.m[i])	return false;
#endif
		 }
	 }
	 return true;	// ray hits box
 }
 
 
#endif