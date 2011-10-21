

#ifndef MODEL_H
#define MODEL_H

#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <string>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#include "Maths.h"

using namespace std;


class Material
{
	
public:
	float Ka[4];
	float Kd[4];
	float Ks[4];
	float Ns;		// shininess

	Material()
	{ 
		for (int i=0;i<4;i++)
			Ka[i] = Kd[i] = Ks[i] = 1;
		Ns = 0;
	}
};


enum shading_modes {FLAT_SHADED, SMOOTH_SHADED};

class Model  
{

public: 
	
	class Vertex		
	{
		public:
	
		int v;		// vertex (index of vList)
		int n;		// normal (index of nList)
		int t;		// texture (index of tList)
		int m;		// material (index of material)
		Vertex() {};
		Vertex(int v_index, int n_index, int t_index=0, int m_index=0)
		{
			v = v_index;
			n = n_index;
			t = t_index;
			m = m_index;
		}
	};

	
	class FACE		
	{
		public:
			
		Vertex v[3];
		FACE (Vertex &v1, Vertex &v2, Vertex &v3) 
		{
			v[0] = v1; 
			v[1] = v2;
			v[2] = v3;
		}
		Vertex& operator[](int index)
		{
			return v[index];
		}
		
		const Vertex& operator[](int index) const
		{
			return v[index];
		}
	};

public:

	Model(const char* obj_file, shading_modes sm);
	virtual ~Model();
	
	inline bool isFlatShaded()
	{
		return m_shadingMode == FLAT_SHADED;
	}
	
	void genTriangleUVs( float du = 0.0f, int orientation=0 );
	
	int		matTotal;	       // total material 
	map<string, int> matMap;   // matMap[material_name] = material_ID
	Material	mat[100];	   // material ID	

	vector<Vec4>	vList;		// Vertex List (Position) - world cord.
	vector<Vec4>	nList;		// Normal List
	vector<Vec4>	tList;		// Texture List
	vector<FACE>	faceList;	// Face List

	int	vTotal, tTotal, nTotal, fTotal;
	
	// AABB of triangles in model
	AABB m_AABB;
	
private:
		
	enum shading_modes m_shadingMode;
	
	// Loads a .obj model from disk
	int	LoadModel(string obj_file);
	
	// Loads a .mtl material file from disk (.mtl should be included in directory along with .obj)
	void LoadMaterial(string mat_file);
		
	FILE	*fp_scene, *fp_material;
	string  s_file, t_file;
	char	mat_file[50];	   // material file name
	
	void genTriangleNormals();
	void computeAABB();

	
};

#endif //MODEL_H
