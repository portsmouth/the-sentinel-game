
#include "Sentinel.h"

#include "Game.h"
#include "World.h"
#include "EventManager.h"
#include "terrain.h"
#include "geometry.h"
#include "Player.h"#
#include "GameObject.h"

#include "DisplayHandler.h"
#include "DisplayObject.h"
#include "ModelLibrary.h"
#include "Model.h"
#include "FileUtils.h"
#include "TextureLibrary.h"

#include "Maths.h"

/********** tweakable Sentinel parameters **************/

// Time delay between the Sentinel seeing the player and beginning absorption
const float SENTINEL_DELAY_BEFORE_ABSORBING  = 5.f; 

/************************************/

const float SENTINEL_VIEWRANGE_COSINE = 0.894427191f;    //cos(a), half-sector angle cosine
const float SENTINEL_VIEWRANGE        = 53.13010f;       //2a degrees, view sector
const float SENTINEL_VIEWRANGE_HALF   = 26.56505f;       //a = acos(2/sqrt(5)) degrees, view half-sector

const float SENTINEL_ROTATION_RATE        = 4.f;   //degrees/sec
const float SENTINEL_ROTATION_ACTIVE_TIME = 5.f; //secs
const float SENTINEL_ROTATION_STALL_TIME  = 5.f;  //secs

const float SENTINEL_PLAYER_ENERGY_ABSORPTION_RATE      = 0.5f; //per sec
const float SENTINEL_GAME_OBJECT_ENERGY_ABSORPTION_RATE = 1.0f; //per sec
const float SENTINEL_AVERAGE_OBJECT_ABSORPTION_INTERVAL = 10.0f; //in secs

const float SENTINEL_MIN_TIME_BETWEEN_TREE_GENERATION_EVENTS = 15.0f;

Sentinel::Sentinel( int squareIndx, Vec4& translation ) :

Enemy(squareIndx, translation)

{
	m_displayObject = DisplayHandler::Instance()->createDisplayObject( m_sentinelModel, m_sentinelTexture, translation );
	
	m_initialEnergy = Sentinel::getInitialEnergy();
	m_energy = m_initialEnergy;
	m_desiredEnergy = m_initialEnergy;
	
	m_rotationOffset = -90.f;
	m_rotation = 0.f;
	
	m_rotationRate = SENTINEL_ROTATION_RATE;
	m_rotationActiveTime = SENTINEL_ROTATION_ACTIVE_TIME;
	m_rotationStallTime = SENTINEL_ROTATION_STALL_TIME;
	m_playerEnergyAbsorptionRate = SENTINEL_PLAYER_ENERGY_ABSORPTION_RATE;
	
	m_viewRangeCosine = SENTINEL_VIEWRANGE_COSINE;
	m_viewRange = SENTINEL_VIEWRANGE;
	m_viewRangeHalf = SENTINEL_VIEWRANGE_HALF;

	m_displayObject->setRotation(m_rotation + m_rotationOffset);
	
	//Square& sentinelSquare = Game::pWorld->pTerrain->getSquare(m_squareIndx);
	
	// eye position currently simply the sentinel base translation + the AABB height
	m_eyePosition = translation;
	m_eyePosition[2] += Sentinel::m_height;
	
	stalledTimer.start();
	objectAbsorptionDecisionTimer.start();
	treeGenerationTimer.start();
	
	m_objectAbsorbSeed = -42;
}

Enemy_Types Sentinel::getEnemyType()
{
	return ENEMY_SENTINEL;
}

Sentinel::~Sentinel()
{
	EventManager::Instance()->heardSentinelDeathCry();
}


float Sentinel::getInitialEnergy() //static
{
	return 15.0f;
}

float Sentinel::getHeight() //static
{
	return m_height;
}

Model*   Sentinel::m_sentinelModel      = NULL;
Texture* Sentinel::m_sentinelTexture    = NULL;
float    Sentinel::m_height = 0.f;

// ASSETS
const std::string kModelSentinel("/Models/sentinel_classic.obj");

void Sentinel::loadAssets() //static
{
	string sentinel_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelSentinel;
	m_sentinelModel = ModelLibrary::Instance()->GetModel(sentinel_modelpath, SMOOTH_SHADED);
	
	// set sentinel height from computed AABB
	m_height = m_sentinelModel->m_AABB.m_max[2] - m_sentinelModel->m_AABB.m_min[2];
	
	//m_sentinelModel->genTriangleUVs(0.0f, 1);
	//string sentinel_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_basicWall;
	//m_sentinelTexture = TextureLibrary::Instance()->GetTexture(sentinel_texturepath, BUILD_MIPMAPS, TEXTURE_MODULATE);
}

