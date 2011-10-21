

#ifndef SENTRY_H
#define SENTRY_H

#include "Enemy.h"

class Sentry : public Enemy
{
	
public:
	
	Sentry( int squareIndx, Vec4& translation, float rotation );
	~Sentry();
	
	virtual void think();
	virtual Enemy_Types getEnemyType();
	
	static void loadAssets();
	static float getInitialEnergy();
	static float getHeight();
				
private:
		
	static Model*   m_sentryModel;
	static Texture* m_sentryTexture;
	static float m_height;
	
};



#endif //SENTRY_H