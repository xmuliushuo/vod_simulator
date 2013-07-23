/*
 * mymanageclient.h
 *
 *  Created on: 2013-2-23
 *      Author: zhaojun
 */

#ifndef MYMANAGECLIENT_H_
#define MYMANAGECLIENT_H_

#include "myclient.h"
#include "modelassemble.h"

#include <vector>

using namespace std;

class MyClientManage{
public:
	MyClientManage(char *serverAddress,int perSendSize,int blockSize,
			int blockNums,double bandWidth,int fileNum,double thelta,double lambda,
			double zeta,double sigma,int playToPlay,int playToPause,int playToForward,
			int playToBackward,int playToStop,int clientNums,char **clusterAddress,int serverPort,
			int clientPort,int devNums,int clusterNum,bool isStartTogether,char *bufferStrategy,
			int period,int lrfuLambda,bool isRepeat,int preFetch,bool special);
	~MyClientManage();
	void CreateClient();

	double GetBand() const {
		return mBand;
	}

	void SetBand(double band) {
		mBand = band;
	}

	int GetBlockNum() const {
		return mBlockNums;
	}

	void SetBlockNum(int blockNums) {
		mBlockNums = blockNums;
	}

	int GetBlockSize() const {
		return mBlockSize;
	}

	void SetBlockSize(int blockSize) {
		mBlockSize = blockSize;
	}

	int GetClientNums() const {
		return mClientNums;
	}

	void SetClientNums(int clientNums) {
		mClientNums = clientNums;
	}

	vector<MyClient*> GetClientVect() const {
		return mClientVect;
	}

	void SetClientVect(vector<MyClient*> clientVect) {
		mClientVect = clientVect;
	}

	int GetFileNum() const {
		return mFileNum;
	}

	void SetFileNum(int fileNum) {
		mFileNum = fileNum;
	}

	double GetLambda() const {
		return mLambda;
	}

	void SetLambda(double lambda) {
		mLambda = lambda;
	}

	ModelAssemble* GetModel() const {
		return mModel;
	}

	void SetModel(ModelAssemble* model) {
		mModel = model;
	}

	int GetPerSendSize() const {
		return mPerSendSize;
	}

	void SetPerSendSize(int perSendSize) {
		mPerSendSize = perSendSize;
	}

	int GetPlayToBackward() const {
		return mPlayToBackward;
	}

	void SetPlayToBackward(int playToBackward) {
		mPlayToBackward = playToBackward;
	}

	int GetPlayToForward() const {
		return mPlayToForward;
	}

	void SetPlayToForward(int playToForward) {
		mPlayToForward = playToForward;
	}

	int GetPlayToPause() const {
		return mPlayToPause;
	}

	void SetPlayToPause(int playToPause) {
		mPlayToPause = playToPause;
	}

	int GetPlayToPlay() const {
		return mPlayToPlay;
	}

	void SetPlayToPlay(int playToPlay) {
		mPlayToPlay = playToPlay;
	}

	int GetPlayToStop() const {
		return mPlayToStop;
	}

	void SetPlayToStop(int playToStop) {
		mPlayToStop = playToStop;
	}

	double GetSigma() const {
		return mSigma;
	}

	void SetSigma(double sigma) {
		mSigma = sigma;
	}

	double GetThelta() const {
		return mThelta;
	}

	void SetThelta(double thelta) {
		mThelta = thelta;
	}

	double GetZeta() const {
		return mZeta;
	}

	void SetZeta(double zeta) {
		mZeta = zeta;
	}

	pthread_t GetTid(){return mClientVect[1]->GetTid();}
private:
	int mClientNums;
	vector<MyClient *> mClientVect;
	ModelAssemble *mModel;

	int mPerSendSize;
	double mBlockSize;
	int mBlockNums;
	double mBand;

	int mFileNum;
	double mThelta;
	double mLambda;
	double mZeta;
	double mSigma;
	int mPlayToPlay;
	int mPlayToPause;
	int mPlayToForward;
	int mPlayToBackward;
	int mPlayToStop;

	int mPreFetch;

	char **mClusterAddress;
	int mServerPort;
	int mClientPort;
	int mDevNums;
	int mClusterNum;
	char *mServerAddress;

	bool mIsStartTogether;
	char *mBufferStrategy;
	int mPeriod;
	double mLrfuLambda;
	bool mIsRepeat;
	bool mSpecial;
};


#endif /* MYMANAGECLIENT_H_ */
