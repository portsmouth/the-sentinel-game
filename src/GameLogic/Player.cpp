


#include "Player.h"

#include "Game.h"
#include "EventManager.h"
#include "World.h"
#include "terrain.h"
#include "GameObject.h"
#include "SDLview.h"
#include "geometry.h"
#include "Maths.h"

 
//time for which game over overlay appears
const float GAMEOVER_DELAY = 2.0;

//time for which level won overlay appears
const float LEVELWON_DELAY = 2.0;

//#define PLAYER_INVULNERABILITY


Player* Player::_instance = NULL;

Player* Player::Instance()
{
	if (_instance == NULL)
	{
		_instance = new Player;
	}
	return _instance;
}

Player::Player(void) :

CROSSHAIR_VERBOTEN_X(0.08f)

{	
}

/* player and view characteristics */
void Player::InitObserver()
{
	m_pWorld = Game::pWorld;
	m_pTerrain = m_pWorld->pTerrain;
	
	// create initial synthoid shell and position player 
	m_pTerrain->setInitPlayerSquare();
	Square& square = m_pTerrain->getPlayerSquare();
	bool created = square.createSynthoidShell();
	if (!created)
	{
		printf("Error: could not create initial synthoid");
		exit(EXIT_FAILURE);
	}
	
	GameObject* synthoid = square.getSynthoidShell();
	
	// deactivate display of initially occupied synthoid
	synthoid->deactivateDisplay();
		
	// min view azimuth is such that view cone just encloses occupied square
	float squareEdgeSize = m_pTerrain->getSquareEdgeSize();
	float avoid_angle = 0.01f*degToRad + 0.2f * atan( squareEdgeSize / ( sqrt(2.0) * SYNTHOID_EYE_LEVEL * Synthoid::getHeight() ) );
	avoid_cosine = cos(avoid_angle);
	
	//observer properties
	m_azimuth = 0.0f;
	m_phi = 0.5f*M_PI;
	
	int dims[2];
	SDLview::Instance()->getViewDims(dims);
	aspect_ratio = (float)dims[0] / (float)dims[1];	
	
	m_viewPoint = synthoid->getTranslation();
	m_viewPoint[2] += SYNTHOID_EYE_LEVEL * Synthoid::getHeight();
	
	m_crosshairs.init();
	makeCrosshairRay();
	CROSSHAIR_VERBOTEN_Y = aspect_ratio * CROSSHAIR_VERBOTEN_X;
	dragZoneX = 2.0f * CROSSHAIR_VERBOTEN_X;
	dragZoneY = 2.0f * CROSSHAIR_VERBOTEN_Y;
		
	float terrain_edge_size = m_pTerrain->getTerrainEdgeSize();
	near = 1.0e-3*terrain_edge_size;
	far = 20.0*terrain_edge_size;
	fovy = 60.0;
	frustum_height = 2.0f * near * (float)tan( fovy * M_PI/360.0 );
	frustum_width = frustum_height * aspect_ratio;
	
	//initial state
	m_state = PLAYER_ACTIVE;
	m_energy = 3.0f * Synthoid::getInitialEnergy() + 1.0f * Boulder::getInitialEnergy() + 10000000.f;
	
	// not targeted at a square initially
	m_targeted = -1;
	m_plinthTargeted = false;
}

/******************************************************************************************/
/*                              in-level observer setup                                   */
/******************************************************************************************/

void Player::SetObserverProjection(void)
{			
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, aspect_ratio, near, far);		
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	float terrain_edge_size = m_pTerrain->getTerrainEdgeSize();
	Vec4 ray = m_viewPoint + GetViewDirection() * terrain_edge_size;
	
	Vec4 y = GetLocalY();
	
	gluLookAt( m_viewPoint[0], m_viewPoint[1], m_viewPoint[2],
			   ray[0],         ray[1],         ray[2],
			   y[0],           y[1],           y[2]    );		
}



