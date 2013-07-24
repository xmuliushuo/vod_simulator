#ifndef TIMER_H_
#define TIMER_H_

#include "vod.h"

struct TimerEvent{
	long long lefttime;
	int sockfd;
	bool isnew;
	char buffer[MESSAGELEN];
};


class Timer {
public:
	Timer();
	~Timer();
private:
};

#endif