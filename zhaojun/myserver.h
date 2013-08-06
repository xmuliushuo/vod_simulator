/*
 * myserver.h
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */

#ifndef MYSERVER_H_
#define MYSERVER_H_

#include "dataserver.h"
#include "mytimer.h"
#include "faketran.h"
#include "mymessage.h"

#include <sys/socket.h>
#include <list>
#include <fstream>

class MyServer{
public:
	MyServer(double bandWidth,int blockSize,int perSendSize,bool isP2POpen,
			int fileNum,int maxLength,int minLength,double minbitRate,double maxBitRate,int serverPort,int clientPort,int devNums,
			int clientNums,char **clusterAddress,int takeSampleFre,bool isUseRealDevice,int period,double lrfuLambda,
			char *bufferStrategy,int blockNums);
	~MyServer();
	void Init();
	void Run();
	void DealWithMessage(char *buf,int length);
	void GetNextBlock();
	void TakeSample();
	pthread_t GetTid(){return mtid;}
	list<ClientReqBlock>::iterator SearchTheReqList(int fileId,int segId);
	void ReadFromDevice();
	void BufferReset();
private:
//	int mListenSockFd[2];//AF_UNIX协议，1号对外公布，0号用于epoll循环
	int mReadDevice[2];
	int mREpollFd;

	int mTakeSample[2];
	int mTakeSampleFre;

	int mNetSockFd;
	int mServerPort;
	int mClientPort;

	char *mClusterAddress[MAX_DEV_NUM];
	int mDevNums;
	int mClientNums;

	DataServer *mDataServer;
	double mBand;
	FakeTran mFakeTran;
	ClientInfoBlock mClientInfo[MAX_CLIENT_NUM + 1];
	list<ClientReqBlock> mReqList;
	list<ClientReqBlock> mPreReqList;
	double mBlockSize;
	int mPerSendSize;
	list<ClientReqBlock>::iterator mCurReqBlock;

	int mFileNum;
	int mMinLength;
	int mMaxLength;
	double mMinBitRate;
	double mMaxBitRate;

	pthread_t mtid;
	pthread_t mRtid;

	int mLinkedNums;
	int mMaxBandSize;
	int mUsedBandSize;
	int mNeedSendBandSize;

	bool mNeverShow;
	ofstream mOFs;
	ofstream mBufferOFs;

	ofstream mClientDelayOfs[MAX_CLIENT_NUM + 1];

	bool mIsUseRealDevice;

	double mLrfuLambda;
	char *mBufferStrategy;
	map<unsigned int, bool> mConnectStatus;

	//add by mjq @2013.6.8
	int mHitTimes;//缓存命中次数
	int mTotalTimes;//总次数
	int mTotalRequest; // 客户端总请求数
	int mReadFromServer; // 没有找到P2P，直接从服务器读取的请求个数
	int mMissTimes; // 未命中次数
	//end by mjq @2013.6.8
};


#endif /* MYSERVER_H_ */