Vec4 Player::GetViewDirection()
{
	float theta = 0.5f*M_PI - m_azimuth;
	float x = cos(m_phi) * sin(theta);
	float y = sin(m_phi) * sin(theta);
	float z = cos(theta);
	
	Vec4 dir(x, y, z);
	dir.normalizeIfNotZero();
	
	return dir;
}

const Vec4& Player::GetViewPoint()
{
	return m_viewPoint;
}

Vec4 Player::GetLocalY()
{
	// y = - spherical_basis(theta)
	float theta = 0.5f*M_PI - m_azimuth;
	float x = cos(m_phi) * cos(theta);
	float y = sin(m_phi) * cos(theta);
	float z = -sin(theta);
	return Vec4(-x, -y, -z);
}

Vec4 Player::GetLocalX()
{
	// x = - spherical_basis(phi)
	float x = -sin(m_phi);
	float y = cos(m_phi);
	return Vec4(-x, -y, 0.0f);
}


void Player::setView(const Vec4& v)
{
	float x = v[0];
	float y = v[1];
	float z = v[2];
	
	m_azimuth = 0.5f*M_PI - acos(z);
	m_phi = atan2(y, x);
}

void Player::RotateViewAboutVertical(float dAngle)
{
	m_phi -= dAngle;
	m_phi = fmod(m_phi, 2.0f*float(M_PI));
}


void Player::RotateViewAboutHorizontal(float dAzimuth)
{
	float newAzimuth = m_azimuth + dAzimuth;
	float cosTheta = cos(0.5f*M_PI - newAzimuth);
	
	if ( cosTheta>avoid_cosine || cosTheta<-avoid_cosine )
	{
		return;
	}
	
	m_azimuth = newAzimuth; 
}

void Player::goForward()
{
	m_viewPoint += GetViewDirection() * 5.0e-2f;
}

void Player::goBackward()
{
	m_viewPoint -= GetViewDirection() * 5.0e-2f;
}

void Player::goLeft()
{
	m_viewPoint -= GetLocalX() * 3.0e-2f;
}

void Player::goRight()
{
	m_viewPoint += GetLocalX() * 3.0e-2f;
}



/******************************************************************************************/
/*                                crosshair handling                                      */
/******************************************************************************************/

void Player::MoveCrossHairs(float dx, float dy)
{
	const float CROSSHAIR_VERBOTEN_X = 0.0001f;
	float CROSSHAIR_VERBOTEN_Y = aspect_ratio * CROSSHAIR_VERBOTEN_X;

	const float dragZoneX = 2.0f * CROSSHAIR_VERBOTEN_X;
	const float dragZoneY = 2.0f * CROSSHAIR_VERBOTEN_Y;
	
	m_crosshairs.m_x += dx;
	m_crosshairs.m_y += dy;
	
	// Keep crosshairs in bounds
	if (m_crosshairs.m_x < CROSSHAIR_VERBOTEN_X)      m_crosshairs.m_x = CROSSHAIR_VERBOTEN_X;
	if (m_crosshairs.m_y < CROSSHAIR_VERBOTEN_Y)      m_crosshairs.m_y = CROSSHAIR_VERBOTEN_Y;
	if (m_crosshairs.m_x > 1.0f-CROSSHAIR_VERBOTEN_X) m_crosshairs.m_x = 1.0f-CROSSHAIR_VERBOTEN_X;
	if (m_crosshairs.m_y > 1.0f-CROSSHAIR_VERBOTEN_Y) m_crosshairs.m_y = 1.0f-CROSSHAIR_VERBOTEN_Y;
	
	// Rotate viewpoint if crosshairs in drag region
	if (m_crosshairs.m_x < dragZoneX)     
	{	
		RotateViewAboutVertical(-0.05f * M_PI/180.0f);
	}
	else if (m_crosshairs.m_x > 1.0f-dragZoneX)
	{
		RotateViewAboutVertical(0.05f * M_PI/180.0f);
	}
	if (m_crosshairs.m_y < dragZoneY)      
	{
		RotateViewAboutHorizontal(-0.05f * M_PI/180.0f);
	}
	else if (m_crosshairs.m_y > 1.0f-dragZoneY) 
	{
		RotateViewAboutHorizontal(0.05f * M_PI/180.0f);
	}
}