void Sentinel::think()
{	
	switch( m_behaviourState )
	{
		case ENEMY_SEARCHING:
			
			if ( playerInVisibleSector() )
			{
				if ( playersHeadIsVisible() )
				{
					m_behaviourState = ENEMY_SEES_PLAYER;
					turnTimer.start();
					objectAbsorbTimer.start();
					break;
				}
				else if ( playersSquareIsVisible() )
				{
					// TO DO: create a meanie if I can, near the player.
					// once that is done, no more meanies can be created until the
					// player moves square.
					
					
				}
			}
			
			if ( m_energy > m_initialEnergy + Tree::getInitialEnergy() && \
				 treeGenerationTimer.time() > SENTINEL_MIN_TIME_BETWEEN_TREE_GENERATION_EVENTS )
			{
				// create a tree to re-distribute what I absorbed
				treeGenerationTimer.start();
				generateTree();
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
			
		case ENEMY_SEES_PLAYER:
			
			if ( !playerInVisibleSector() || !playersHeadIsVisible() )
			{
				m_behaviourState = ENEMY_SEARCHING;
			}
			else
			{
				if ( objectAbsorbTimer.time() > SENTINEL_DELAY_BEFORE_ABSORBING )
				{
					m_behaviourState = ENEMY_ABSORBING_PLAYER;
					objectAbsorbTimer.start();
				}
			}
			break;
			
		case ENEMY_ABSORBING_PLAYER:
			
			if ( !playerInVisibleSector() || !playersHeadIsVisible() )
			{
				m_behaviourState = ENEMY_SEARCHING;
			}
			else
			{
				absorbPlayer();
				objectAbsorbTimer.start();
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


void Sentinel::generateTree()
{
	// Choose a random unoccupied flat square
	const vector<int>& flatSquares = Game::pWorld->pTerrain->getFlatSquares();
	vector<Square*>::size_type N = flatSquares.size();
	
	int sIndx;
	bool occupied;
	
	do
	{
		int fIndx = getRand( N-1, &Game::pWorld->pTerrain->getSeed() );
		sIndx = flatSquares[fIndx];
		occupied = ( Game::pWorld->pTerrain->getSquare(sIndx).m_occupants.size() > 0 );
	}
	while (occupied);
	
	// Put a tree there
	Square& square = Game::pWorld->pTerrain->getSquare(sIndx);
	Vec4 translation = square.m_center;
	
	Tree* tree = new Tree( square.m_indx, translation );
	square.m_occupants.push_back( (GameObject*)tree );
	
	// Make the tree materialize (fade in) over time
	tree->markForMaterialization();
}


bool Sentinel::objectsVisible()
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
			 
			 // sentinel should not try to absorb itself, or its plinth, ...
			 && square.m_indx != m_squareIndx ) 
		{
			GameObject* object = square.getHighestOccupant();
			
			if (object->getType() != OBJECT_ENEMY && object->getState() == OBJECT_NORMAL)
			{
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
		}
		
		iter++;
	}
	
	return m_seenObjects.size() > 0;
}



bool Sentinel::wishToAbsorbAnObject()
{
	bool absorbNow = PoissonDecision( objectAbsorptionDecisionTimer.time(), 
									  SENTINEL_AVERAGE_OBJECT_ABSORPTION_INTERVAL, 
									  &m_objectAbsorbSeed );
	
	objectAbsorptionDecisionTimer.start();
	return absorbNow; 
}

void Sentinel::chooseObjectToAbsorb()
{
	// sentinel will absorb an object - choose one randomly
	int nObject = getRand( m_seenObjects.size()-1, &m_objectAbsorbSeed);
				
	m_objectBeingAbsorbed = m_seenObjects[nObject];	
}


// TO DO: render sentinel absorption/creation beam

void Sentinel::absorbObject()
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
	float dE = objectAbsorbTimer.time() * SENTINEL_GAME_OBJECT_ENERGY_ABSORPTION_RATE;
	m_objectBeingAbsorbed->removeEnergy( dE );
	m_energy += dE;
	m_initialEnergy = m_energy;
	objectAbsorbTimer.start();
	
	// turn to face object 
	turnToFacePoint(T);
}


