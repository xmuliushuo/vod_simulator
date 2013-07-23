/*
 * myclientmanage.cpp
 *
 *  Created on: 2013-2-23
 *      Author: zhaojun
 */

#include "myclientmanage.h"
#include "log.h"

MyClientManage::MyClientManage(char *serverAddress,int perSendSize,int blockSize,
		int blockNums,double bandWidth,int fileNum,double thelta,double lambda,
		double zeta,double sigma,int playToPlay,int playToPause,int playToForward,
		int playToBackward,int playToStop,int clientNums,char **clusterAddress,int serverPort,
		int clientPort,int devNums,int clusterNum,bool isStartTogether,char *bufferStrategy,
		int period,int lrfuLambda,bool isRepeat,int preFetch,bool special){
	mPreFetch = preFetch;

	mSpecial = special;
	mPerSendSize = perSendSize;
	mBlockNums = blockNums;
	mBand = bandWidth;
	mBlockSize = blockSize / 1000.0;

	mIsStartTogether = isStartTogether;

	mFileNum = fileNum;
	mThelta = thelta;
	mLambda = lambda;
	mZeta = zeta;
	mSigma = sigma;
	mPlayToPlay = playToPlay;
	mPlayToBackward = playToBackward;
	mPlayToForward = playToForward;
	mPlayToPause = playToPause;
	mPlayToStop = playToStop;
	mClientNums = clientNums;

	mClusterNum = clusterNum;
	mClusterAddress = clusterAddress;
	mServerPort = serverPort;
	mClientPort = clientPort;
	mDevNums = devNums;

	mServerAddress = serverAddress;

	mBufferStrategy = bufferStrategy;

	mClientVect.clear();

	mPeriod = period;
	mLrfuLambda = lrfuLambda / 1000.0;

	mIsRepeat = isRepeat;

	LOG_INFO("");
	LOG_INFO("Client Config");
	LOG_INFO("BandWidth = " << mBand);
	LOG_INFO("BlockSize = " << mBlockSize);
	LOG_INFO("BlockNums = " << mBlockNums);
	LOG_INFO("PerSendSize = " << mPerSendSize);
	LOG_INFO("FileNums = " << mFileNum);
	LOG_INFO("Thelta = " << mThelta);
	LOG_INFO("Lambda = " << mLambda);
	LOG_INFO("Zeta = " << mZeta);
	LOG_INFO("Sigma = " << mSigma);
	LOG_INFO("PlayToPlay = " << mPlayToPlay);
	LOG_INFO("PlayToBackward = " << mPlayToBackward);
	LOG_INFO("PlayToForward = " << mPlayToForward);
	LOG_INFO("PlayToPause = " << mPlayToPause);
	LOG_INFO("PlayToStop = " << mPlayToStop);
	LOG_INFO("ClientNums = " << mClientNums);
	LOG_INFO("ServerPort = " << mServerPort);
	LOG_INFO("ServerAddress = " << mServerAddress);
	LOG_INFO("ClientPort = " << mClientPort);
	LOG_INFO("DevNums = " << mDevNums);
	LOG_INFO("BufferStrategy = " << mBufferStrategy);
	LOG_INFO("Period = " << mPeriod);
	LOG_INFO("LrfuLambda = " << mLrfuLambda);
	LOG_INFO("IsRepeat = " << mIsRepeat);

	for(int i = 0;i < devNums;i++){
		LOG_INFO("Cluster " << i << " address is:" << mClusterAddress[i]);
	}

	mModel = new ModelAssemble(mFileNum,mThelta,mLambda,mZeta,mSigma,mClientNums,
			mPlayToPlay,mPlayToPause,mPlayToForward,mPlayToBackward,mPlayToStop,mIsStartTogether);
}

MyClientManage::~MyClientManage(){
	delete mModel;
	for(int i = 1;i < mClientVect.size();i++){
		delete mClientVect[i];
	}
	mClientVect.clear();
}

void MyClientManage::CreateClient(){
	mClientVect.push_back(NULL);
	int startClient = (mClientNums / mDevNums) * mClusterNum + 1;
	int endClient = startClient + (mClientNums / mDevNums);
	for(int i = startClient;i < endClient;i++){
		MyClient *client = new MyClient(mModel,mBlockSize,mPerSendSize,mBand,mBlockNums,i,mServerPort,
				mClientPort,mDevNums,mClientNums,mClusterAddress,mServerAddress,mBufferStrategy,
				mPeriod,mLrfuLambda,mIsRepeat,mPreFetch,mSpecial,mIsStartTogether);
		mClientVect.push_back(client);
		LOG_INFO("create thread " << i << " " << mClientVect[i - startClient + 1]->GetTid());
	}
}
