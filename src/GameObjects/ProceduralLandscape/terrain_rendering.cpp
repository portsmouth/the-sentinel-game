
#include "terrain.h"
#include "Player.h"
#include "GameObject.h"
#include "TextureLibrary.h"
#include "FileUtils.h"

using std::string;




/********************************************************************************/
/*                          Terrain rendering                                   */
/********************************************************************************/

// Toggle terrain shader

//#define USE_SHADERS

#ifdef USE_SHADERS
#include "GbDefines.hh"
#include "GLSLShader.hh"
static GLSLShader shader;
#endif


void Terrain::loadAssets()
{
	// TEXTURES
	TextureLibrary *texturelibrary = TextureLibrary::Instance();
	
	string planewalls_texturepath = FileUtils::GetApplicationResourcesFolderPath()   + kTerrain_planeWalls;
	string creasedwalls_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_creasedWalls;
	string flat_texturepath  = FileUtils::GetApplicationResourcesFolderPath()        + kTerrain_flatGround;
	
	m_planeWallTexture = texturelibrary->GetTexture(planewalls_texturepath, BUILD_MIPMAPS);
	m_creasedWallTexture = texturelibrary->GetTexture(creasedwalls_texturepath, BUILD_MIPMAPS);
	m_flatTexture = texturelibrary->GetTexture(flat_texturepath, BUILD_MIPMAPS);
	
#ifdef USE_SHADERS
	printf("loading terrain shader\n");
	
	string shader1_vert = FileUtils::GetApplicationResourcesFolderPath() + string("/Shaders/terrain.vs");
	string shader1_frag = FileUtils::GetApplicationResourcesFolderPath() + string("/Shaders/terrain.fs");
	
	shader.loadFromFile( shader1_vert.c_str(), shader1_frag.c_str() );
 	shader.bind();
 	shader.uniform("BlackBorder",0.03f);
 	shader.uniform("InteriorRange",0.05f,0.3f);
 	shader.release();
#endif
}


// terrain has its own display lists separate from DisplayHandler, since it never changes within a level

static inline void drawTriangle( const Square::Triangle t, const Vec4& n, const vector<Vec4>& m_mesh )
{
	const Vec4& v0 = m_mesh[ t.m_indx[0] ];
	const Vec4& v1 = m_mesh[ t.m_indx[1] ];
	const Vec4& v2 = m_mesh[ t.m_indx[2] ];
	
	glNormal3f( n[0], n[1], n[2] );
	
	glTexCoord2f( t.m_u[0], t.m_v[0] );
	glVertex3f( v0[0], v0[1], v0[2] );
	
	glTexCoord2f( t.m_u[1], t.m_v[1] );
	glVertex3f( v1[0], v1[1], v1[2] );
	
	glTexCoord2f( t.m_u[2], t.m_v[2] );
	glVertex3f( v2[0], v2[1], v2[2] );
}

static inline void drawTriangle_highlight( const Square::Triangle t, const vector<Vec4>& m_mesh )
{
	const Vec4& v0 = m_mesh[ t.m_indx[0] ];
	const Vec4& v1 = m_mesh[ t.m_indx[1] ];
	const Vec4& v2 = m_mesh[ t.m_indx[2] ];
	
	const float offset = 0.001f;
	
	glVertex3f( v0[0], v0[1], v0[2] + offset );
	glVertex3f( v1[0], v1[1], v1[2] + offset );
	glVertex3f( v2[0], v2[1], v2[2] + offset );
}

static inline void drawTriangleNorm( const Square::Triangle t, const Vec4& n, const vector<Vec4>& m_mesh )
{
	const Vec4& v0 = m_mesh[ t.m_indx[0] ];
	const Vec4& v1 = m_mesh[ t.m_indx[1] ];
	const Vec4& v2 = m_mesh[ t.m_indx[2] ];
	
	//centroid
	Vec4 C = (v0 + v1 + v2)/3.f;
	Vec4 normTip = C + 0.05f*n;
	glVertex3f( C[0], C[1], C[2] );
	glVertex3f( normTip[0], normTip[1], normTip[2] );
}