void Player::makeCrosshairRay()
{
	float lW = 0.5f * frustum_width  * ( 2.0f * m_crosshairs.m_x - 1.0f );
	float lH = 0.5f * frustum_height * ( 2.0f * m_crosshairs.m_y - 1.0f );
	
	m_crosshairs.m_dir = near * GetViewDirection() + lW * GetLocalX() + lH * GetLocalY();
	m_crosshairs.m_dir.normalizeIfNotZero();
}


bool Player::crosshairsInDragzone()
{
	bool indragzone = false;
	indragzone |= (m_crosshairs.m_x < dragZoneX);
	indragzone |= (m_crosshairs.m_x > 1.0f - dragZoneX);
	indragzone |= (m_crosshairs.m_y < dragZoneY);
	indragzone |= (m_crosshairs.m_y > 1.0f - dragZoneY);
	return indragzone;
}


float Player::perFrameDragAngle()
{
	return 0.5f * M_PI/180.0f;
	//return 0.1f * M_PI/180.0f;
}


void Player::handleCrosshairs()
{
	// Rotate viewpoint if crosshairs in drag region
	if (m_crosshairs.m_x < dragZoneX)     
	{	
		RotateViewAboutVertical(-perFrameDragAngle());
	}
	else if (m_crosshairs.m_x > 1.0f-dragZoneX)
	{
		RotateViewAboutVertical(perFrameDragAngle());
	}
	if (m_crosshairs.m_y < dragZoneY)      
	{
		RotateViewAboutHorizontal(-perFrameDragAngle());
	}
	else if (m_crosshairs.m_y > 1.0f-dragZoneY) 
	{
		RotateViewAboutHorizontal(perFrameDragAngle());
	}	
	
	makeCrosshairRay();
}


/*************************************************************/
/*                 other accessors                           */
/*************************************************************/

float Player::getEnergy()
{
	return m_energy;
}

void Player::SetState(Player_States newState)
{
	m_state = (Player_States)newState;
}

Player_States Player::GetState()
{
	return (Player_States)m_state;
}




/******************************************************************************************/
/*                              player gameplay logic                                     */
/******************************************************************************************/


void Player::raycastTerrain()
{
	m_targeted = -1;
	m_plinthTargeted = false;
	m_pTerrain->raycast(m_viewPoint, m_crosshairs.m_dir, m_playerRaycastData, TERRAIN_AND_BOULDERS_AND_PLINTH); 
	if (!m_playerRaycastData.m_hit) return;
	
	switch (m_playerRaycastData.m_type)
	{
		case TERRAIN_HIT:
		{
			m_targeted = m_playerRaycastData.m_square;
			break;
		}
			
		case BOULDER_AABB_HIT:
		{
			Boulder* boulder = m_playerRaycastData.m_boulder;
			if (boulder) m_targeted = boulder->getSquareIndx();
			break;
		}
			
		case SENTINEL_PLINTH_HIT:
		{
			m_plinthTargeted = true;
			break;
		}
			
	}
}


int Player::getTargetedSquare()
{
	return m_targeted;
}

// attempt to extract a specified amount of energy from the player (and return the amount actually extracted)
void Player::removeEnergy(float& deltaEnergy)
{
	if (m_state == PLAYER_ACTIVE)
	{
		float e0 = m_energy;
		m_energy -= deltaEnergy;
		
		if (m_energy <= 0.f)
		{
			m_energy = 0.f;
			
#ifdef PLAYER_INVULNERABILITY
			return;
#endif //PLAYER_INVULNERABILITY
			
			if (m_state == PLAYER_ACTIVE)
			{ 
				m_state = PLAYER_KILLED;
				deathTimer.start();	
			}
		}	
		
		deltaEnergy = e0 - m_energy;
	}
}

