/*
 * myclient.h
 *
 *  Created on: 2013-2-21
 *      Author: zhaojun
 */

#ifndef MYCLIENT_H_
#define MYCLIENT_H_

#include "mymessage.h"
#include "mytimer.h"
#include "modelassemble.h"
#include "faketran.h"

#include "dbuffer.h"
#include "dbufferlru.h"
#include "dbufferlrut.h"
#include "dbufferlfut.h"
#include "dbuffermfut.h"
#include "dbufferlfru.h"
#include "dbufferlfu.h"
#include "dbufferplfu.h"
#include "dbuffermfu.h"
#include "dbufferlrfu.h"
#include "dbufferfifo.h"
#include "dbufferdwq.h"
#include "dbufferdwqr.h"
#include "dbufferdwk.h"
#include "dbufferdwkr.h"
#include "dbufferdw.h"
#include "dbufferdwr.h"
#include "dbufferdwcc.h"
#include "dbufferdwcct.h"
#include "dbufferlrur.h"
#include "dbuffermfur.h"
#include <sys/socket.h>
#include <pthread.h>

#include <fstream>

extern MyTimer globalTimer;

class MyClient{
public:
	MyClient(ModelAssemble *model,int blockSize,int perSendSize,
			double bandWidth,int blockNums,int clientNum,int serverPort,int clientPort,int devNums,
			int clientNums,char **clusterAddress,char *serverAddress,char *bufferStrategy,
			int period,double lrfuLambda,bool isRepeat,int preFetch,bool special,bool isStartTogether);
	~MyClient();

	void Init();
	void Run();
	void DealWithMessage(char *buffer,int length);
	void Play();
	void GetNextBlock();
	bool SearchTheReqList(int fileId,int segId);
	void BufferReset();

	int getClientNum() const {
		return mClientNum;
	}

	pthread_t GetTid(){return mtid;}

//	int JudgeCluster(char *address);
private:
//	int mListenSockFd[2];
	bool mIsReadFin;
	int mBufferResetFd[2];

	int mNetSockFd;
	int mMyPort;
	int mClientPort;

	int mAskNum;

	char *mClusterAddress[MAX_DEV_NUM];
	int mDevNums;
	int mClientNums;

	char *mServerAddress;
	int mServerPort;

	int mPlaySockFd[2];

	int mEpollFd;

	ModelAssemble *mModel;
	DBuffer *mDbuffer;
	FakeTran mFakeTran;

	list<ClientReqBlock> mReqList;
	int mBlockSize;
	int mPerSendSize;
	list<ClientReqBlock>::iterator mCurReqBlock;

	int mFileId;
	int mSegId;
	int mClientNum;
	int mBlockNum;
	bool mIsPlaying;
	double mBitRate;
	int mMaxSegId;

	int mPlayerStatus;
	int mJumpSeg;

	int mLinkedNums;
	double mBand;

	int mPreFetch;

	ClientInfoBlock mClientInfo[MAX_CLIENT_NUM + 1];

	pthread_t mtid;

	string mFileName;
	ofstream mOFs;
	ofstream mRecordFs;
	bool mDelay;
	timeval mDelayBeginTime;
	timeval mDelayEndTime;

	char *mBufferStrategy;

	int mPeriod;
	double mLrfuLambda;
	bool mIsRepeat;

	int mHitTimes;
	int mTotalTimes;
	bool mSpecial;

	int mOldFileId;
	int mOldSegId;
	
	int mDload1;
	int mDload2;
	bool mIsStartTogether;

	bool mIsFirstStart;
};

#endif /* MYCLIENT_H_ */
