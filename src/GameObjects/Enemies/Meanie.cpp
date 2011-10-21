
#include "Meanie.h"

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

const float MEANIE_PLAYER_ENERGY_ABSORPTION_RATE      = 0.5f; //per sec
const float MEANIE_GAME_OBJECT_ENERGY_ABSORPTION_RATE = 1.0f; //per sec
const float MEANIE_AVERAGE_OBJECT_ABSORPTION_INTERVAL = 10.0f; //in secs


Meanie::Meanie( int squareIndx, Vec4& translation, float rotation ) :

Enemy(squareIndx, translation)

{
	m_displayObject = DisplayHandler::Instance()->createDisplayObject( m_meanieModel, m_meanieTexture, translation );
	
	m_initialEnergy = Meanie::getInitialEnergy();
	m_energy = m_initialEnergy;
	m_desiredEnergy = m_initialEnergy;
	
	m_rotationOffset = -90.f;
	m_rotation = rotation;
	
	m_displayObject->setRotation(m_rotation + m_rotationOffset);

	// eye position currently simply the sentinel base translation + the AABB height
	m_eyePosition = translation;
	m_eyePosition[2] += Meanie::m_height;
	
	stalledTimer.start();
	objectAbsorptionDecisionTimer.start();
	
	m_objectAbsorbSeed = -42;
}

Enemy_Types Meanie::getEnemyType()
{
	return ENEMY_MEANIE;
}

Meanie::~Meanie()
{
}


float Meanie::getInitialEnergy() //static
{
	return 3.0f;
}

float Meanie::getHeight() //static
{
	return m_height;
}

Model*   Meanie::m_meanieModel      = NULL;
Texture* Meanie::m_meanieTexture    = NULL;
float    Meanie::m_height = 0.f;

// ASSETS
const std::string kModelSentinel("/Models/sentinel3.obj");

void Meanie::loadAssets() //static
{
	string meanie_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelSentinel;
	m_meanieModel = ModelLibrary::Instance()->GetModel(meanie_modelpath, SMOOTH_SHADED);
	
	//string meanie_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_planeWalls;
	//m_meanieTexture = TextureLibrary::Instance()->GetTexture(meanie_texturepath, BUILD_MIPMAPS);

	// set sentinel height from computed AABB
	m_height = m_meanieModel->m_AABB.m_max[2] - m_meanieModel->m_AABB.m_min[2];	
}

void Meanie::think()
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
			
			if ( wishToAbsorbAnObject() && objectsVisible() )
			{
				m_behaviourState = ENEMY_ABSORBING_OBJECT;
				chooseObjectToAbsorb();
				turnTimer.start();
				objectAbsorbTimer.start();
				break;
			}
			
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
			
		case ENEMY_BEING_ABSORBED:
			
			break;
			
	}
	
}


bool Meanie::objectsVisible()
{
	// construct projected gaze direction
	Vec4 gazeDir( cos(m_rotation * degToRad), sin(m_rotation * degToRad), 0.f );
	
	m_seenObjects.clear();
	
	const set<int>& occupiedSquares = Game::pWorld->pTerrain->getOccupiedSquares();
	set<int>::const_iterator iter = occupiedSquares.begin();
	while (iter != occupiedSquares.end())
	{
		Square& square = Game::pWorld->pTerrain->getSquare( *iter );
		
		if ( square.m_occupants.size() > 0 
			 
			 // sentinel should not try to absorb itself or its plinth ..
			 && square.m_indx != m_squareIndx ) 
		{
			GameObject* object = square.getHighestOccupant();
			
			const Vec4& n = object->getSentinelRelativeDirection();
			
			float angleCosine = gazeDir * n;
			if ( angleCosine > m_viewRangeCosine )
			{
				const Vec4& T = object->getTranslation();
				HitData data;
				data.m_boulder = NULL;
				bool hit = Game::pWorld->pTerrain->isTargetVisibleFromOrigin( m_eyePosition, T, data );
				if ( hit || data.m_boulder == (Boulder*)object )
				{
					m_seenObjects.push_back(object);
				}
			}
		}
		
		iter++;
	}
	
	return m_seenObjects.size() > 0;
}



bool Meanie::wishToAbsorbAnObject()
{
	bool absorbNow = PoissonDecision( objectAbsorptionDecisionTimer.time(), 
									  MEANIE_AVERAGE_OBJECT_ABSORPTION_INTERVAL, 
									  &m_objectAbsorbSeed );
	
	objectAbsorptionDecisionTimer.start();
	return absorbNow; 
}

void Meanie::chooseObjectToAbsorb()
{
	// sentinel will absorb an object - choose one randomly
	int nObject = getRand( m_seenObjects.size()-1, &m_objectAbsorbSeed);
				
	m_objectBeingAbsorbed = m_seenObjects[nObject];	
}


// TO DO: render sentinel absorption/creation beam

void Meanie::absorbObject()
{
	Object_States objectState = m_objectBeingAbsorbed->getState();
	
	// if object being absorbed by player, sentinel can't have it 
	// (because object might be deleted by player's absorption)
	if (objectState == OBJECT_ABSORBING) 
	{
		m_behaviourState = ENEMY_SEARCHING;
		return;
	}
	
	// if object has been fully absorbed, mark it for deletion, and start searching again
	if (objectState == OBJECT_ABSORBED) 
	{
		m_objectBeingAbsorbed->markForDeletion();
		m_behaviourState = ENEMY_SEARCHING;
		return;
	}
	
	const Vec4& T = m_objectBeingAbsorbed->getTranslation();
	float dE = objectAbsorbTimer.time() * MEANIE_GAME_OBJECT_ENERGY_ABSORPTION_RATE;
	m_objectBeingAbsorbed->removeEnergy( dE );
	m_energy += dE;
	m_initialEnergy = m_energy;
	objectAbsorbTimer.start();
	
	// turn to face object 
	turnToFacePoint(T);
}





