#ifndef TIMER_H_
#define TIMER_H_

#include <pthread.h>

#include <list>

#include "message.h"

using namespace std;

struct TimerEvent{
	long long left_time;
	int sockfd;
	bool isnew;
	char buffer[MESSAGELEN];
};

class Timer {
public:
	static Timer * GetTimer();
	~Timer();
	void RegisterTimer(TimerEvent &event);
	void TimerThread();
private:
	Timer();
	void WakeTimer(TimerEvent &event);
	static Timer *m_timer;
	list<TimerEvent> m_timerlist;
	pthread_mutex_t m_timer_mutex;
	pthread_cond_t m_timer_cond;
};

#endif