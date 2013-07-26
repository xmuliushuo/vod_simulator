/*
 * main.cpp
 *
 *  Created on: 2013-2-24
 *      Author: zhaojun
 */

#include "myserver.h"
#include "myclientmanage.h"
#include "globalfunction.h"
#include "mytimer.h"
#include <sys/time.h>

#include <string.h>

#include <sstream>

#include "log.h"

MyTimer globalTimer(1);

timeval globalStartTime;
double globalModify;

//MyServer(double bandWidth,int blockSize,int perSendSize,bool isP2POpen,
//		int fileNum,int maxLength,int minLength,double bitRate)

//MyClientManage(int serverFd,int perSendSize,int blockSize,
//			int blockNums,double bandWidth,int fileNum,double thelta,double lambda,
//			double zeta,double sigma,int playToPlay,int playToPause,int playToForward,
//			int playToBackward,int playToStop);

int main(){
	srand((unsigned int)time(NULL));
	gettimeofday(&globalStartTime,NULL);

	MyServer *server;
//	MyClientManage *clientManage;

	double serverBand,clientBand;
	int blockSize,perSendSize;
	bool isP2POpen;
	int fileNum;
	int maxLength,minLength;
	double minBitRate,maxBitRate;
	int serverFd;
	int blockNums;
	int thelta,lambda,zeta,sigma;
	int playToPlay,playToPause,playToForward,playToBackward,playToStop;
	int clientNums;
	int devNums;
	char *clusterAddress[MAX_DEV_NUM];
	int serverPort;
	int clientPort;
	int sampleFre;

	char *bufferStrategy;
	int period;
	int lrfulambda;

	bool isUseRealDevice;

	serverBand = atof(keyMap["ServerBand"].c_str());
	clientBand = atof(keyMap["ClientBand"].c_str());
	blockSize = atoi(keyMap["BlockSize"].c_str());
	perSendSize = atoi(keyMap["PerSendSize"].c_str());
	fileNum = atoi(keyMap["SourceNums"].c_str());
	maxLength = atoi(keyMap["MaxLength"].c_str());
	minLength = atoi(keyMap["MinLength"].c_str());
	minBitRate = atof(keyMap["MinBitRate"].c_str());
	maxBitRate = atof(keyMap["MaxBitRate"].c_str());
	blockNums = atoi(keyMap["SBlockNums"].c_str());

	period = atoi(keyMap["Period"].c_str());
	lrfulambda = atoi(keyMap["LrfuLambda"].c_str());

	thelta = atoi(keyMap["Thelta"].c_str());
	lambda = atoi(keyMap["Lambda"].c_str());
	zeta = atoi(keyMap["Zeta"].c_str());
	sigma = atoi(keyMap["Sigma"].c_str());
	playToPlay = atoi(keyMap["PlayToPlay"].c_str());
	playToPause = atoi(keyMap["PlayToPause"].c_str());
	playToForward = atoi(keyMap["PlayToForward"].c_str());
	playToBackward = atoi(keyMap["PlayToBackward"].c_str());
	playToStop = atoi(keyMap["PlayToStop"].c_str());
	clientNums = atoi(keyMap["ClientNums"].c_str());
	devNums = atoi(keyMap["DevNums"].c_str());
	serverPort = atoi(keyMap["ServerPort"].c_str());
	clientPort = atoi(keyMap["ClientPort"].c_str());
	sampleFre = atoi(keyMap["SampleFrequency"].c_str());
	globalModify = atof(keyMap["Modify"].c_str());

	isUseRealDevice = !strcmp(keyMap["IsUseRealDevice"].c_str(),"true") ? true : false;

	int multiple = atoi(keyMap["Multiple"].c_str());
	globalTimer.setMultiple(multiple);

	stringstream sstring;
	for(int i = 0;i < devNums;i++){
		sstring.str("");
		sstring << "ClusterAddress" << (i + 1);
		string keyName = sstring.str();
		clusterAddress[i] = const_cast<char *>(keyMap[keyName.c_str()].c_str());
	}

	server = new MyServer(serverBand,blockSize,perSendSize,isP2POpen,fileNum,maxLength,minLength,
			minBitRate,maxBitRate,serverPort,clientPort,devNums,clientNums,clusterAddress,sampleFre,isUseRealDevice,
			period,lrfulambda,bufferStrategy,blockNums);

//	clientManage = new MyClientManage(serverFd,perSendSize,blockSize,blockNums,clientBand,fileNum,thelta,lambda,
//			zeta,sigma,playToPlay,playToPause,playToForward,playToBackward,playToStop,clientNums);
//
//	cout << "create clients" << endl;
//	clientManage->CreateClient();

//	sleep(10);

	pthread_join(server->GetTid(),NULL);

	keyMap.clear();
	delete server;
//	delete clientManage;

//	exit(0);

	return 0;
}


