/*
 * faketran.h
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */

#ifndef FAKETRAN_H_
#define FAKETRAN_H_

#include "mytimer.h"
#include <sys/socket.h>

extern MyTimer globalTimer;
extern double globalModify;

class FakeTran{
public:
	FakeTran();
	~FakeTran();
	int GetFd(){return mSockFd[0];}
	int GetOtherFd(){return mSockFd[1];}
	void TranData(double bandWidth,int blockSize,int toClientNum,int oper);
	void Active();
private:
	int mSockFd[2];
};


#endif /* FAKETRAN_H_ */
