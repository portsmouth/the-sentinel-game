

#ifndef PLAYER_H
#define PLAYER_H

#include "Timer.h"
#include "Vec4.h"
#include "Game_enums.h"

#include <GLConsole.h>

class Player {
	
protected:
	
	// SINGLETON
	Player();	
	
private:
	
	static Player* _instance;
	
public:	
	
	static Player* Instance();
	
	friend class EventManager;
	
	
	// Single global instance so glut can get access
	GLConsole theConsole;
	
	
	// player viewing
	Vec4 GetViewDirection();
	const Vec4& GetViewPoint();
	Vec4 GetLocalX();
	Vec4 GetLocalY();
	void RotateViewAboutVertical(float dAngle);
	void RotateViewAboutHorizontal(float dAzimuth);
	void setView(const Vec4& v);
	
	// player movement
	void Move();
	void goForward();
	void goBackward();
	void goLeft();
	void goRight();
	

	// state queries
	Player_States GetState();
	bool finishedDying();
	bool finishedWinning();
		
	// selection crosshairs
	struct Crosshairs
	{
		void init()
		{
			m_x = 0.5f;
			m_y = 0.5f;
		}
		float m_x, m_y;  // crosshair coords in fraction of screen width, (0,0) = lower left, (1,1) = upper right
		Vec4 m_dir;  // corresponding direction vector of ray
	};
	Crosshairs m_crosshairs;
	const float CROSSHAIR_VERBOTEN_X;
	float CROSSHAIR_VERBOTEN_Y;
	float dragZoneX, dragZoneY;
	void makeCrosshairRay();
	void MoveCrossHairs(float dx, float dy);
	void handleCrosshairs();
	bool crosshairsInDragzone();
	float perFrameDragAngle();
	
	// misc
	float getEnergy();
	void SetObserverProjection();
	void InitObserver();

	// player game actions
	void performRequestedUserAction( UserAction requestedAction );
	void createSynthoidShell();
	void transferToSynthoidShell();
	void continueTransfer();
	void createBoulder();
	void createTree();
	void absorbObject();
	void hyperspace();
	
	// raycasting
	void raycastTerrain();
	int getTargetedSquare();
	
	// damage
	void removeEnergy(float& deltaEnergy);
	void giveEnergy(float deltaEnergy);
	
private:
			
	// utility functions
	void _createSynthoidOnSquare(class Square& square);
	bool _transferToSynthoidOnSquare(class Square& square);
		
	// observer parameters
	Vec4 m_viewPoint;
	float aspect_ratio,near,far,fovy;
	float frustum_width, frustum_height;
	float avoid_cosine;

	// view direction on sphere
	float m_azimuth;
	float m_phi;
	
	// goal/target position/view during synthoid transfer
	Vec4 m_transferTargetPosA, m_transferTargetPosB;
	float m_transferTargetAzimuthA, m_transferTargetAzimuthB;
	float m_transferTargetPhiA, m_transferTargetPhiB;
	
	// initial/target synthoids during transfer 
	class Synthoid* m_initialSynthoid;
	class Synthoid* m_targetSynthoid;
	
	class World *m_pWorld;
	class Terrain *m_pTerrain;

	// player game state
	void SetState(Player_States newState);
	enum Player_States m_state;
	float m_energy;
	
	// raycasting
	HitData m_playerRaycastData;
	int m_targeted;
	bool m_plinthTargeted;
	
	Timer creationTimer;
	Timer deathTimer;
	Timer winTimer;
	Timer transferTimer;
	Timer hyperspaceTimer;
	

};

#endif //PLAYER_H
