
#include "Square.h"

#include "GameObject.h"
#include "Maths.h"


/***********************************************************************/
/*                            Triangle                                 */
/***********************************************************************/

// ray direction D must be normalized
bool Square::Triangle::intersect(const Vec4& P, const Vec4& D, float* tInsersect, const vector<Vec4>& m_mesh)
{
	float denom = m_n * D;
	if (denom == 0.f) return false;
	
	const Vec4& p0 = m_mesh[ m_indx[0] ];
	const Vec4& p1 = m_mesh[ m_indx[1] ];
	const Vec4& p2 = m_mesh[ m_indx[2] ];
	
	float t = - ( m_n*P - m_n*p0 ) / denom;
	if (t<0.0f) return false;
	
	Vec4 I = P + t * D;	
	Vec4 diff0 = p0 - I;
	Vec4 diff1 = p1 - I;
	Vec4 diff2 = p2 - I;
	
	float s0 = (diff0 % diff1) * m_n;
	float s1 = (diff1 % diff2) * m_n;
	float s2 = (diff2 % diff0) * m_n;
	
	bool hit;
	if (s0 >= 0.f)
	{
		hit = ( s1 >= 0.f && s2 >= 0.f );
	}
	else
	{
		hit = ( s1 <= 0.f && s2 <= 0.f );
	}
	
	if (hit)
	{
		*tInsersect = t;
		return true;
	}
	
	return false;
}


// triangles without an underlying mesh, for sundry stuff like raycasting against the Sentinel plinth

SpaceTriangle::SpaceTriangle( const Vec4& v0, const Vec4& v1, const Vec4& v2 )
{
	m_v[0] = v0;
	m_v[1] = v1;
	m_v[2] = v2;
	
	// winding 0->1->2 gives normal by right hand rule
	Vec4 d10 = v1 - v0;
	Vec4 d20 = v2 - v0;
	m_n = d10 % d20;
	m_n.normalizeIfNotZero();	
}

bool SpaceTriangle::intersect(const Vec4& P, const Vec4& D, float* tInsersect)
{
	float denom = m_n * D;
	if (denom == 0.f) return false;
		
	float t = - ( m_n*P - m_n*m_v[0] ) / denom;
	if (t<0.0f) return false;
	
	Vec4 I = P + t * D;	
	Vec4 diff0 = m_v[0] - I;
	Vec4 diff1 = m_v[1] - I;
	Vec4 diff2 = m_v[2] - I;
	
	float s0 = (diff0 % diff1) * m_n;
	float s1 = (diff1 % diff2) * m_n;
	float s2 = (diff2 % diff0) * m_n;
	
	bool hit;
	if (s0 >= 0.f)
	{
		hit = ( s1 >= 0.f && s2 >= 0.f );
	}
	else
	{
		hit = ( s1 <= 0.f && s2 <= 0.f );
	}
	
	if (hit)
	{
		*tInsersect = t;
		return true;
	}
	
	return false;	
}


/***********************************************************************/
/*                            Square                                   */
/***********************************************************************/

Square::Square( int i, int j, int indx, int numSquaresPerEdge ) :

m_i(i),
m_j(j),
m_indx(indx)

{
	//assert(m_indx = m_j + numSquaresPerEdge * m_i);
	int MESH_EGDE_SIZE = numSquaresPerEdge + 1;
	
	m_corners[0] = m_j + MESH_EGDE_SIZE * m_i;
	m_corners[1] = m_corners[0] + 1;
	m_corners[2] = m_corners[0] + MESH_EGDE_SIZE;
	m_corners[3] = m_corners[2] + 1;
}

