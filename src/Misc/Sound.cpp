

#include "Sound.h"

#include "Game.h"
#include "FileUtils.h"


#include "Player.h"
#include "World.h"
#include "terrain.h"

#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

#define kPlayerFire "playerfire.ogg"
#define kEnterGrid "entergrid.ogg"
#define kEnemyFire "enemyfire.ogg"
#define kEnemyDestroyed "enemy_destroyed.ogg"

inline float SQR(float x) {
	return (x) == 0.0 ? 0.0 : (x)*(x);
}



Sound* Sound::_instance = NULL;

Sound* Sound::Instance()
{
	if (_instance == NULL)
	{
		_instance = new Sound;
	}
	return _instance;
}

Sound::Sound() //constructor
{
	
	if ((int)Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 256) == -1)	// 2 == stereo
	{
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		exit(1);
	}
	
	Mix_AllocateChannels(8);
	int num_channels = Mix_GroupChannels(0, 3, ChannelGroup_General);
	num_channels = Mix_GroupChannels(4, 5, ChannelGroup_Collisions);
	num_channels = Mix_GroupChannels(6, 7, ChannelGroup_Scrapes);
	SoundVolume = 1.0;
	
	//load sounds
	string path = FileUtils::GetApplicationResourcesFolderPath() + "/Sounds/";

	const char *general_sounds[] =
	{
		// WARNING: keep this in sync with the GeneralSound enum in the header!
		kPlayerFire,
		kEnemyFire,
		kEnterGrid,
		kEnemyDestroyed,
		NULL
	};
	
	const char **ptr = general_sounds;
	while(*ptr)
	{
		Mix_Chunk *chunk = Mix_LoadWAV((path + *ptr++).c_str());
		GeneralSounds.push_back(chunk);
	}
		
}



/* If inVolume is greater than 1.0, the maximum volume will be used.  
   If less that zero, nothing wlil happen. */
void Sound::PlayGeneralSound(GeneralSound inSound, float inVolume)
{
	if(inSound >= 0 and (unsigned int)inSound < GeneralSounds.size() and GeneralSounds[inSound] != NULL)
	{
		int channel = FindChannelInGroup(ChannelGroup_General);
		if(channel != -1)
		{
			Mix_Volume(channel, int(float(MIX_MAX_VOLUME) * inVolume * SoundVolume));
			Mix_Chunk *chunk = GeneralSounds[inSound];
			channel = Mix_PlayChannel(channel, chunk, 0);
		}
	}
}


/* Looks for either the first available channel in the group, or failing that, 
   the oldest one still playing. It may still return -1, in which case you probably 
   should just not play the sound. */
int Sound::FindChannelInGroup(ChannelGroup inGroup)
{
	int channel = Mix_GroupAvailable(inGroup);
	if(channel == -1)
		channel = Mix_GroupOldest(inGroup);
	
	return channel;
}

float Sound::ComputeSoundVolume(float *sourceposition)
{
	Vec4 playerpos = Player::Instance()->GetViewPoint();
	
	float sounddistance = 0.0;
	for (int a=0;a<3;a++) {
		sounddistance += SQR( sourceposition[a]-playerpos[a] );
	}				
	sounddistance = sqrtf(sounddistance);
	float tes = Game::pWorld->pTerrain->getTerrainEdgeSize();
	float hearingrange = tes;
	float soundvolume;
	if (sounddistance>hearingrange) {
		soundvolume = 0.0;
	}
	else 
	{
		soundvolume = 1.0-sounddistance/hearingrange;
	}
	return soundvolume;
}

Sound::~Sound() //destructor
{
	for(unsigned int index = 0; index < GeneralSounds.size(); ++index)
	{
		if(GeneralSounds[index]) Mix_FreeChunk(GeneralSounds[index]);
	}
	
	Mix_CloseAudio();
}