const float ambient[4]  = {0.1f, 0.1f, 0.1f, 1.f};	
const float specular[4] = {0.0f, 0.0f, 0.0f, 1.f};	
const float diffuse_flat[4]  = {1.f, 1.f, 1.f, 1.f};
const float diffuse_wall[4]  = {255.0f/255.0f, 231.0f/255.0f, 121.0f/255.0f, 1.f};	
const float emission_flat[4] = {0.5f, 1.f, 1.f, 1.f};
const float emission_wall[4] = {0.1f*225.0f/255.0f, 0.1f*150.0f/255.0f, 0.1f*15.0f/255.0f, 1.f};	
const float shine[1] = { 20.0 };


void Terrain::draw(void)
{
	// winding seems to be opposite to normal...
	glFrontFace(GL_CW);
	
	//draw walls
#ifdef USE_SHADERS
	shader.bind();
	shader.uniform("TileColor0",229.0f/255.0f,122.0f/255.0f, 28.0f/255.0f,1.0f);
	shader.uniform("TileColor1",233.0f/255.0f,229.0f/255.0f,202.0f/255.0f,1.0f);
	shader.uniform("Diagonal",1.0f);
#else
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
	
	//glEnable(GL_FOG);	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	
	glColor3f(1.f,1.f,1.f);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_wall);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission_wall);
	m_planeWallTexture->Apply();
#endif
	glCallList(	m_planeWallList );
	
#ifdef USE_SHADERS
	shader.uniform("TileColor0",243.0f/255.0f, 95.0f/255.0f, 30.0f/255.0f,1.0f);
	shader.uniform("TileColor1",255.0f/255.0f,255.0f/255.0f,255.0f/255.0f,1.0f);
	shader.uniform("Diagonal",0.0f);
#else
	m_creasedWallTexture->Apply();
#endif	
	glCallList(	m_creasedWallList );
	
	//draw flat squares - TO DO: checkboard with red/blue emission
#ifdef USE_SHADERS
	shader.uniform("TileColor0", 55.0f/255.0f, 93.0f/255.0f,200.0f/255.0f,1.0f);
	shader.uniform("TileColor1",218.0f/255.0f,218.0f/255.0f,218.0f/255.0f,1.0f);
	shader.uniform("Diagonal",1.0f);
#else
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse_flat);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission_flat);
	m_flatTexture->Apply();
#endif	
	glCallList(	m_flatList );
	
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	//glDisable(GL_DEPTH_TEST);
	
	int hit = Player::Instance()->getTargetedSquare();
	if (hit > -1)
	{
		glBegin(GL_TRIANGLES);
		glColor3f(1,0,1);
		Square& square = m_squares[hit];
		if (square.m_flat) highlightSquare(hit);
		glEnd();
	}
	
	//drawBoulderAABBs_DEBUG();
	//drawPlinthTop_DEBUG;
	//drawBresenhamBand_DEBUG();
	//drawOccupiedSquares_DEBUG();
	//drawNormals_DEBUG();
	
	//reverse this hack:
	glFrontFace(GL_CCW);
	
#ifdef USE_SHADERS
	shader.release();
#else	
	glPopAttrib();
#endif
}

void Terrain::createDisplayLists()
{
	m_planeWallList = glGenLists(1);
	m_creasedWallList = glGenLists(1);
	m_flatList = glGenLists(1);
	
	// Plane Walls
	glNewList( m_planeWallList, GL_COMPILE );
	glBegin(GL_TRIANGLES);
	vector<int>::iterator iterPlaneWall = m_planeWalls.begin();
	while (iterPlaneWall != m_planeWalls.end())
	{
		int squareIndx = *iterPlaneWall;
		Square& square = m_squares[squareIndx];
		
		drawTriangle( square.m_t1,  square.m_t1.m_n, m_mesh );
		drawTriangle( square.m_t2,  square.m_t2.m_n, m_mesh );
		
		++iterPlaneWall;		
	}
	glEnd();
	glEndList();
	
	// Creased Walls
	glNewList( m_creasedWallList, GL_COMPILE );
	glBegin(GL_TRIANGLES);
	vector<int>::iterator iterCreasedWall = m_creasedWalls.begin();
	while (iterCreasedWall != m_creasedWalls.end())
	{
		int squareIndx = *iterCreasedWall;
		Square& square = m_squares[squareIndx];
		
		drawTriangle( square.m_t1,  square.m_t1.m_n, m_mesh );
		drawTriangle( square.m_t2,  square.m_t2.m_n, m_mesh );
		
		++iterCreasedWall;		
	}
	glEnd();
	glEndList();
	
	
	// Flat squares
	glNewList( m_flatList, GL_COMPILE );
	glBegin(GL_TRIANGLES);
	vector<int>::iterator iterFlat = m_flat.begin();
	while (iterFlat != m_flat.end())
	{
		int squareIndx = *iterFlat;
		Square& square = m_squares[squareIndx];
		
		drawTriangle( square.m_t1,  square.m_t1.m_n, m_mesh );
		drawTriangle( square.m_t2,  square.m_t2.m_n, m_mesh );
		
		++iterFlat;		
	}
	glEnd();
	glEndList();
}


