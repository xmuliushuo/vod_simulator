#include "spread.h"
#include "utils.h"
#include "lbserver.h"
#include "log.h"

void *StartSpreadThread(void *arg)
{
	Spread *spread = (Spread *)arg;
	spread->SpreadProcess();
	return NULL;
}

Spread::Spread(SpreadNode &spreadNode):mSpreadNode(spreadNode)
{
	mSpreadNode.isFinished = false;
	pthread_mutex_init(&mMutex,NULL);
	pthread_cond_init(&mCond,NULL);

	Pthread_create(&mPid, NULL, &StartSpreadThread, this);
	Pthread_detach(mPid);
}


Spread::~Spread()
{
	pthread_mutex_destroy(&mMutex);
	pthread_cond_destroy(&mCond);
}

void Spread::Init()
{
}

void Spread::SpreadProcess()
{
	StartSpread();
	FinishSpread();
}

double Spread::RemainTime(double remainTime, struct timespec &timeout, struct timeval now)
{
	int tempusec = now.tv_usec + (remainTime - (int)remainTime) * 1000000;
	timeout.tv_sec = now.tv_sec + (int)remainTime + tempusec / 1000000;
	timeout.tv_nsec = (tempusec % 1000000) * 1000;
	return remainTime;
}

void Spread::StartSpread()
{
	double remainTime;// 剩余扩散时间
	int oldLoad;
	struct timeval now1,now2;
	struct timespec timeout;// pthread_cond_timedwait

	pthread_mutex_lock(&mMutex);

	oldLoad = mSpreadNode.currentLoad;
	remainTime = mSpreadNode.fileLength * oldLoad * 1.0 / mSpreadNode.diskBand;
	//复制开始时间
 	mSpreadNode.startTime = getRelativeTime();
	while(remainTime > 0)
	{
		oldLoad = mSpreadNode.currentLoad;// 睡眠前记录当前负载
		gettimeofday(&now1, NULL);
		RemainTime(remainTime, timeout, now1);
		int ret = pthread_cond_timedwait(&mCond, &mMutex, &timeout);
		if (ETIMEDOUT == ret)
		{
			remainTime = 0.0;
			break;
		}
		else if (ret != 0)
		{
			LOG_INFO("pthread_cond_timedwait error,exit.");
			exit(1);
		}
		gettimeofday(&now2, NULL);
		double timeslips = getTimeSlips(&now2, &now1);
		remainTime -= timeslips;
		if(remainTime < 0)
			break;
		remainTime = remainTime * mSpreadNode.currentLoad / oldLoad;
	}
	// 复制结束时间
 	mSpreadNode.endTime = getRelativeTime();
	mSpreadNode.isFinished = true;
	pthread_mutex_unlock(&mMutex);
}

void Spread::FinishSpread()
{
	mSpreadNode.lb->FinishSpreadSendMsg(&mSpreadNode);
}

// 该函数由lb主线程调用，会修改mSpreadNode的 srcServerId targetServerId currentLoad
void Spread::NotifySpread(int serverId, int load)
{
	int newload;
	pthread_mutex_lock(&mMutex);
	if(mSpreadNode.isFinished)
	{
		goto out;
	}
	if(mSpreadNode.srcServerId != serverId && mSpreadNode.targetServerId != serverId)
	{
		goto out;
	}
	else if(mSpreadNode.srcServerId == serverId) // srcServerId的负载变化
	{
		mSpreadNode.srcServerLoad = load;
	}
	else if(mSpreadNode.targetServerId == serverId)
	{
		mSpreadNode.targetServerLoad = load;
	}
	newload = mSpreadNode.srcServerLoad > mSpreadNode.targetServerLoad ? mSpreadNode.srcServerLoad : mSpreadNode.targetServerLoad;
	if(newload == mSpreadNode.currentLoad)
	{
		goto out;
	}
	else
	{
		mSpreadNode.currentLoad = newload;
		pthread_cond_signal(&mCond);
	}
out:
	pthread_mutex_unlock(&mMutex);
	return;
}
