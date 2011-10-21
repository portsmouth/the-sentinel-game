

#ifndef MEANIE_H
#define MEANIE_H

#include "Enemy.h"

class Meanie : public Enemy
{
	
public:
	
	Meanie( int squareIndx, Vec4& translation, float rotation );
	~Meanie();
	
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
	
	static Model*   m_meanieModel;
	static Texture* m_meanieTexture;
	static float m_height;
	
};


#endif //MEANIE_H