void Player::giveEnergy(float deltaEnergy)
{
	m_energy += deltaEnergy;
}

bool Player::finishedDying()
{
	return ( deathTimer.time() > GAMEOVER_DELAY );
}


bool Player::finishedWinning()
{
	return ( winTimer.time() > LEVELWON_DELAY );
}


/*
 
 Notes from C64 version:
 
 
 - start game with enough energy for 3 synthoids + 1 bolder
 - can only place synthoid on bolder if bolder top is raycasted
 - sentinel is dormant until something is done (e.g. create a synthoid, absorb something)
 - in 1st level, sentinel moves about once every 15 seconds, about 30 degrees each time
 - have to cast on topmost bolder (top or side) to add bolder or synthoid
 - Sentinel seems to start pointing away from player, which gives some "breathing space"
 
 
*/



/*
 List of executable actions:
 
 - Look around (move the pointer on the screen)
 - Absorb an object to gain its energy (point a square where an object is present and press A)
 - Create trees in empty squares (point the desired square and press T)
 - Create one or more boulders in empty squares (point the desired square and press B)
 - Absorb one or more boulders (point the boulder on the bottom of the stack and press A)
 - Create a new Synthoid shell in an empty square or on a boulder (point the desired place and press R)
 - Transfer consciousness to another Synthoid (point the Synthoid and press Q)
 - Hyperspace to a random part of the level at the expense of 3 units of energy (press H) 
 (note that the player may not hyperspace to a higher square; only one of equal or lower height)

 */

// Delay in seconds after creating an object before which the player can create another
const float OBJECT_CREATION_DELAY = 0.05f;




void Player::performRequestedUserAction( UserAction requestedAction )
{
	// perform requested user action
	switch ( requestedAction )
	{
		case ABSORB:                 absorbObject();            break;
		case CREATE_SYNTHOID_SHELL:  createSynthoidShell();     break;
		case TRANSFER_TO_SYNTHOID:   transferToSynthoidShell(); break;
		case CREATE_TREE:            createTree();              break;
		case CREATE_BOULDER:         createBoulder();           break;
		case HYPERSPACE:             hyperspace();              break;
		default: break;
	}	
}

void Player::_createSynthoidOnSquare(Square& square)
{
	if (!square.m_flat) return;
	
	bool created = square.createSynthoidShell();
	
	if (created)
	{
		m_energy -= Synthoid::getInitialEnergy();
		creationTimer.start();
		
		Synthoid* synthoid = static_cast<Synthoid*>( square.getSynthoidShell() );
		if (synthoid) synthoid->facePlayer();
	}	
}

/*
 
 BUG:  you can add an object on a boulder while the the sentinel is absorbing the boulder.
       Leading to an object standing on empty space.
 
 */

void Player::createSynthoidShell()
{
	// Disallow creation if less than OBJECT_CREATION_DELAY secs have elapsed since last creation
	if (creationTimer.isStarted() && creationTimer.time() < OBJECT_CREATION_DELAY) return;

	// Cannot create if no square targeted
	if (m_targeted<0) return;
	
	// Raycast against terrain only, to determine if base square is visible
	HitData H;
	m_pTerrain->raycast(m_viewPoint, m_crosshairs.m_dir, H, TERRAIN_ONLY); 
	if (!H.m_hit || H.m_square != m_targeted) return;
	
	// Cannot create if player has insufficient energy
	if ( m_energy < Synthoid::getInitialEnergy() ) return;
	
	Square& square = m_pTerrain->getSquare( m_targeted );
	_createSynthoidOnSquare(square);
}


const float PLAYER_SYNTHOID_TRANSFER_TIME = 1.5f;

