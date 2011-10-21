
#include "sky.h"

#include "Game.h"
#include "Player.h"
#include "World.h"
#include "terrain.h"
#include "Sentinel.h"
#include "Maths.h"

#include "FileUtils.h"
#include <string>
#include <iostream>
using std::string;
using std::cout;
using std::endl;

float ran2(long *idum);

Sky::Sky(SkyType type) :

m_type(type)

{
	loadAssets();
}


Sky::~Sky() //destructor
{
	//remember to free up textures
	TextureLibrary *texturelibrary = TextureLibrary::Instance();
	string resourcepath = FileUtils::GetApplicationResourcesFolderPath();

	string skyboxpath = resourcepath + kSkybox;
	texturelibrary->FreeTexture(skyboxpath + kUP);
	texturelibrary->FreeTexture(skyboxpath + kDOWN);
	texturelibrary->FreeTexture(skyboxpath + kLEFT);
	texturelibrary->FreeTexture(skyboxpath + kRIGHT);
	texturelibrary->FreeTexture(skyboxpath + kBACK);
	texturelibrary->FreeTexture(skyboxpath + kFRONT);
	
	texturelibrary->FreeTexture(resourcepath + kTerrain_planeWalls);
}	

void Sky::Init()
{
	World *pWorld = Game::pWorld;
	float tes = 0.5f * pWorld->pTerrain->getTerrainEdgeSize();
	
	//4 corner vertices of each face
	int faces[6][4] = 
			{{1,0,4,5},  //FRONT				
			 {2,3,7,6},  //BACK		
			 {0,2,6,4},  //LEFT
			 {3,1,5,7},  //RIGHT
			 {5,4,6,7},  //UP
			 {0,1,3,2}}; //DOWN
	
	//tex coords of each face vertex
	double faces_tx[4][2] =
	{ {0.0,1.0}, {1.0,1.0}, {1.0,0.0}, {0.0,0.0} };
	
	
	// OUTDOORS
	for (int corn = 0; corn < 8; ++corn)
		for (int i = 0; i < 3; ++i)		
			m_outdoorsData.sbox_corners[corn][i] = tes * ( -0.5 + ((corn >> i) & 1) );
	
	for (int i = 0; i < 6; ++i)
		for (int j = 0; j < 4; ++j)
			m_outdoorsData.sbox_faces[i][j] = faces[i][j];
	
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 2; ++j)
			m_outdoorsData.sbox_faces_tx[i][j] = faces_tx[i][j];
	
	
	// ROOM
	float ses = pWorld->pTerrain->getSquareEdgeSize();
	const Vec4& T = pWorld->pSentinel->getTranslation();
	float Th = T[2] + Sentinel::getHeight();  
	float nh = (int) (Th / ses) + 50;
	Th = ses * nh;
	
	float nSquaresPerPanel = 5.0f;
	float npanelsX = pWorld->pTerrain->getNumSquaresPerEdge() / nSquaresPerPanel;
	float npanelsH = nh/nSquaresPerPanel;
	
	float corners[8][3] =
	{{ -tes, -tes, 0.0f },
	{  tes, -tes, 0.0f },
	{ -tes,  tes, 0.0f },
	{  tes,  tes, 0.0f },
	{ -tes, -tes,  Th },
	{  tes, -tes,  Th },
	{ -tes,  tes,  Th },
	{  tes,  tes,  Th }};
	
	for (int corn = 0; corn < 8; ++corn)
		for (int i = 0; i < 3; ++i)		
			m_roomData.sbox_corners[corn][i] = corners[corn][i];
	
	for (int i = 0; i < 6; ++i)
		for (int j = 0; j < 4; ++j)
		m_roomData.sbox_faces[i][j] = faces[i][j];

	float side_faces_tx[4][2] =
	{ {0.0,npanelsH}, {npanelsX,npanelsH}, {npanelsX,0.0}, {0.0,0.0} };
	
	for (int i = 0; i < 4; ++i)
	for (int j = 0; j < 2; ++j)
		m_roomData.sbox_side_faces_tx[i][j] = side_faces_tx[i][j];
	
	float up_faces_tx[4][2] =
	{ {0.0,npanelsX}, {npanelsX,npanelsX}, {npanelsX,0.0}, {0.0,0.0} };
	
	for (int i = 0; i < 4; ++i)
	for (int j = 0; j < 2; ++j)
		m_roomData.sbox_up_faces_tx[i][j] = up_faces_tx[i][j];
	
}


