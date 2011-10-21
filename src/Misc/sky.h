
#ifndef SKY_H
#define SKY_H

#include "GameObject.h"
#include "TextureLibrary.h"


enum sky_directions
{
	SKY_FRONT,
	SKY_BACK,
	SKY_LEFT,
	SKY_RIGHT,
	SKY_UP,
	SKY_DOWN
};

class Sky 
{
	
public:	
	
	enum SkyType
	{	
		ROOM,
		OUTDOORS
	};
	
	Sky(SkyType type = OUTDOORS); //constructor
	~Sky(); //destructor
	void Init();
	void draw(void);
	
	// load assets
	void loadAssets();

private:

	//skybox
	struct SkyboxData
	{
		float sbox_corners[8][3];
		int sbox_faces[6][4];
		double sbox_faces_tx[4][2];
		float sbox_side_faces_tx[4][2];
		float sbox_up_faces_tx[4][2];
	};
		
	SkyboxData m_roomData;
	SkyboxData m_outdoorsData;

	void drawOutdoors();
	void drawRoom();
	
	//textures
	Texture *mTexture_sky[6];
	Texture *mTexture_wall;
	
	SkyType m_type;
};

#endif //SKY_H