void Player::continueTransfer()
{
	if (!m_initialSynthoid || !m_targetSynthoid) return;
	
	float t = transferTimer.time() / PLAYER_SYNTHOID_TRANSFER_TIME;

	if (t >= 1.0f) t = 1.0f;	
	m_viewPoint = m_transferTargetPosA     + t * ( m_transferTargetPosB     - m_transferTargetPosA     );
	m_azimuth   = m_transferTargetAzimuthA + t * ( m_transferTargetAzimuthB - m_transferTargetAzimuthA );
	m_phi       = m_transferTargetPhiA     + t * ( m_transferTargetPhiB     - m_transferTargetPhiA     );
	
	m_initialSynthoid->setTransparency(1.0f-t);
	m_targetSynthoid->setTransparency(t);
		
	if (t >= 1.0f) 
	{
		SetState(PLAYER_ACTIVE);
		m_targetSynthoid->deactivateDisplay();
		
		m_initialSynthoid->markAsOffLimits(false);
		m_targetSynthoid->markAsOffLimits(false);
	}	
}

static inline void _directionVecToAngles(const Vec4& v, float& azimuth, float& angle)
{
	float x = v[0];
	float y = v[1];
	float z = v[2];
	
	azimuth = 0.5f*M_PI - acos(z);
	angle = atan2(y, x);
}


bool Player::_transferToSynthoidOnSquare(Square& targetSquare)
{
	if (!targetSquare.m_flat) return false;
	
	// Square must contain a synthoid shell
	GameObject* targetSynthoid = targetSquare.getSynthoidShell();
	if (!targetSynthoid) return false;
	m_targetSynthoid = static_cast<Synthoid*>( targetSynthoid );
	
	// cannot transfer if target synthoid is being absorbed or materializing
	if ( m_targetSynthoid->getState() != OBJECT_NORMAL ) return false;
	
	// Move player viewpoint gradually to target synthoid position, and player viewpoint gradually to target viewpoint	
	Square& previousSquare = m_pTerrain->getPlayerSquare();
	
	m_pTerrain->setPlayerSquare(targetSquare.m_indx);
	m_transferTargetPosA = GetViewPoint();
	m_transferTargetPosB = m_targetSynthoid->getTranslation();
	m_transferTargetPosB[2] += SYNTHOID_EYE_LEVEL * Synthoid::getHeight();
	
	m_initialSynthoid = static_cast<Synthoid*>( previousSquare.getSynthoidShell() );
	
	// Make the initial/target synthoid objects off-limits to absorption/deletion while the player is transferring
	m_initialSynthoid->markAsOffLimits(true);
	m_targetSynthoid->markAsOffLimits(true);
	
	// Make the previously inhabited synthoid display object active
	m_initialSynthoid->activateDisplay();
		
	Vec4 transferTargetDirA = GetViewDirection();
	Vec4 transferTargetDirB = GetViewPoint() - m_targetSynthoid->getTranslation();
	transferTargetDirB.normalizeIfNotZero();
	
	// check that the new view direction is not too close to vertical		
	if ( transferTargetDirB[2] > avoid_cosine || transferTargetDirB[2] < -avoid_cosine )
	{
		// if so, point the new view direction along the horizontal direction towards the old point
		transferTargetDirB = GetViewPoint() - m_targetSynthoid->getTranslation();
		transferTargetDirB[2] = 0.0f;
		transferTargetDirB.normalizeIfNotZero();
	}
	
	// rotate old synthoid to look towards the new synthoid position
	Vec4 initialSynthoidViewDir = -1.0f * transferTargetDirB;
	m_initialSynthoid->faceDirection(initialSynthoidViewDir);
	
	// Convert target view vector to azimuth/angle
	_directionVecToAngles(transferTargetDirA, m_transferTargetAzimuthA, m_transferTargetPhiA);
	_directionVecToAngles(transferTargetDirB, m_transferTargetAzimuthB, m_transferTargetPhiB);

	SetState(PLAYER_TRANSFERRING);
	transferTimer.start();
	return true;
}

