#ifndef SPREAD_H
#define SPREAD_H

#include <pthread.h>

class LBServer;

typedef struct SpreadNode{
	int fileId;
	int srcServerId;
	int targetServerId;
	int srcServerLoad;
	int targetServerLoad;
	int currentLoad;
	int diskBand;
	int fileLength;//playLen;
	LBServer *lb;
	bool isFinished;
	double startTime;
	double endTime;
} SpreadNode;


class Spread{
public:
	Spread(SpreadNode &spreadNode);
	~Spread();
	void Init();

	double RemainTime(double remainTime, struct timespec &timeout, struct timeval now);
	void StartSpread();
	void FinishSpread();
	void SpreadProcess();

	void NotifySpread(int serverId,int load);		 

private:
	SpreadNode mSpreadNode;

	pthread_t mPid;
	pthread_mutex_t mMutex;
	pthread_cond_t mCond;
};



#endif
