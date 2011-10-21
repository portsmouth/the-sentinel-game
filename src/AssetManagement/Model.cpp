
#include "Model.h"
#include <iostream>

#include "Game.h"
#include "World.h"
#include "terrain.h"
#include "FileUtils.h"


Model::Model(const char* obj_file, shading_modes sm) :

m_shadingMode(sm)

{
	matTotal = 0;		// mat[0] reserved for default meterial
	vTotal = tTotal = nTotal = fTotal = 0;
	
	float default_value[3] = {1,1,1};
	
	vList.push_back( Vec4(default_value) );	
	nList.push_back( Vec4(default_value) );	
	tList.push_back( Vec4(default_value) );
	
	 // default material: mat[0]
	 mat[0].Ka[0] = 0.0; mat[0].Ka[1] = 0.0; mat[0].Ka[2] = 0.0; mat[0].Ka[3] = 1.0; 
	 mat[0].Kd[0] = 1.0; mat[0].Kd[1] = 1.0; mat[0].Kd[2] = 1.0; mat[0].Kd[3] = 1.0; 
	 mat[0].Ks[0] = 0.8; mat[0].Ks[1] = 0.8; mat[0].Ks[2] = 0.8; mat[0].Ks[3] = 1.0;
	 mat[0].Ns = 32;
	 matTotal++;

	int result = LoadModel(string(obj_file));	
	if (result == -1) 
	{
		printf("Error loading model: %s\n", obj_file);
		exit(EXIT_FAILURE);
	}
	
	computeAABB();
	
}

Model::~Model()
{
}


void Model::computeAABB()
{
	vector<Vec4> faceVerts;
	
	for (unsigned int i=0; i<faceList.size(); ++i)
	{
		FACE& face = faceList[i];
		for (int a=0; a<3; ++a)
		{
			const Vec4& V = vList[ face.v[a].v ];
			faceVerts.push_back(V);
		}
	}
	
	m_AABB.findAABB( faceVerts );
}


