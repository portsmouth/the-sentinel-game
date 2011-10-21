
#ifndef SOUND_H
#define SOUND_H

#include <SDL_mixer/SDL_mixer.h>
#include <vector>

enum GeneralSound
{
	PlayerFire,
	EnemyFire,
	EnterGrid,
	Enemy_destroyed
};

enum ChannelGroup
{
	ChannelGroup_General = 1,
	ChannelGroup_Collisions,
	ChannelGroup_Scrapes
};

class Sound {
	
protected:
	
	// SINGLETON
	Sound();	
	
private:
	
	static Sound* _instance;
	
public:	
	
	static Sound* Instance();
	
	~Sound(); //destructor
	void PlayGeneralSound(GeneralSound inSound, float inVolume);
	float ComputeSoundVolume(float *sourceposition);
	
private:
	std::vector<Mix_Chunk*>	GeneralSounds;
	int FindChannelInGroup(ChannelGroup inGroup);
	float SoundVolume;
};

#endif //SOUND_H
