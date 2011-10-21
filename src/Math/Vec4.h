

#ifndef VEC4_H
#define VEC4_H

#include <cstddef>

class Vec4
{
	
public:
	
	Vec4();                                          // Initializes Vec4 to (0, 0, 0, 1)
	Vec4( float x, float y, float z, float w=0.f );  // Initializes Vec4 to (x, y, z, w)
	Vec4 (float *v);                                 // Initializes Vec4 to (v[0], v[1], v[2], 0)  
	Vec4( const Vec4& other );                       // copy constructor     
	
	//assignment
	Vec4& operator=(const Vec4 &rhs)
	{
		for (int a=0; a<4; ++a)
		{
			m_v[a] = rhs[a];
		}
		return *this;
	}
	
	// access to elements
	float &operator[](const size_t);
	const float &operator[](const size_t) const;
	
	// magnitude
	float length3(void);
	
	//normalize
	void normalizeIfNotZero();
		
	//zero
	void setZero3();
	void setZero4();
	
	// addition and subtraction
	friend Vec4& operator+=( Vec4& lhs, const Vec4& rhs );
	friend Vec4 operator+( const Vec4& lhs, const Vec4& rhs );
	
	friend Vec4& operator-=( Vec4& lhs, const Vec4& rhs );
	friend Vec4 operator-( const Vec4& lhs, const Vec4& rhs );
	
	// multiplication and division by scalars
	friend Vec4& operator*=( Vec4& lhs, const float rhs );
	friend Vec4 operator*( const Vec4& lhs, const float rhs );
	friend Vec4 operator*( const float lhs, const Vec4& rhs );
	
	friend Vec4& operator/=( Vec4& lhs, const float rhs );
	friend Vec4 operator/( const Vec4& lhs, const float rhs );
	
	// dot product
	friend float operator*( const Vec4& lhs, const Vec4& rhs );
	
	// cross product
	friend Vec4& operator%=( Vec4& lhs, const Vec4& rhs );
	friend Vec4 operator%( const Vec4& lhs, const Vec4& rhs );
	
	friend bool operator==( const Vec4& lhs, const Vec4& rhs );
	friend bool operator!=( const Vec4& lhs, const Vec4& rhs );

private:
		
	float m_v[4];
	
};


//element access
inline float& Vec4::operator[](const size_t index)
{
	return m_v[index];
}

inline const float& Vec4::operator[](const size_t index) const
{
	return m_v[index];
}


inline bool operator==( const Vec4& lhs, const Vec4& rhs )
{
	bool equal = true;
	for (int a=0; a<4; ++a)
	{
		equal &= ( lhs.m_v[a] == rhs.m_v[a] );
	}
	return equal;
}

inline bool operator!=( const Vec4& lhs, const Vec4& rhs )
{
	return !(lhs==rhs);
}

inline Vec4& operator+=( Vec4& lhs, const Vec4& rhs )
{
	for (int a=0; a<3; ++a)
	{
		lhs.m_v[a] += rhs.m_v[a];
	}
	return lhs;
}

inline Vec4& operator-=( Vec4& lhs, const Vec4& rhs )
{
	for (int a=0; a<3; ++a)
	{
		lhs.m_v[a] -= rhs.m_v[a];
	}
	return lhs;
}

inline Vec4 operator+( const Vec4& lhs, const Vec4& rhs )
{
	Vec4 ret(lhs);
	ret += rhs;
	return ret;
}

inline Vec4 operator-( const Vec4& lhs, const Vec4& rhs )
{
	Vec4 ret(lhs);
	ret -= rhs;
	return ret;
}

inline float operator*( const Vec4& lhs, const Vec4& rhs )
{
	float dot = 0.f;
	for (int a=0; a<3; ++a)
	{
		dot += lhs.m_v[a] * rhs.m_v[a];
	}
	return dot;
}

inline Vec4& operator%=( Vec4& lhs, const Vec4& rhs )
{
	Vec4 tmp = lhs;
	
	tmp.m_v[0] = lhs.m_v[1] * rhs.m_v[2] - lhs.m_v[2] * rhs.m_v[1];
	tmp.m_v[1] = lhs.m_v[2] * rhs.m_v[0] - lhs.m_v[0] * rhs.m_v[2];
	tmp.m_v[2] = lhs.m_v[0] * rhs.m_v[1] - lhs.m_v[1] * rhs.m_v[0];
	
	lhs = tmp;
	return lhs;
}

inline Vec4 operator%( const Vec4& lhs, const Vec4& rhs )
{
	Vec4 ret(lhs);
	ret %= rhs;
	return ret;	
}

inline Vec4& operator*=( Vec4& lhs, const float rhs )
{
	for (int a=0; a<3; ++a)
	{
		lhs.m_v[a] *= rhs;
	}
	return lhs;
}

inline Vec4 operator*( const Vec4& lhs, const float rhs )
{
	Vec4 ret(lhs);
	ret *= rhs;
	return ret;	
}
inline Vec4 operator*( const float lhs, const Vec4& rhs )
{
	Vec4 ret(rhs);
	ret *= lhs;
	return ret;	
}


inline Vec4& operator/=( Vec4& lhs, const float rhs )
{
	if (rhs == 0.f) return lhs;
	for (int a=0; a<3; ++a)
	{
		lhs.m_v[a] /= rhs;
	}
	return lhs;
}

inline Vec4 operator/( const Vec4& lhs, const float rhs )
{
	Vec4 ret(lhs);
	if (rhs == 0.f) return lhs;
	ret /= rhs;
	return ret;	
}


#endif // VEC4_H