int Model::LoadModel(string obj_file)
{
	float scale = 0.95f * Game::pWorld->pTerrain->getSquareEdgeSize();
	
	string modelspath = FileUtils::GetApplicationResourcesFolderPath() + std::string("/Models/");
	const char* obj_database = modelspath.c_str();
	
	char	token[100], buf[100], v[5][100];	
	float	vec[3];

	int	n_vertex, n_texture, n_normal;
	int	cur_tex = 0;				// state variable

	fp_scene = fopen(obj_file.c_str(),"r");
	s_file = obj_file;

	if (!fp_scene) 
	{
		cout<< string("Cannot open object File \"") << obj_file << "\" !" << endl;
		return -1;
	}

	cout << endl << obj_file << endl;
		
	while( !feof(fp_scene) )
	{
		token[0] = '\0';
		fscanf(fp_scene, "%s", token);		

		if (!strcmp(token,"g"))
		{
			fscanf(fp_scene,"%s",buf);
		}

		else if (!strcmp(token,"mtllib"))
		{
  			fscanf(fp_scene,"%s", mat_file);
			LoadMaterial(string(obj_database) + string(mat_file));
		}

		else if (!strcmp(token,"usemtl"))
		{
			fscanf(fp_scene,"%s",buf);
			cur_tex = matMap[s_file+string("_")+string(buf)];
		}

		else if (!strcmp(token,"v"))
		{
			fscanf(fp_scene, "%f %f %f", &vec[0], &vec[1], &vec[2]);
			Vec4 v(vec);
			v *= scale;
			
			vList.push_back(v);
		}

		else if (!strcmp(token,"vn"))
		{
			fscanf(fp_scene,"%f %f %f", &vec[0], &vec[1], &vec[2]);
			Vec4 N(vec); N.normalizeIfNotZero();
			nList.push_back(N);
		}
		else if (!strcmp(token,"vt"))
		{
			fscanf(fp_scene, "%f %f", &vec[0], &vec[1]);
			tList.push_back( Vec4(vec) );
		}

		else if (!strcmp(token,"f"))
		{
			for (int i=0;i<3;i++)	
			{
				fscanf(fp_scene,"%s",v[i]);
			}
		
			Vertex tmp_vertex[3];		// for faceList structure

			for (int i=0;i<3;i++)		// for each vertex of this face
			{
				char str[20], ch;
				int base,offset;
				base = offset = 0;

				// calculate vertex-list index
				while( (ch=v[i][base+offset]) != '/' && (ch=v[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				n_vertex = atoi(str);
				base += (ch == '\0')? offset : offset+1;
				offset = 0;

				// calculate texture-list index
				while( (ch=v[i][base+offset]) != '/' && (ch=v[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				n_texture = atoi(str);	
				base += (ch == '\0')? offset : offset+1;
				offset = 0;

				// calculate normal-list index
				while( (ch=v[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				n_normal = atoi(str);	

				tmp_vertex[i].v = n_vertex;
				tmp_vertex[i].t = n_texture;
				tmp_vertex[i].n = n_normal;
				tmp_vertex[i].m = cur_tex;
			}

			faceList.push_back(FACE(tmp_vertex[0], tmp_vertex[1], tmp_vertex[2]));
		}

		else if (!strcmp(token,"#"))	
			fgets(buf, 100, fp_scene);

	}

	if (fp_scene) fclose(fp_scene);

	vTotal = vList.size();
	nTotal = nList.size();
	tTotal = tList.size();
	fTotal = faceList.size();
	
	if (m_shadingMode == FLAT_SHADED)
	{
		genTriangleNormals();
	}
	
	return 1;
}

// replace smooth normals with normals pointing along the normal of each triangle, for a faceted look
void Model::genTriangleNormals()
{
	for (unsigned int i=0; i<faceList.size(); ++i)
	{
		FACE& face = faceList[i];

		const Vec4& v0 = vList[ face.v[0].v ];
		const Vec4& v1 = vList[ face.v[1].v ];
		const Vec4& v2 = vList[ face.v[2].v ];
		
		Vec4 d10 = v0 - v1;
		Vec4 d20 = v2 - v0;
		Vec4 N = d20 % d10;
		N.normalizeIfNotZero();	
	
		Vec4& n0 = nList[ face.v[0].n ];
		Vec4& n1 = nList[ face.v[1].n ];
		Vec4& n2 = nList[ face.v[2].n ];
		
		n0 = N;
		n1 = N;
		n2 = N;
	}
}



 // generate (and replace) UVs with ((0,0), (1,0), (0,1)) per triangle, for really basic (or retro..) texturing
 void Model::genTriangleUVs( float du, int orientation )
 {
	 tList.clear();
		 
	 float dup = 1.0f - du;
	 const float UV0[3][2] = {{du, du}, {dup, dup}, {du, dup}};
	 const float UV1[3][2] = {{du, du}, {dup, du}, {du, dup}};
	 const float UV2[3][2] = {{du, du}, {du, dup}, {dup, du}};
	 
	 float UVs[3][2];
	 
	 switch (orientation)
	 {
		 case 0: 
			 for (int i=0; i<3; ++i)
				 for (int j=0; j<2; ++j) UVs[i][j] = UV0[i][j]; break;
		 case 1: 
			 for (int i=0; i<3; ++i)
				 for (int j=0; j<2; ++j) UVs[i][j] = UV1[i][j]; break;
		 case 2: 
			 for (int i=0; i<3; ++i)
				 for (int j=0; j<2; ++j) UVs[i][j] = UV2[i][j]; break;
	 }
	 
	 for (unsigned int i=0; i<faceList.size(); ++i)
	 {
		 FACE& face = faceList[i];
		 
		 for (int i=0; i<3; ++i)
		 {
			 face.v[i].t = tList.size();
			 
			 Vec4 tv;
			 tv[0] = UVs[i][0];
			 tv[1] = UVs[i][1];
			 
			 tList.push_back(tv);
		 }
	 }
 }


void Model::LoadMaterial(string mat_file)
{
	char	token[100], buf[100];
	float	r,g,b;

	fp_material = fopen(mat_file.c_str(), "r");
	t_file = mat_file;

	if (!fp_material) 
	{
		cout << "Can't open material file \"" << mat_file << "\"!" << endl;
		return;
	}

	cout << mat_file << endl;

	int cur_mat = 0;

	while(!feof(fp_material))
	{
		token[0] = '\0';
		fscanf(fp_material,"%s", token);	
		if (!strcmp(token,"newmtl"))
		{
			fscanf(fp_material,"%s",buf);
			cur_mat = matTotal++;					
			matMap[s_file+string("_")+string(buf)] = cur_mat; 	// matMap["material_name"] = material_id;
		}

		else if (!strcmp(token,"Ka"))
		{
			fscanf(fp_material,"%f %f %f",&r,&g,&b);
			mat[cur_mat].Ka[0] = r;
			mat[cur_mat].Ka[1] = g;
			mat[cur_mat].Ka[2] = b;
			mat[cur_mat].Ka[3] = 1;
		}

		else if (!strcmp(token,"Kd"))
		{
			fscanf(fp_material,"%f %f %f",&r,&g,&b);
			mat[cur_mat].Kd[0] = r;
			mat[cur_mat].Kd[1] = g;
			mat[cur_mat].Kd[2] = b;
			mat[cur_mat].Kd[3] = 1;
		}

		else if (!strcmp(token,"Ks"))
		{
			fscanf(fp_material,"%f %f %f",&r,&g,&b);
			mat[cur_mat].Ks[0] = r;
			mat[cur_mat].Ks[1] = g;
			mat[cur_mat].Ks[2] = b;
			mat[cur_mat].Ks[3] = 1;
		}

		else if (!strcmp(token,"Ns"))
		{
			fscanf(fp_material,"%f",&r);
			mat[cur_mat].Ns = r;
		}

		else if (!strcmp(token,"#"))	
			fgets(buf,100,fp_material);

	}

	printf("total material:%d\n", (int)matMap.size());

	if (fp_material) fclose(fp_material);
}








/*
 void Model::generateGLBuffers()
 {
	 // in header /////
	 
	 // OpenGL buffers for rendering this model
	 void generateGLBuffers();
	 
	 GLfloat* m_vertsBufferGL;     // vertex buffer
	 GLint m_sizeVertsBufferGL;
	 
	 GLushort* m_indexBufferGL;    // index buffer
	 GLint m_sizeIndexBufferGL;
	 
	 GLfloat* m_normsBufferGL;     // normals buffer
	 GLint m_sizeNormsBufferGL;
	 
	 GLfloat* m_texcoordsBufferGL; // texture coords buffer
	 GLint m_sizeTexCoordsBufferGL;
	 
	 GLfloat* m_diffuseColorBufferGL; // diffuse color buffer
	 GLint m_sizeDiffuseBufferGL;
	 
	 GLuint m_MeshID_APPLE;
	 GLuint m_vertexBufferObject;
	 
	 
	 /////////////////
	 
	 
	 
	 // generate a vertex buffer object, and get a pointer to the buffer
	 
	 
	 //  fill vertex buffer...
	 GLfloat* m_vertsBufferGL = new GLfloat[ 3*vList.size() ];
	 m_sizeVertsBufferGL = 3*vList.size();
	 for (unsigned int i=0; i<vList.size(); ++i)
	 {
		 Vec4& v = vList[i];
		 m_vertsBufferGL[3*i  ] = (GLfloat)v[0];
		 m_vertsBufferGL[3*i+1] = (GLfloat)v[1];
		 m_vertsBufferGL[3*i+2] = (GLfloat)v[2];
	 }
	 
	 // go through faces and build index buffer...
	 m_indexBufferGL = new GLushort[ 3*faceList.size() ];
	 m_sizeIndexBufferGL = 3*faceList.size();
	 for (unsigned int i=0; i<faceList.size(); ++i)
	 {
		 FACE& face = faceList[i];
		 m_indexBufferGL[3*i  ] = (GLushort) face.v[0].v;
		 m_indexBufferGL[3*i+1] = (GLushort) face.v[1].v;
		 m_indexBufferGL[3*i+2] = (GLushort) face.v[2].v;
	 }
	 
	 // build normals buffer...
	 // assume here that number of normals = number of vertices, i.e. make sure model has one normal per vertex
	 m_normsBufferGL = new GLfloat[ 3*nList.size() ];
	 m_sizeNormsBufferGL = 3*nList.size();
	 for (unsigned int i=0; i<vList.size(); ++i)
	 {
		 Vec4& n = nList[i];
		 m_normsBufferGL[3*i  ] = (GLfloat)n[0];
		 m_normsBufferGL[3*i+1] = (GLfloat)n[1];
		 m_normsBufferGL[3*i+2] = (GLfloat)n[2];
	 }
	 
	 //  build texture coords buffer...
	 m_texcoordsBufferGL = new GLfloat[ 2*tList.size() ];
	 m_sizeTexCoordsBufferGL = 2*tList.size();
	 for (unsigned int i=0; i<vList.size(); ++i)
	 {
		 Vec4& t = tList[i];
		 m_texcoordsBufferGL[2*i  ] = (GLfloat)t[0];
		 m_texcoordsBufferGL[2*i+1] = (GLfloat)t[1];
	 }
	 
	 // build diffuse color buffer
	 m_diffuseColorBufferGL =  new GLfloat[ 3*vList.size() ];
	 m_sizeDiffuseBufferGL = 3*vList.size();
	 //for (unsigned int i=0; i<faceList.size(); ++i)
	 //{
	 //FACE& face = faceList[i];
	 //m_diffuseColorBufferGL[ face.v[0].v ] = mat[ face.v[0].m ].Kd[0];
		 
		 //}
		 
	 glEnableClientState(GL_VERTEX_ARRAY);
	 glEnableClientState(GL_NORMAL_ARRAY);
	 glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	 glEnableClientState( GL_ELEMENT_ARRAY_APPLE );
	 
	 glGenVertexArraysAPPLE( 1, &m_MeshID_APPLE );
	 glBindVertexArrayAPPLE( m_MeshID_APPLE );
	 
	 glVertexArrayParameteriAPPLE(GL_VERTEX_ARRAY_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
	 
	 glVertexPointer(3, GL_FLOAT, 0, m_vertsBufferGL);
	 glNormalPointer  (   GL_FLOAT, 0, m_normsBufferGL    );
	 glTexCoordPointer(2, GL_FLOAT, 0, m_texcoordsBufferGL);
	 glElementPointerAPPLE( GL_UNSIGNED_SHORT, m_indexBufferGL );
	 
	 glDisableClientState(GL_VERTEX_ARRAY);
	 glDisableClientState(GL_NORMAL_ARRAY);
	 glDisableClientState(GL_TEXTURE_COORD_ARRAY);	
	 glDisableClientState( GL_ELEMENT_ARRAY_APPLE );
	 
	 //GLuint vertex_VBO;
	 //GLuint normal_VBO;
	 //GLuint texcoo_VBO;
	 
	 //glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertex_VBO);
	 //glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_sizeVertsBufferGL * sizeof(GLfloat), m_vertsBufferGL, GL_STATIC_DRAW_ARB);
	 //m_vertsBufferGL = (GLfloat*) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_ONLY_ARB);
	 
	 //index array
	 //glEnableClientState(GL_INDEX_ARRAY);
	 //glIndexPointer(GL_SHORT, 0, m_indexBufferGL);
	 
	 //glEnableClientState( GL_ELEMENT_ARRAY_APPLE );
	 //glElementPointerAPPLE( GL_UNSIGNED_SHORT, m_indexBufferGL );
	 
	 // ask apple driver to cache the vertex buffer on the GPU
	 //glEnableClientState( GL_VERTEX_ARRAY_RANGE_APPLE );
	 //glVertexArrayRangeAPPLE( m_sizeVertsBufferGL * sizeof(GLfloat), m_vertsBufferGL );
	 //glVertexArrayParameteriAPPLE(GL_VERTEX_ARRAY_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
	 
	 //glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	 
	  glVertexPointer(3, GL_FLOAT, 0, &vertex_VBO);
	  glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertex_VBO);
	  glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_sizeVertsBufferGL * sizeof(GLfloat), m_vertsBufferGL, GL_STATIC_DRAW_ARB);
	  m_vertsBufferGL = (GLfloat*) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_ONLY_ARB);
	  
	  glNormalPointer(GL_FLOAT, 0, &normal_VBO);
	  glBindBufferARB(GL_ARRAY_BUFFER_ARB, normal_VBO);
	  glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_sizeNormsBufferGL * sizeof(GLfloat), m_normsBufferGL, GL_STATIC_DRAW_ARB);
	  m_normsBufferGL = (GLfloat*) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_ONLY_ARB);
	  
	  glTexCoordPointer(2, GL_FLOAT, 0, &texcoo_VBO);
	  glBindBufferARB(GL_ARRAY_BUFFER_ARB, texcoo_VBO);
	  glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_sizeTexCoordsBufferGL * sizeof(GLfloat), m_texcoordsBufferGL, GL_STATIC_DRAW_ARB);
	  m_texcoordsBufferGL = (GLfloat*) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_ONLY_ARB);
	  
	 delete [] m_vertsBufferGL;    
	 delete [] m_indexBufferGL;    
	 delete [] m_normsBufferGL;     
	 delete [] m_texcoordsBufferGL; 
	 
 }
*/ 

