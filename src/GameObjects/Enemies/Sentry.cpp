

#include "Sentry.h"

#include "Game.h"
#include "World.h"
#include "EventManager.h"
#include "terrain.h"
#include "geometry.h"
#include "Player.h"

#include "DisplayHandler.h"
#include "DisplayObject.h"
#include "ModelLibrary.h"
#include "Model.h"
#include "FileUtils.h"
#include "TextureLibrary.h"

#include "Maths.h"


const float SENTRY_VIEWRANGE_COSINE = 0.894427191f;    //cos(a), half-sector angle cosine
const float SENTRY_VIEWRANGE        = 53.13010f;       //2a degrees, view sector
const float SENTRY_VIEWRANGE_HALF   = 26.56505f;       //a = acos(2/sqrt(5)) degrees, view half-sector

const float SENTRY_ROTATION_RATE        = 10.f;   //degrees/sec
const float SENTRY_ROTATION_ACTIVE_TIME = 5.f; //secs
const float SENTRY_ROTATION_STALL_TIME  = 2.f;  //secs

const float SENTRY_PLAYER_ENERGY_ABSORPTION_RATE      = 0.333f; //per sec
const float SENTRY_GAME_OBJECT_ENERGY_ABSORPTION_RATE = 1.0f; //per sec
const float SENTRY_AVERAGE_OBJECT_ABSORPTION_INTERVAL = 10.0f; //in secs

Sentry::Sentry( int squareIndx, Vec4& translation, float rotation ) :

Enemy(squareIndx, translation)

{
	m_displayObject = DisplayHandler::Instance()->createDisplayObject( m_sentryModel, m_sentryTexture, translation );
	
	m_initialEnergy = Sentry::getInitialEnergy();
	m_energy = m_initialEnergy;
	m_desiredEnergy = m_initialEnergy;
	
	m_rotationOffset = -90.f;
	m_rotation = rotation;
	
	m_rotationRate = SENTRY_ROTATION_RATE;
	m_rotationActiveTime = SENTRY_ROTATION_ACTIVE_TIME;
	m_rotationStallTime = SENTRY_ROTATION_STALL_TIME;
	m_playerEnergyAbsorptionRate = SENTRY_PLAYER_ENERGY_ABSORPTION_RATE;
	
	m_viewRangeCosine = SENTRY_VIEWRANGE_COSINE;
	m_viewRange = SENTRY_VIEWRANGE;
	m_viewRangeHalf = SENTRY_VIEWRANGE_HALF;
	
	m_displayObject->setRotation(m_rotation + m_rotationOffset);
	
	// eye position currently simply the sentinel base translation + the AABB height
	m_eyePosition = translation;
	m_eyePosition[2] += Sentry::m_height;
	
	stalledTimer.start();
	objectAbsorptionDecisionTimer.start();
	
	m_objectAbsorbSeed = -42;
}

Enemy_Types Sentry::getEnemyType()
{
	return ENEMY_SENTRY;
}

Sentry::~Sentry()
{
}

float Sentry::getInitialEnergy() //static
{
	return 10.0f;
}

float Sentry::getHeight() //static
{
	return m_height;
}

Model*   Sentry::m_sentryModel      = NULL;
Texture* Sentry::m_sentryTexture    = NULL;
float    Sentry::m_height = 0.f;

// ASSETS
const std::string kModelSentinel("/Models/sentry.obj");

void Sentry::loadAssets() //static
{
	string sentry_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelSentinel;
	m_sentryModel = ModelLibrary::Instance()->GetModel(sentry_modelpath, FLAT_SHADED);
	m_sentryModel->genTriangleUVs();
	
	string sentry_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_basic;
	m_sentryTexture = TextureLibrary::Instance()->GetTexture(sentry_texturepath, BUILD_MIPMAPS);

	// set sentinel height from computed AABB
	m_height = m_sentryModel->m_AABB.m_max[2] - m_sentryModel->m_AABB.m_min[2];
}

/*
 
  Sentry bug, if sentry is allowed to absorb objects
 
 see landscape 51, code 6950 3296
 
 probaby sentry/sentinel competing for same object, pointer becomes invalid
 
need reference counting for game objects to handle that
 
 */


void Sentry::think()
{	
	switch( m_behaviourState )
	{
		case ENEMY_SEARCHING:
			
			if ( playerInVisibleSector() )
			{
				if ( playersHeadIsVisible() )
				{
					m_behaviourState = ENEMY_ABSORBING_PLAYER;
					turnTimer.start();
					objectAbsorbTimer.start();
					break;
				}
				else if ( playersSquareIsVisible() )
				{
					// TO DO: create a meanie if I can, near the player
					
					
				}
			}
			
			if ( m_energy > m_desiredEnergy + 1 )
			{
				// TO DO: create a tree to re-distribute what I absorbed
				
			}
			
			/*
			if ( wishToAbsorbAnObject() && objectsVisible() )
			{
				m_behaviourState = ENEMY_ABSORBING_OBJECT;
				chooseObjectToAbsorb();
				turnTimer.start();
				objectAbsorbTimer.start();
				break;
			}
			 */
			
			rotate();
			break;
			
			//TO DO: sentinel tree creation
			
		case ENEMY_ABSORBING_PLAYER:
			
			if ( !playerInVisibleSector() || !playersHeadIsVisible() )
			{
				m_behaviourState = ENEMY_SEARCHING;
			}
			else
			{
				absorbPlayer();
			}
			
			break;
			
		/*	
		case ENEMY_ABSORBING_OBJECT:
			
			if ( playerInVisibleSector() && playersHeadIsVisible() )
			{
				m_behaviourState = ENEMY_ABSORBING_PLAYER;
				turnTimer.start();
				objectAbsorbTimer.start();
			}
			else
			{
				absorbObject();
			}
			break;
		*/
			
		case ENEMY_BEING_ABSORBED:
			
			break;
			
	}
	
}





