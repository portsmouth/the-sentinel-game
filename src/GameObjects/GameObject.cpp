
#include "GameObject.h"
#include "Sentinel.h"
#include "Sentry.h"
#include "Meanie.h"

#include "Game.h"
#include "Player.h"
#include "World.h"
#include "terrain.h"
#include "geometry.h"
#include "Maths.h"

#include "DisplayHandler.h"
#include "DisplayObject.h"
#include "ModelLibrary.h"
#include "Model.h"
#include "TextureLibrary.h"
#include "FileUtils.h"

GameObject::GameObject( Object_Types type, int squareIndx ) :

m_type(type), 
m_squareIndx(squareIndx), 
m_state(OBJECT_NORMAL)

{
	Game::pWorld->pTerrain->registerSquareAsOccupied(m_squareIndx);
	m_energy = 0.f;
}

GameObject::~GameObject()
{
	// remove display object
	delete m_displayObject;
}


void GameObject::setSentinelRelativeDirection()
{
	Sentinel* S = Game::pWorld->pSentinel;
	if (S)
	{
		const Vec4& Ts = S->getTranslation();
		const Vec4& T  = getTranslation();
		
		m_sentinelRelativeDirection = T - Ts;
		m_sentinelRelativeDirection[2] = 0.f;
		m_sentinelRelativeDirection.normalizeIfNotZero();
	}	
}


// MUST be run before any object is instantiated
void GameObject::loadAssets() //static
{
	Boulder::loadAssets();
	Tree::loadAssets();
	Synthoid::loadAssets();
	Sentinel::loadAssets();
	Plinth::loadAssets();
	Sentry::loadAssets();
	Meanie::loadAssets();
}


float GameObject::getInitialEnergy()
{
	switch(m_type)
	{
		case OBJECT_TREE:           return Tree     ::getInitialEnergy();
		case OBJECT_BOULDER:        return Boulder  ::getInitialEnergy();
		case OBJECT_SYNTHOID_SHELL: return Synthoid ::getInitialEnergy();
		case OBJECT_ENEMY:         
		{
			Enemy *E = static_cast<Enemy*> ( this );
			switch ( E->getEnemyType() )
			{
				case ENEMY_SENTINEL: return Sentinel ::getInitialEnergy(); 
				case ENEMY_SENTRY:   return Sentry   ::getInitialEnergy(); 
				case ENEMY_MEANIE:   return Meanie   ::getInitialEnergy(); 
			}
		}
			
		default: return 0.f;
	}
}


// keep transparency in sync with the energy content
void GameObject::updateTransparency()
{
	float transparency = 1.f - m_energy / m_initialEnergy;
	m_displayObject->setTransparency( transparency );
}


// attempt to extract a specified amount of energy (and return the amount actually extracted)
void GameObject::removeEnergy( float& dE )
{
	if (m_state == OBJECT_ABSORBED || m_state == OBJECT_DEAD || m_state == OBJECT_OFFLIMITS) 
	{
		dE = 0.0f;
		return;
	}
		
	float e0 = m_energy;
	m_energy -= dE;
	if (m_energy <= 0.f) 
	{
		m_energy = 0.f;
		m_state = OBJECT_ABSORBED;
	}
	
	dE = e0 - m_energy;
	updateTransparency();
}

// absorb object with one call (automatic absorption, for the player), at the set rate
void GameObject::absorb()
{
	// sentinel and its plinth cannot be absorbed in the ordinary way
	if (m_type == OBJECT_SENTINEL_PLINTH) return;
	
	if (m_state == OBJECT_NORMAL)
	{
		m_state = OBJECT_ABSORBING;
		m_absorbTimer.start();
	}
}

void GameObject::stopAbsorption()
{
	if (m_state != OBJECT_ABSORBING) return;
	
	m_state = OBJECT_NORMAL;
}


// player absorption
const float PLAYER_GAMEOBJECT_ENERGY_ABSORPTION_RATE = 2.0f; //energy units per sec

void GameObject::updateAbsorptionByPlayer()
{
	if (m_state != OBJECT_ABSORBING) return;
	
	float dE = PLAYER_GAMEOBJECT_ENERGY_ABSORPTION_RATE * m_absorbTimer.time();
	
	//transfer energy to player
	removeEnergy(dE);
	Player::Instance()->giveEnergy(dE);
	if (m_state == OBJECT_ABSORBED)
	{
		markForDeletion();
	}
	
	m_absorbTimer.start();
	return;
}

