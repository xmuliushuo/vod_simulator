/*
 * mytimer.h
 *
 *  Created on: 2013-2-19
 *      Author: zhaojun
 */

#ifndef MYTIMER_H_
#define MYTIMER_H_

#include "myheap.h"
#include <pthread.h>
#include <functional>

inline bool operator < (const TimerEvent &event1,const TimerEvent &event2){
	return event1.leftTime < event2.leftTime;
}

template <class T>
struct myless{
	bool operator()(const T &x,const T &y) const {return x < y;}
};

class MyTimer{
public:
	MyTimer(int multiple);
	~MyTimer();
	void RegisterTimer(TimerEvent event);
	void WaitTimer();
	void WakeTimer(TimerEvent event);
	void CancelTimer(TimerEvent event);
	void Run();

	double getMultiple() const {
		return m_multiple;
	}

	void setMultiple(double multiple) {
		m_multiple = multiple;
	}
private:
	Myheap< TimerEvent,myless<TimerEvent> > m_heap;
	pthread_t m_tid;
	pthread_mutex_t m_vectMutex;
	pthread_cond_t m_vectCond;
	double m_multiple;
};

#endif /* MYTIMER_H_ */
