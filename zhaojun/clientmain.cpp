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

#include <stdio.h>
#include <stdlib.h>

using namespace std;

timeval globalStartTime;
double globalModify;

int main(){
	srand((unsigned int)time(NULL));
	gettimeofday(&globalStartTime,NULL);

//	MyServer *server;
	MyClientManage *clientManage;

	std::map<std::string,std::string> keyMap;

	double serverBand,clientBand;
	int blockSize,perSendSize;
	int fileNum;
	int maxLength,minLength;
	double bitRate;
	int serverFd;
	int blockNums;
	int thelta,lambda,zeta,sigma;
	int playToPlay,playToPause,playToForward,playToBackward,playToStop;
	int clientNums;
	int devNums;
	char *clusterAddress[MAX_DEV_NUM];
	int serverPort;
	int clientPort;
	int clusterNum;
	bool isStartTogether;
	char *bufferStrategy;
	int period;
	int lrfuLambda;
	bool isRepeat;
	bool special;

	serverBand = atof(keyMap["ServerBand"].c_str());
	clientBand = atof(keyMap["ClientBand"].c_str());
	perSendSize = atoi(keyMap["PerSendSize"].c_str());
	isP2POpen = !strcmp(keyMap["isP2POpen"].c_str(),"true") ? true : false;
	isRepeat = !strcmp(keyMap["isRepeat"].c_str(),"true") ? true : false;
	special = !strcmp(keyMap["Special"].c_str(),"true") ? true : false;
//	globalReadList = !strcmp(keyMap["ReadList"].c_str(),"true") ? true : false;

	fileNum = atoi(keyMap["SourceNums"].c_str());
	maxLength = atoi(keyMap["MaxLength"].c_str());
	minLength = atoi(keyMap["MinLength"].c_str());
	bitRate = atof(keyMap["BitRate"].c_str());
	thelta = atoi(keyMap["Thelta"].c_str());
	lambda = atoi(keyMap["Lambda"].c_str());
	zeta = atoi(keyMap["BackZeta"].c_str());
	sigma = atoi(keyMap["BackSigma"].c_str());
	playToPlay = atoi(keyMap["PlayToPlay"].c_str());
	playToPause = atoi(keyMap["PlayToPause"].c_str());
	playToForward = atoi(keyMap["PlayToForward"].c_str());
	playToBackward = atoi(keyMap["PlayToBackward"].c_str());
	playToStop = atoi(keyMap["PlayToStop"].c_str());
	devNums = atoi(keyMap["DevNums"].c_str());
	serverPort = atoi(keyMap["ServerPort"].c_str());
	clientPort = atoi(keyMap["ClientPort"].c_str());
	clusterNum = atoi(keyMap["ClusterNum"].c_str());
	isStartTogether = !strcmp(keyMap["IsStartTogether"].c_str(),"true") ? true : false;

	globalModify = atof(keyMap["Modify"].c_str());

	bufferStrategy = const_cast<char *>(keyMap["BufferStrategy"].c_str());

	period = atoi(keyMap["Period"].c_str());
	lrfuLambda = atoi(keyMap["LrfuLambda"].c_str());

	int multiple = atoi(keyMap["Multiple"].c_str());
	globalTimer.setMultiple(multiple);

	int preFetch = atoi(keyMap["PreFetch"].c_str());
	
	

	stringstream sstring;
	for(int i = 0;i < devNums;i++){
		sstring.str("");
		sstring << "ClusterAddress" << (i + 1);
		string keyName = sstring.str();
		clusterAddress[i] = const_cast<char *>(keyMap[keyName.c_str()].c_str());
	}

	clientManage = new MyClientManage(serverAddress,perSendSize,blockSize,blockNums,clientBand,fileNum,thelta,lambda,
			zeta,sigma,playToPlay,playToPause,playToForward,playToBackward,playToStop,clientNums,clusterAddress,
			serverPort,clientPort,devNums,clusterNum,isStartTogether,bufferStrategy,
			period,lrfuLambda,isRepeat,preFetch,special);
//
	cout << "create clients" << endl;
	clientManage->CreateClient();

	sleep(1);
	pthread_join(clientManage->GetTid(),NULL);

	keyMap.clear();
	delete clientManage;

	return 0;
}


