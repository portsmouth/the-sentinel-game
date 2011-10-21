

#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
using std::vector;

#include "GameObject.h"
#include "Timer.h"


enum Enemy_Types
{
	ENEMY_SENTINEL,
	ENEMY_MEANIE,
	ENEMY_SENTRY
};

enum Enemy_Behaviour_States 
{
	ENEMY_SEARCHING, 
	ENEMY_SEES_PLAYER,
	ENEMY_ABSORBING_PLAYER,
	ENEMY_ABSORBING_OBJECT,
	ENEMY_BEING_ABSORBED
};

enum Enemy_Rotation_States
{
	ENEMY_ROTATING,
	ENEMY_STALLED
};


class Enemy : public GameObject
{
	
public:
	
	Enemy( int squareIndx, Vec4& translation );
	virtual ~Enemy();	
	
	virtual Enemy_Types getEnemyType();
	
	virtual Enemy_Behaviour_States getEnemyBehaviourState()
	{
		return m_behaviourState;
	}
	
	virtual void think();
	virtual void kill();
	virtual void _drawLine_DEBUG();
	
protected:

	virtual void rotate();
	virtual void turnToFacePoint( const Vec4& targetPoint );	
	
	virtual bool playerInVisibleSector();
	virtual bool playersHeadIsVisible();
	virtual bool playersSquareIsVisible();

	virtual void absorbPlayer();
	virtual void keepRotAngleInRange();
	
	float m_viewRangeCosine;
	float m_viewRange;
	float m_viewRangeHalf;
	
	float m_rotation;       //rotation angle in degrees, [0, 360.0]
	float m_rotationOffset; //model rotation needed to put enemy facing along +x
	float m_baseRot;
	float m_rotationRate;
	float m_rotationActiveTime;
	float m_rotationStallTime;
	float m_playerEnergyAbsorptionRate;
	
	float m_desiredEnergy;
	int m_hitIndx;
	Vec4 m_eyePosition; //Enemy's viewpoint
	long m_objectAbsorbSeed;
	GameObject* m_objectBeingAbsorbed;
	
	enum Enemy_Behaviour_States m_behaviourState;
	enum Enemy_Rotation_States m_rotationState;
	
	vector<int> m_sectorSquares;
	vector <GameObject*> m_seenObjects;
	
	Timer rotationTimer;
	Timer stalledTimer;
	Timer turnTimer;
	Timer objectAbsorbTimer;
	Timer objectAbsorptionDecisionTimer;
};

inline void Enemy::keepRotAngleInRange()
{
	if      (m_rotation < 0.f)   m_rotation += 360.f;
	else if (m_rotation > 360.f) m_rotation -= 360.f;
}

#endif //ENEMY_H

