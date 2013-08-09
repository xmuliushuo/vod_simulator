/*
 * faketran.cpp
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */


#include "mymessage.h"

#include "log.h"

void FakeTran::TranData(double bandWidth,int blockSize,int toClientNum,int oper){
	//blockSize单位为KB
	double sleepTime = (blockSize / 1000.0) / (bandWidth / 8);
	TimerEvent event;
	event.isNew = true;
	event.sockfd = mSockFd[1];
	event.leftTime = sleepTime * 1000000;
	event.leftTime -= (1000 * (globalModify - 1));

	int *ptr = (int *)event.buffer;
	*ptr = MSG_FAKE_FIN;
	ptr++;
	*ptr = toClientNum;
	ptr++;
	*ptr = oper;

	globalTimer.RegisterTimer(event);
}

void FakeTran::Active(){
	char buffer[20];
	int *ptr = (int *)buffer;
	*ptr = MSG_FAKE_FIN;
	ptr++;
	*ptr = -1;
	ptr++;
	*ptr = -1;
	send(mSockFd[1],buffer,20,0);
}



