
#include "Timer.h"
#include "SDL/SDL.h"


/* ************************************** */

TimerRegistry* TimerRegistry::_instance = NULL;

TimerRegistry* TimerRegistry::Instance()
{
	if (_instance == NULL)
	{
		_instance = new TimerRegistry;
	}
	return _instance;
}

TimerRegistry::TimerRegistry() {}


void TimerRegistry::pause()
{
	set<Timer*>::iterator it = m_activeTimers.begin();
	
	while ( it != m_activeTimers.end() )
	{
		Timer* timer = *it;
		timer->pause();
		it++;
	}	
}

void TimerRegistry::unpause()
{
	set<Timer*>::iterator it = m_activeTimers.begin();
	
	while ( it != m_activeTimers.end() )
	{
		Timer* timer = *it;
		timer->unpause();
		it++;
	}	
}

void TimerRegistry::registerTimer( Timer* timer )
{
	m_activeTimers.insert(timer);
}

void TimerRegistry::unregisterTimer( Timer* timer )
{
	set<Timer*>::iterator it;
	it = m_activeTimers.find(timer);
	
	if ( it != m_activeTimers.end() )
	{
		m_activeTimers.erase(it);
	}
}



/* ************************************** */

inline float Timer::GetCurrentTime()
{
	return 1.0e-3*(float)SDL_GetTicks(); 
}


Timer::Timer()
{
    //Initialize the variables
    startTime = 0.0;
    pausedTime = 0.0;
    paused = false;
    started = false; 
	
	TimerRegistry::Instance()->registerTimer(this);
}

Timer::~Timer()
{
	TimerRegistry::Instance()->unregisterTimer(this);
}

void Timer::start()
{
    //Start the timer
    started = true;
	
	//Unpause the timer
    paused = false;
	
	//Get the current clock time
    startTime = GetCurrentTime();    
}

float Timer::time()
{
    //If the timer is running
    if( started )
    {
        //If the timer is paused
        if( paused )
        {
            //Return time elapsed between start and last pause time
            return pausedTime - startTime;
        }
        else
        {
            //Return non-paused time elapsed since the start time
            return GetCurrentTime() - startTime;
        }    
    }
    //If the timer isn't running
    return 0;    
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( started && !paused )
    {
        //Pause the timer
        paused = true;
		
     	//store the pause time
        pausedTime = GetCurrentTime();
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused )
    {
        //Unpause the timer
        paused = false;
		
        //update the startTime so that time 
        //gap during pause isn't counted
        startTime += GetCurrentTime() - pausedTime;
		
        //Reset the paused ticks
        pausedTime = 0.0;
    }
}


void Timer::stop()
{
    //Stop the timer
    started = false;
    //Unpause the timer
    paused = false;    
}

bool Timer::isStarted()
{
    return started;    
}

bool Timer::isPaused()
{
	return paused;    
}



















