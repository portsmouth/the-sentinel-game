
#include "Sentinel.h"

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


Enemy::Enemy( int squareIndx, Vec4& translation ) :

m_behaviourState(ENEMY_SEARCHING),
m_rotationState(ENEMY_STALLED),

GameObject(OBJECT_ENEMY, squareIndx)

{
}

Enemy::~Enemy()
{
}

void Enemy::think()
{
}

Enemy_Types Enemy::getEnemyType()
{
}


void Enemy::turnToFacePoint( const Vec4& targetPoint )
{	
	// construct projected gaze direction
	Vec4 gazeDir( cos(m_rotation * degToRad), sin(m_rotation * degToRad), 0.f );
	
	// construct projected direction from sentinel-eye to player square center
	Vec4 toTarget = targetPoint - m_eyePosition;
	toTarget[2] = 0.f;
	toTarget.normalizeIfNotZero();

	// angle to player about positive z axis, in degrees
	// (signs not a problem, we know we're in the right quadrant now since the target must be visible
	//  and is thus within a sector angle from the sentinel)
	float smallAngleApprox = radToDeg * (gazeDir % toTarget) * Vec4(0,0,1);
	const float ENEMY_TURN_ANGULAR_ACCURACY = 0.01f; //degrees
	
	if ( fabs(smallAngleApprox) > ENEMY_TURN_ANGULAR_ACCURACY ) 
	{
		float availableTurnAngle = turnTimer.time() * m_rotationRate;
		if ( fabs(smallAngleApprox) < availableTurnAngle )
		{
			m_rotation += smallAngleApprox;
		}
		else
		{
			float sign = smallAngleApprox > 0.f ? 1.f : -1.f;
			m_rotation += sign * availableTurnAngle;
		}
		
		keepRotAngleInRange();
		m_displayObject->setRotation(m_rotation + m_rotationOffset);
	}
	
	turnTimer.start();
}


void Enemy::absorbPlayer()
{
	Square& playerSquare = Game::pWorld->pTerrain->getPlayerSquare();
	turnToFacePoint( playerSquare.m_center );
	
	float dE = objectAbsorbTimer.time() * m_playerEnergyAbsorptionRate;
	Player::Instance()->removeEnergy( dE );
	m_energy += dE;
	m_initialEnergy = m_energy;
	
	objectAbsorbTimer.start();
}


void Enemy::_drawLine_DEBUG()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG );
	glLineWidth(2.0f);

	Square* square;
	if ( m_behaviourState == ENEMY_ABSORBING_OBJECT )
	{
		square = &Game::pWorld->pTerrain->getSquare( m_objectBeingAbsorbed->getSquareIndx() );
	}
	else
	{
		square = &Game::pWorld->pTerrain->getPlayerSquare();
	}
	
	Vec4 A = m_eyePosition;
	Vec4 C = square->m_center;
	Vec4 D = C - A;
	float L = D.length3();
	D.normalizeIfNotZero();
	
	float Z = m_eyePosition[2] - C[2];
	Vec4 dir( cos(m_rotation * degToRad), sin(m_rotation * degToRad), 0.f );
	
	float X = fabs(L * (dir * D));
	Vec4 F(X * dir[0], X * dir[1], -Z);
	Vec4 B = A + F;
	Vec4 G = A + 10.f*dir;
	Vec4 E = A + 10.f * D;
	
	glColor3d(1.0,0.0,0.0);
	glBegin(GL_LINES);
	glVertex3fv(&A[0]);
	glVertex3fv(&B[0]);
	glEnd();
	
	glColor3d(1.0,1.0,0.0);
	glBegin(GL_LINES);
	glVertex3fv(&A[0]);
	glVertex3fv(&G[0]);
	glEnd();
	
	// highlight targeted square
	glBegin(GL_TRIANGLES);
	glColor3f(0,0,1);
	if (m_hitIndx>=0) Game::pWorld->pTerrain->highlightSquare(m_hitIndx); 
	glEnd();
	
	glPopAttrib();
}


bool Enemy::playerInVisibleSector()
{
	// construct projected gaze direction
	Vec4 dir( cos(m_rotation * degToRad), sin(m_rotation * degToRad), 0.f );
	
	// construct projected direction from sentinel-eye to player location	
	const Vec4& playerLoc = Player::Instance()->GetViewPoint();
	
	Vec4 toPlayer = playerLoc - m_eyePosition;
	Vec4 toPlayer_proj = toPlayer;
	toPlayer_proj[2] = 0.f;
	toPlayer_proj.normalizeIfNotZero();
	
	// if player does not lie in sentinel view sector, he is not visible
	return !(dir * toPlayer_proj < m_viewRangeCosine);
}

// raycast to determine if landscape obscures player's head from sentinel viewpoint
bool Enemy::playersHeadIsVisible()
{
	const Vec4& P = Player::Instance()->GetViewPoint();
	HitData data;
	return Game::pWorld->pTerrain->isTargetVisibleFromOrigin( m_eyePosition, P, data );
}

// raycast to determine if landscape obscures player's square from sentinel viewpoint
bool Enemy::playersSquareIsVisible()
{
	Vec4& C = Game::pWorld->pTerrain->getPlayerSquare().m_center;
	HitData data;
	return Game::pWorld->pTerrain->isTargetVisibleFromOrigin( m_eyePosition, C, data );
}


void Enemy::rotate()
{
	switch( m_rotationState )
	{
		case ENEMY_STALLED:
		{	
			if (stalledTimer.time() > m_rotationStallTime)
			{
				m_baseRot = m_rotation;
				m_rotationState = ENEMY_ROTATING;
				rotationTimer.start();
			}
			break;
		}
			
		case ENEMY_ROTATING:
		{	
			float dT = rotationTimer.time();
			
			if (dT > m_rotationActiveTime)
			{
				m_rotationState = ENEMY_STALLED;
				stalledTimer.start();
				break;
			}
			
			m_rotation = m_baseRot + dT * m_rotationRate;
			keepRotAngleInRange();
			
			m_displayObject->setRotation(m_rotation + m_rotationOffset);
			
			break;
		}
	}
}


void Enemy::kill()
{
	if (m_state == OBJECT_NORMAL)
	{
		m_state = OBJECT_ABSORBING;
		m_behaviourState = ENEMY_BEING_ABSORBED;
		m_absorbTimer.start();
	}
}





