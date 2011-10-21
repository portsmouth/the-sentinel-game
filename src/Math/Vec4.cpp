

#include "Vec4.h"

#include <string.h>
#include <cmath>


Vec4::Vec4()
{
	m_v[0] = 0.f;
	m_v[1] = 0.f;
	m_v[2] = 0.f;
	m_v[3] = 1.f;
}

Vec4::Vec4( float x, float y, float z, float w )
{
	m_v[0] = x;
	m_v[1] = y;
	m_v[2] = z;
	m_v[3] = w;
}

Vec4::Vec4 (float *v)
{
	memcpy(m_v, v, 3*sizeof(float));
	m_v[3] = 0.f;
}

// copy constructor
Vec4::Vec4( const Vec4& other )
{
	for (int a=0; a<4; ++a)
	{
		m_v[a] = other.m_v[a];
	}
}

//magnitude
float Vec4::length3(void)
{
	return sqrt( (*this) * (*this) );
}


//normalize
void Vec4::normalizeIfNotZero()
{
	float magsqr = (*this) * (*this);
	if (magsqr > 0.f)
	{
		if (magsqr == 1.0f) return; // already normalized
		float mag = sqrt(magsqr);
		for (int a=0; a<3; ++a)
		{
			m_v[a] /= mag;
		}
	}
}

//zero x,y,z
void Vec4::setZero3()
{
	for (int a=0; a<3; ++a)
	{
		m_v[a] = 0.f;
	}
}

//zero x,y,z,w
void Vec4::setZero4()
{
	for (int a=0; a<4; ++a)
	{
		m_v[a] = 0.f;
	}
}