const float MIN_TIME_BETWEEN_SYNTHOID_TRANSFERS = 0.3f;

void Player::transferToSynthoidShell()
{
	static bool firstTransfer = true;
		
	// prevent two transfers within a small time interval, since the viewpoint
	// switching makes it possible to transfer again immediately after a transfer 
	if (!firstTransfer && transferTimer.time() < MIN_TIME_BETWEEN_SYNTHOID_TRANSFERS) return;
	
	// Cannot transfer if no square targeted
	if (m_targeted<0) return;
	
	Square& targetSquare = m_pTerrain->getSquare( m_targeted );
	if ( _transferToSynthoidOnSquare(targetSquare) )
	{
		firstTransfer = false;
	}
}

void Player::createBoulder()
{
	// Disallow creation if less than OBJECT_CREATION_DELAY secs have elasped since last creation
	if (creationTimer.isStarted() && creationTimer.time() < OBJECT_CREATION_DELAY) return;
	
	// Cannot create if no square targeted
	if (m_targeted<0) return;
	
	// Cannot create if player has insufficient energy
	if ( m_energy < Boulder::getInitialEnergy() ) return;
	
	Square& square = m_pTerrain->getSquare( m_targeted );
	if (!square.m_flat) return;
	
	bool created = square.addBoulder();
	
	if (created)
	{
		m_energy -= Boulder::getInitialEnergy();
		creationTimer.start();
	}	
}

void Player::createTree()
{
	
}



void Player::absorbObject()
{
	// Cannot absorb if no square targeted
	if (m_targeted<0 && !m_plinthTargeted) return;
	
	// Cannot absorb any more energy once Sentinel has been absorbed
	if ( m_pWorld->pSentinel == NULL ) return;
	
	// normal square is targeted (not Sentinel's plinth)
	if (!m_plinthTargeted)
	{
		Square& square = m_pTerrain->getSquare( m_targeted );
		if (!square.m_flat) return;
	
		// player cannot absorb from his own square
		if ( square.m_indx == m_pTerrain->getPlayerSquare().m_indx ) return;
		
		// Nothing to absorb if square is empty
		if (square.m_occupants.size() == 0) return;
		
		// cannot absorb sentinel square occupants by targeting plinth square
		if ( square.m_indx == m_pTerrain->getSentinelSquare().m_indx ) return;
		
		// Absorb highest 
		GameObject* object = square.getHighestOccupant();
		if (object) object->absorb();
	}
	
	// Sentinel's plinth is targeted - Victory!
	else
	{
		EventManager::Instance()->killSentinel();
	}
	
}


const float HYPERSPACE_ENERGY_COST = 0.3f;
const float MIN_TIME_BETWEEN_HYPERSPACES = 1.0f;

void Player::hyperspace()
{
	static bool firstHyperspace = true;
	
	// prevent two hyperspaces within a small time interval
	if (!firstHyperspace && hyperspaceTimer.time() < MIN_TIME_BETWEEN_HYPERSPACES) return;
	
	// Cannot hyperspace if player has insufficient energy to create a synthoid
	if ( m_energy < Synthoid::getInitialEnergy() ) return;

	// Hyperspace is also the mechanism for advancing the level
	if (m_state == PLAYER_DEFEATED_SENTINEL)
	{
		// wins the level 
		m_state = PLAYER_WINS_LEVEL;
		winTimer.start();
		return;
	}
	
	Square& square = m_pTerrain->getPlayerSquare();
	
	int indx = m_pTerrain->findRandomEmptySquareLowerThan(square);
	if (indx<0) return;
	
	Square& targetSquare = m_pTerrain->getSquare(indx);
	
	_createSynthoidOnSquare(targetSquare);
	_transferToSynthoidOnSquare(targetSquare);
	
	firstHyperspace = false;
	hyperspaceTimer.start();
}




