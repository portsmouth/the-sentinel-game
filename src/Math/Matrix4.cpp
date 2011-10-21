/*
 *  Matrix4.cpp
 *  The Sentinel
 *
 *  Created by Jamie Portsmouth on 29/06/2007.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "Matrix4.h"

#include <cmath>


// create a rotation matrix with given axis and angle
void Matrix4::setRotation( const Vec4& axis, float angle )
{
	float sin_angle = sin(angle);
	float cos_angle = cos(angle);
	float one_minus_cos = 1.0f - cos_angle;
	float x = axis[0];
	float y = axis[1];
	float z = axis[2];
	
	m_matrix[ 0] = (x * x * one_minus_cos) + cos_angle;
	m_matrix[ 1] = (y * x * one_minus_cos) + (z * sin_angle);
	m_matrix[ 2] = (x * z * one_minus_cos) - (y * sin_angle);
	m_matrix[ 3] = 0.0f;
	m_matrix[ 4] = (x * y * one_minus_cos) - (z * sin_angle);
	m_matrix[ 5] = (y * y * one_minus_cos) + cos_angle;
	m_matrix[ 6] = (y * z * one_minus_cos) + (x * sin_angle);
	m_matrix[ 7] = 0.0f;
	m_matrix[ 8] = (x * z * one_minus_cos) + (y * sin_angle);
	m_matrix[ 9] = (y * z * one_minus_cos) - (x * sin_angle);
	m_matrix[10] = (z * z * one_minus_cos) + cos_angle;
	m_matrix[11] = 0.0f;
	m_matrix[12] = 0.0f;
	m_matrix[13] = 0.0f;
	m_matrix[14] = 0.0f;
	m_matrix[15] = 1.0f;
}

// right multiply a Matrix4 by a homogeneous Vec4 
Vec4 operator*( const Matrix4& lhs, const Vec4& rhs )
{
	Vec4 ret;
	ret.setZero4();
	
	int i, j;
	float v;
	for (j=0; j<4; ++j)
	{			
		i = 4*j;
		v = rhs[j];
		ret[0] += lhs.m_matrix[i++] * v;
		ret[1] += lhs.m_matrix[i++] * v;
		ret[2] += lhs.m_matrix[i++] * v;
		ret[3] += lhs.m_matrix[i  ] * v;
	}
	
	return ret;
}








