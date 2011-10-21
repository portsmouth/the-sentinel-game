
#ifndef TIMER_H
#define TIMER_H

#include <set>
using namespace std;

class TimerRegistry
{

protected:
	
	// SINGLETON
	TimerRegistry();	
	
private:
	
	static TimerRegistry* _instance;
	
public:	
	
	static TimerRegistry* Instance();	
	~TimerRegistry();
	
	void pause();
	void unpause();
	
	void registerTimer( class Timer* timer );
	void unregisterTimer( class Timer* timer );
	
private:
	
	set<class Timer*> m_activeTimers;
	
};


// Simple timer class 
// - automatically registers itself with the TimerRegistry on construction, and unregisters on destruction
class Timer
{
public:
    
    Timer();
	~Timer();
	
    // The various clock actions
    void start();
    void stop();
    void pause();
    void unpause();
    
    // Get the number of time units since the timer started or was paused 
    float time();
    
    // Checks the status of the timer
    bool isStarted();
    bool isPaused();  
		
private:
	
	inline float GetCurrentTime();

	// The clock time when the timer started 
	float startTime;
    
    // The clock time when the timer was paused
    float pausedTime;
    
    // The timer status
    bool started;
    bool paused;
};


#endif //TIMER_H