void Sky::loadAssets()
{
	TextureLibrary *texturelibrary = TextureLibrary::Instance();
	string resourcepath = FileUtils::GetApplicationResourcesFolderPath();
	string skyboxpath = resourcepath + kSkybox;
	
	mTexture_sky[SKY_UP]    = texturelibrary->GetTexture(skyboxpath + kUP, BUILD_MIPMAPS);
	mTexture_sky[SKY_DOWN]  = texturelibrary->GetTexture(skyboxpath + kDOWN, BUILD_MIPMAPS);
	mTexture_sky[SKY_LEFT]  = texturelibrary->GetTexture(skyboxpath + kLEFT, BUILD_MIPMAPS);
	mTexture_sky[SKY_RIGHT] = texturelibrary->GetTexture(skyboxpath + kRIGHT, BUILD_MIPMAPS);
	mTexture_sky[SKY_BACK]  = texturelibrary->GetTexture(skyboxpath + kBACK, BUILD_MIPMAPS);
	mTexture_sky[SKY_FRONT] = texturelibrary->GetTexture(skyboxpath + kFRONT, BUILD_MIPMAPS);
	
	mTexture_wall = texturelibrary->GetTexture(resourcepath + kTerrain_basicWall, BUILD_MIPMAPS);
}

void Sky::draw()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG); 	//turn off fog while drawing sky
	glDisable(GL_DEPTH_TEST);	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0,1.0);
	
	glDisable(GL_DEPTH_TEST);	
	
	switch (m_type)
	{
		case OUTDOORS:
			drawOutdoors();
			break;
			
		case ROOM:
			drawRoom();
			break;
	}
	
	glEnable(GL_DEPTH_TEST);
}

void Sky::drawRoom()
{
	enum sky_directions sd;
	
	mTexture_wall->Apply();
	
	for (sd = SKY_FRONT; sd<SKY_UP; sd=(sky_directions)(sd+1)) 
	{
		mTexture_sky[sd]->Apply();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glBegin(GL_QUADS);
		for (int a=0;a<4;a++) 
		{
			glTexCoord2d(m_roomData.sbox_side_faces_tx[a][0], m_roomData.sbox_side_faces_tx[a][1]);
			glVertex3d( m_roomData.sbox_corners[ m_roomData.sbox_faces[sd][a] ][0] ,
						m_roomData.sbox_corners[ m_roomData.sbox_faces[sd][a] ][1] ,
						m_roomData.sbox_corners[ m_roomData.sbox_faces[sd][a] ][2] );
		}
		glEnd();
	}
	
	
	sd = SKY_UP;
	glBegin(GL_QUADS);
	for (int a=0;a<4;a++) 
	{
		glTexCoord2d(m_roomData.sbox_up_faces_tx[a][0], m_roomData.sbox_up_faces_tx[a][1]);
		glVertex3d( m_roomData.sbox_corners[ m_roomData.sbox_faces[sd][a] ][0] ,
					m_roomData.sbox_corners[ m_roomData.sbox_faces[sd][a] ][1] ,
					m_roomData.sbox_corners[ m_roomData.sbox_faces[sd][a] ][2] );
	}
	glEnd();
	
	glDisable(GL_POLYGON_OFFSET_FILL);
}

void Sky::drawOutdoors()
{
	enum sky_directions sd;
	
	glPushMatrix();
	const Vec4& viewPoint = Player::Instance()->GetViewPoint();
	glTranslatef(viewPoint[0], viewPoint[1], viewPoint[2]);
	
	for (sd = SKY_FRONT; sd<=SKY_DOWN; sd=(sky_directions)(sd+1)) 
	{
		mTexture_sky[sd]->Apply();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glBegin(GL_QUADS);
		for (int a=0;a<4;a++) 
		{
			glTexCoord2d(m_outdoorsData.sbox_faces_tx[a][0], m_outdoorsData.sbox_faces_tx[a][1]);
			glVertex3d( m_outdoorsData.sbox_corners[ m_outdoorsData.sbox_faces[sd][a] ][0] ,
						m_outdoorsData.sbox_corners[ m_outdoorsData.sbox_faces[sd][a] ][1] ,
						m_outdoorsData.sbox_corners[ m_outdoorsData.sbox_faces[sd][a] ][2] );
		}
		glEnd();
		
		//draw edges
		/*
		 glColor3f(0.0,0.0,1.0);
		 glBegin(GL_LINE_LOOP);
		 for (int a=0;a<4;a++) {
			 glVertex3f( m_outdoorsData.sbox_corners[ m_outdoorsData.sbox_faces[sd][a] ][0] ,
						 m_outdoorsData.sbox_corners[ m_outdoorsData.sbox_faces[sd][a] ][1] ,
						 m_outdoorsData.sbox_corners[ m_outdoorsData.sbox_faces[sd][a] ][2] );
		 }
		 glEnd();
		 glColor3f(1.0,1.0,1.0);	
		 */
	}
	
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPopMatrix();
}

	