void GameObject::markForDeletion()
{
	m_state = OBJECT_DEAD;
}

void GameObject::deactivateDisplay()
{
	m_displayObject->deactivate();
}

void GameObject::activateDisplay()
{
	m_displayObject->activate();
}

const AABB& GameObject::getWorldAABB()
{
	return m_displayObject->getWorldAABB();
}


const float GAMEOBJECT_MATERIALIZATION_RATE = 0.2f; // alpha per second

void GameObject::markForMaterialization()
{
	m_state = OBJECT_MATERIALIZING;
	m_energy = 0.0f;
	m_materializationTimer.start();
}

void GameObject::updateMaterialization()
{
	float alpha = m_materializationTimer.time() / GAMEOBJECT_MATERIALIZATION_RATE;
	if (alpha>1.0f) 
	{
		m_energy = m_initialEnergy;
		m_state = OBJECT_NORMAL;
		updateTransparency();
		return;
	}
	
	m_energy = alpha * m_initialEnergy;
	updateTransparency();
}

void GameObject::markAsOffLimits(bool toggle)
{
	if ( toggle && m_state == OBJECT_NORMAL )
	{
		m_state = OBJECT_OFFLIMITS;
	}
	
	if ( !toggle && m_state == OBJECT_OFFLIMITS )
	{
		m_state = OBJECT_NORMAL;
	}
}

/*************************************************************/
/*                        Synthoid                           */
/*************************************************************/

Synthoid::Synthoid( int squareIndx, Vec4& translation ) :

GameObject(OBJECT_SYNTHOID_SHELL, squareIndx)

{
	m_displayObject = DisplayHandler::Instance()->createDisplayObject( m_synthoidModel, m_synthoidTexture, translation );
	
	m_initialEnergy = Synthoid::getInitialEnergy();
	m_energy = m_initialEnergy;
	
	setSentinelRelativeDirection();
	
	m_rotationOffset = 0.0f;
}

Model*   Synthoid::m_synthoidModel   = NULL;
Texture* Synthoid::m_synthoidTexture = NULL;
float    Synthoid::m_height = 0.f;


// ASSETS
const std::string kModelSynthoid("/Models/synthoid.obj");

void Synthoid::loadAssets() //static
{
	string synthoid_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelSynthoid;
	m_synthoidModel = ModelLibrary::Instance()->GetModel(synthoid_modelpath, FLAT_SHADED);
	m_synthoidModel->genTriangleUVs();
	
	// set synthoid height from computed AABB
	m_height = m_synthoidModel->m_AABB.m_max[2] - m_synthoidModel->m_AABB.m_min[2];
	
	string synthoid_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_basic;
	m_synthoidTexture = TextureLibrary::Instance()->GetTexture(synthoid_texturepath, BUILD_MIPMAPS);
}

float Synthoid::getHeight() //static
{
	return m_height;
}

float Synthoid::getInitialEnergy() //static
{
	return 8.0f;
}


void Synthoid::facePlayer()
{
	const Vec4& T = getTranslation();
	const Vec4& P = Player::Instance()->GetViewPoint();
	
	Vec4 D = P - T;
	D[2] = 0.0f;
	D.normalizeIfNotZero();
	
	m_rotation = - radToDeg * atan2f( D[0], D[1] );
	m_displayObject->setRotation(m_rotation + m_rotationOffset);
}

void Synthoid::faceDirection(Vec4& dir)
{
	Vec4 D = dir;
	D[2] = 0.0f;
	
	m_rotation = - radToDeg * atan2f( D[0], D[1] );
	m_displayObject->setRotation(m_rotation + m_rotationOffset);
}

void Synthoid::setTransparency(float transparency)
{
	m_displayObject->setTransparency(transparency);
}


/*************************************************************/
/*                        Boulder                            */
/*************************************************************/

Boulder::Boulder( int squareIndx, Vec4& translation ) :

GameObject(OBJECT_BOULDER, squareIndx)

{
	m_displayObject = DisplayHandler::Instance()->createDisplayObject( m_boulderModel, m_boulderTexture, translation );
	
	m_initialEnergy = Boulder::getInitialEnergy();
	m_energy = m_initialEnergy;
	
	setSentinelRelativeDirection();
	
	AABB& aabb = m_displayObject->getWorldAABB();
	aabb.expand(0.85f);
}


