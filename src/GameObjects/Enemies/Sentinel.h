

#ifndef SENTINEL_H
#define SENTINEL_H

#include "Enemy.h"

class Sentinel : public Enemy
{
	
public:
	
	Sentinel( int squareIndx, Vec4& translation );
	~Sentinel();
	
	virtual void think();
	virtual Enemy_Types getEnemyType();
	
	static void loadAssets();
	static float getInitialEnergy();
	static float getHeight();
				
private:
		
	bool objectsVisible();
	bool wishToAbsorbAnObject();
	void chooseObjectToAbsorb();
	void absorbObject();
	void generateTree();
		
	static Model*   m_sentinelModel;
	static Texture* m_sentinelTexture;
	static float m_height;
	
	Timer treeGenerationTimer;

};


#endif //SENTINEL_H