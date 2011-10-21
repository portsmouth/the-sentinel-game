/*
 *  GameObject.h
 *
 *  Created by Jamie Portsmouth on 14/01/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Game_enums.h"
#include "Maths.h"
#include "Timer.h"
#include "DisplayObject.h"

class GameObject
{
	
public:	
	
	GameObject( Object_Types type, int squareIndx );
	virtual ~GameObject();

	// asset loading: static function which MUST be called before instantiation of any GameObject
	static void loadAssets();
	
	// access
	virtual Object_Types getType();
	virtual const Vec4& getTranslation();
	virtual Object_States getState();
	virtual int getSquareIndx();
	virtual const Vec4& getSentinelRelativeDirection();
	virtual const AABB& getWorldAABB();
	
	// absorption handling
	void removeEnergy( float& dE );
	virtual void absorb();
	virtual void updateAbsorptionByPlayer();
	virtual void stopAbsorption();
	virtual void markForDeletion();
	virtual void markAsOffLimits(bool toggle);
	float getInitialEnergy();
	
	// materialization (for fade in effect on creation)
	virtual void updateMaterialization();
	virtual void markForMaterialization();
	
	// display activation/de-activation
	virtual void deactivateDisplay();
	virtual void activateDisplay();
	
protected:
		
	virtual void updateTransparency();
	virtual void setSentinelRelativeDirection();
	
	// type
	Object_Types m_type;
		
	// location in square grid
	int m_squareIndx;
		
	// unit direction vector in plane, from sentinel to this object
	Vec4 m_sentinelRelativeDirection;
	
	// pointer to the DisplayObject for this GameObject
	DisplayObject* m_displayObject;
	
	// absorption handling
	Object_States m_state;
	Timer m_absorbTimer;

	float m_initialEnergy;
	float m_energy;
	
	// materialization handling
	Timer m_materializationTimer;
};


inline const Vec4& GameObject::getTranslation()
{
	return m_displayObject->m_translation;
}

inline Object_States GameObject::getState()
{
	return m_state;
}

inline Object_Types GameObject::getType()
{
	return m_type;
}

inline int GameObject::getSquareIndx()
{
	return m_squareIndx;
}

inline const Vec4& GameObject::getSentinelRelativeDirection()
{
	return m_sentinelRelativeDirection;
}


/************************************************************/
class Synthoid : public GameObject
/************************************************************/
{
	
public:
	
	Synthoid( int squareIndx, Vec4& translation );
	
	static void loadAssets();
	static float getHeight();
	static float getInitialEnergy();
	
	void setTransparency(float transparency);
	
	void facePlayer();
	void faceDirection(Vec4& dir);
	
private:
		
	static Model*   m_synthoidModel;
	static Texture* m_synthoidTexture;
	
	float m_rotationOffset;
	float m_rotation;
	
	static float m_height;
};

// fraction of vertical bounding box height to view point height
const float SYNTHOID_EYE_LEVEL = 0.9f;

const float SYNTHOID_ENERGY_CONTENT = 3.0f;

/************************************************************/
class Boulder : public GameObject
/************************************************************/
{
	
public:

	Boulder( int squareIndx, Vec4& translation );
	
	static void loadAssets();
	static float getHeight();
	static float getInitialEnergy();
	
private:
		
	static Model* m_boulderModel;
	static Texture* m_boulderTexture;
	
	static float m_height;
};



/************************************************************/
class Tree : public GameObject
/************************************************************/
{
	
public:
	
	Tree( int squareIndx, Vec4& translation );

	static void loadAssets();
	static float getInitialEnergy();
	
private:

	static Model*   m_treeModel;
	static Texture* m_treeTexture;
};


/************************************************************/
class Plinth : public GameObject
/************************************************************/
{
	
public:
	
	Plinth( int squareIndx, Vec4& translation );
	
	static void loadAssets();
	static float getHeight();
	static struct AABB& getAABB();

private:
		
	static Model*   m_plinthModel;
	static Texture* m_plinthTexture;
	
	static float m_height;
};







#endif //GAMEOBJECT_H