Model*   Boulder::m_boulderModel   = NULL;
Texture* Boulder::m_boulderTexture = NULL;
float    Boulder::m_height = 0.f;


// ASSETS
const std::string kModelBoulder_lowres("/Models/boulder1_ultralowres.obj");
const std::string kModelBoulder_medres("/Models/boulder1_lowres.obj");
const std::string kModelBoulder_highres("/Models/boulder1.obj");
const std::string kModelBoulder_triangulated("/Models/boulder_triangulated.obj");
const std::string kTextureBoulder("/Textures/GRYDIRT2.JPG");

void Boulder::loadAssets() //static
{
	string boulder_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelBoulder_triangulated;
	string boulder_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_basic;
	
	//string boulder_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelBoulder_medres;
	//string boulder_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTextureBoulder;

	m_boulderModel = ModelLibrary::Instance()->GetModel(boulder_modelpath, SMOOTH_SHADED);
	m_boulderModel->genTriangleUVs(0.005f);
	
	// set boulder height from computed AABB
	m_height = m_boulderModel->m_AABB.m_max[2] - m_boulderModel->m_AABB.m_min[2];
		
	m_boulderTexture = TextureLibrary::Instance()->GetTexture(boulder_texturepath, BUILD_MIPMAPS);
}

float Boulder::getHeight() //static
{
	return m_height;
}

float Boulder::getInitialEnergy() //static
{
	return 2.0f;
}


/*************************************************************/
/*                        Tree                               */
/*************************************************************/

Tree::Tree( int squareIndx, Vec4& translation ) :

GameObject(OBJECT_TREE, squareIndx)

{
	m_displayObject = DisplayHandler::Instance()->createDisplayObject( m_treeModel, m_treeTexture, translation );
	
	m_initialEnergy = Tree::getInitialEnergy();
	m_energy = m_initialEnergy;
	
	setSentinelRelativeDirection();
}

Model*   Tree::m_treeModel   = NULL;
Texture* Tree::m_treeTexture = NULL;

// ASSETS
const std::string kModelTree_lowres("/Models/tree1_lowres.obj");
const std::string kModelTree_medres("/Models/tree1.obj");
const std::string kTextureTree("/Textures/mold.jpg");
const std::string kModelTree_blender("/Models/blenderTest.obj");

void Tree::loadAssets() //static
{
	string tree_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelTree_medres;
	m_treeModel = ModelLibrary::Instance()->GetModel(tree_modelpath, FLAT_SHADED);
	m_treeModel->genTriangleUVs();
	
	//string tree_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTextureTree;
	string tree_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_basic;
	m_treeTexture = TextureLibrary::Instance()->GetTexture(tree_texturepath, BUILD_MIPMAPS);
	 
}

float Tree::getInitialEnergy() //static
{
	return 1.0f;
}


/*************************************************************/
/*                   Plinth (for Sentinel)                    */
/*************************************************************/

Plinth::Plinth( int squareIndx, Vec4& translation ) :

GameObject(OBJECT_SENTINEL_PLINTH, squareIndx)

{
	m_displayObject = DisplayHandler::Instance()->createDisplayObject( m_plinthModel, m_plinthTexture, translation );
}

Model*   Plinth::m_plinthModel   = NULL;
Texture* Plinth::m_plinthTexture = NULL;
float    Plinth::m_height = 0.f;

// ASSETS
const std::string kModelSentinelPlinth("/Models/sentinel_plinth.obj");

void Plinth::loadAssets() //static
{
	string plinth_modelpath = FileUtils::GetApplicationResourcesFolderPath() + kModelSentinelPlinth;
	m_plinthModel = ModelLibrary::Instance()->GetModel(plinth_modelpath, FLAT_SHADED);

	//m_plinthModel->genTriangleUVs();
	//string plinth_texturepath = FileUtils::GetApplicationResourcesFolderPath() + kTerrain_basic;
	//m_plinthTexture = TextureLibrary::Instance()->GetTexture(plinth_texturepath, BUILD_MIPMAPS);
	
	// set plinth height from computed AABB
	m_height = m_plinthModel->m_AABB.m_max[2] - m_plinthModel->m_AABB.m_min[2];
}

float Plinth::getHeight() //static
{
	return m_height;
}

// object space AABB of the plinth
AABB& Plinth::getAABB() //static
{
	return m_plinthModel->m_AABB;
}



