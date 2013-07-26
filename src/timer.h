#ifndef TIMER_H_
#define TIMER_H_

#include "utils.h"

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