// ray direction D must be normalized
bool Square::intersect(const Vec4& P, const Vec4& D, float* tInsersect, const vector<Vec4>& m_mesh)
{
	// test against bounding sphere first
	Vec4 O = P - m_center;
	float On = O * D;
	if ( On > 0.0f )
	{
		return false;
	}
	
	float det = O*O - On*On;
	if ( m_boundRadius < det )
	{
		return false;
	}
	
	// return minimum distance to intersection with both triangles
	float t1, t2;
	bool i1 = m_t1.intersect(P, D, &t1, m_mesh);
	bool i2 = m_t2.intersect(P, D, &t2, m_mesh);
	
	if ( !i1 && !i2 ) return false;
	if ( i1 && i2 ) 
	{
		*tInsersect = fminf(t1, t2);
		return true;
	}
	if (i1)
	{
		*tInsersect = t1;
		return true;
	}
	else
	{
		*tInsersect = t2;
		return true;
	}
}



// test whether triangles in square lie in same plane
bool Square::isPlanar(const vector<Vec4>& m_mesh)
{
	float cs = m_t1.m_n * m_t2.m_n;
	
	return ( fabs(cs - 1.0f) < FLATNESS_TOL );
}


// TO DO
bool Square::createTree()
{
	
	return false;
}

GameObject* Square::getHighestOccupant()
{
	vector<GameObject*>::iterator iter_occupants = m_occupants.begin();
	
	bool first = true;
	float maxH;
	GameObject* highest = NULL;
	
	while (iter_occupants != m_occupants.end())
	{
		GameObject* object = *iter_occupants;
		const Vec4& t = object->getTranslation();
		
		if (first) 
		{
			maxH = t[2];
			highest = object;
			first = false;
		}
		else
		{
			if (t[2] > maxH) 
			{
				maxH = t[2];
				highest = object;
			}
		}
		iter_occupants++;
	}
	
	return highest;
}


bool Square::createSynthoidShell()
{
	// Can only create synthoid on an empty square or on a stack of boulders
	if (m_occupants.size() != 0)
	{
		vector<GameObject*>::iterator iter_occupants = m_occupants.begin();
		while (iter_occupants != m_occupants.end())
		{
			GameObject* object = *iter_occupants;
			if (object->getType() != OBJECT_BOULDER) return false;
			iter_occupants++;
		}
	}
	
	Vec4 synthoidTranslation = m_center;
	
	if (m_occupants.size() != 0)
	{
		GameObject* highestBoulder = getHighestOccupant();
		if (!highestBoulder) return false;  
		
		// Do not allow creation if the highest boulder is currently absorbing
		if (highestBoulder->getState() == OBJECT_ABSORBING) return false;
		
		const Vec4& t =  highestBoulder->getTranslation();
		
		//Boulder* boulder = static_cast<Boulder*>(highestBoulder);
		synthoidTranslation[2] = t[2] + Boulder::getHeight();
	}

	Synthoid* synthoid = new Synthoid( m_indx, synthoidTranslation );
	m_occupants.push_back( (GameObject*)synthoid );
	
	return true;
}


GameObject* Square::getSynthoidShell()
{
	vector<GameObject*>::iterator iter_occupants = m_occupants.begin();
	while (iter_occupants != m_occupants.end())
	{
		GameObject* object = *iter_occupants;
		if (object->getType() == OBJECT_SYNTHOID_SHELL) return object;
		iter_occupants++;
	}
	
	return NULL;
}

bool Square::addBoulder()
{
	// Can only create boulder on an empty square or on a stack of boulders
	if (m_occupants.size() != 0)
	{
		vector<GameObject*>::iterator iter_occupants = m_occupants.begin();
		while (iter_occupants != m_occupants.end())
		{
			GameObject* object = *iter_occupants;
			if (object->getType() != OBJECT_BOULDER) return false;
			iter_occupants++;
		}
	}
	
	Vec4 boulderTranslation = m_center;
	
	if (m_occupants.size() != 0)
	{
		GameObject* highestBoulder = getHighestOccupant();
		if (!highestBoulder) return false;  
		
		// Do not allow creation if the highest boulder is currently absorbing
		if (highestBoulder->getState() == OBJECT_ABSORBING) return false;
		
		const Vec4& t =  highestBoulder->getTranslation();
		
		boulderTranslation[2] = t[2] + Boulder::getHeight();
	}

	Boulder* boulder = new Boulder( m_indx, boulderTranslation );
	m_occupants.push_back( (GameObject*)boulder );
	
	return true;
}

