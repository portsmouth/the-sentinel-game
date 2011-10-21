



#ifndef MATRIX4_H
#define MATRIX4_H

#include "Vec4.h"


class Matrix4
{
	
public:
	
	// create a rotation matrix with given axis and angle (in radians)
	void setRotation( const Vec4& axis, float angle );
	
	// right multiply a Matrix4 by a homogeneous Vec4 
	friend Vec4 operator*( const Matrix4& lhs, const Vec4& rhs );
	
private:	
	
	float m_matrix[16];
	
};



#endif // MATRIX4_H