/****************** DEBUG  ******************/

void Terrain::highlightSquare( int indx )
{
	if (indx<0 || indx>(int)m_squares.size()-1) return;
	Square& square = m_squares[indx];
	
	drawTriangle_highlight( square.m_t1, m_mesh );
	drawTriangle_highlight( square.m_t2, m_mesh );
}

void Terrain::drawPlinthTop_DEBUG()
{
	glColor3f(1,1,1);
	Vec4& a0 = m_plinthTopTriangles[0]->m_v[0];
	Vec4& a1 = m_plinthTopTriangles[0]->m_v[2];
	Vec4& a2 = m_plinthTopTriangles[0]->m_v[1];
	
	Vec4& b0 = m_plinthTopTriangles[1]->m_v[0];
	Vec4& b1 = m_plinthTopTriangles[1]->m_v[2];
	Vec4& b2 = m_plinthTopTriangles[1]->m_v[1];
	
	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	glVertex3f( a0[0], a0[1], a0[2] );
	glVertex3f( a1[0], a1[1], a1[2] );
	glVertex3f( a2[0], a2[1], a2[2] );
	
	glVertex3f( b0[0], b0[1], b0[2] );
	glVertex3f( b1[0], b1[1], b1[2] );
	glVertex3f( b2[0], b2[1], b2[2] );
	glEnd();
	glEnable(GL_LIGHTING);
}

void Terrain::drawBoulderAABBs_DEBUG()
{
	vector<int>::iterator iter = m_squaresHit[0].begin();
	while (iter != m_squaresHit[0].end())
	{
		int squareIndx = *iter;
		Square& square = m_squares[squareIndx];
		
		if (square.m_occupants.size() == 0)
		{
			iter++;
			continue;
		}
		
		vector<GameObject*>::iterator iter_occupants = square.m_occupants.begin();
		while (iter_occupants != square.m_occupants.end())
		{
			GameObject* object = *iter_occupants;
			
			if ( object->getType() == OBJECT_BOULDER )
			{
				Boulder* boulder = static_cast<Boulder*> ( object );
				
				const AABB& aabb = boulder->getWorldAABB();
				
				aabb.draw();
			}
			
			iter_occupants++;
		}
		
		++iter;
	}
}

void Terrain::drawBresenhamBand_DEBUG()
{
	vector<int>::iterator iter1 = m_squaresHit[0].begin();
	
	glBegin(GL_TRIANGLES);
	glColor3f(1,0,1);
	while (iter1 != m_squaresHit[0].end())
	{
		int squareIndx = *iter1;
		highlightSquare(squareIndx);
		++iter1;		
	}
	glEnd();
}

void Terrain::drawOccupiedSquares_DEBUG()
{
	glColor3f(1,0,0);
	glBegin(GL_TRIANGLES);
	set<int>::iterator iter = m_occupiedSquares.begin();
	while (iter != m_occupiedSquares.end())
	{
		int indx = *iter;
		highlightSquare(indx);
		iter++;
	}
	glEnd();
}

void Terrain::drawNormals_DEBUG()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG );
	glColor3d(1.0,0.0,0.0);
	glLineWidth(2.0f);
	
	vector<Square>::iterator iter = m_squares.begin();
	
	glBegin(GL_LINES);
	
	while (iter != m_squares.end())
	{
		Square& square = (*iter);
		
		drawTriangleNorm( square.m_t1, square.m_t1.m_n, m_mesh );
		drawTriangleNorm( square.m_t2, square.m_t2.m_n, m_mesh );
		
		++iter;		
	}
	
	glEnd();
	glPopAttrib();	
}



