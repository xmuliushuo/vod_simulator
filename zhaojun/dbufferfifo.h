/*
 * dbufferfifo.h
 *
 *  Created on: 2013-5-13
 *      Author: zhaojun
 */

#ifndef DBUFFERFIFO_H_
#define DBUFFERFIFO_H_

#include "globalfunction.h"
#include "dbuffer.h"

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <list>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

using namespace std;

struct FIFOBlock{
	FIFOBlock(int fileId,int segId){//,bool isLocked){
		this->fileId = fileId;
		this->segId = segId;
//		this->isLocked = isLocked;
	}
	int fileId;
	int segId;
//	bool isLocked;
//	int freq;
};

class DBufferFIFO : public DBuffer{
public:
	DBufferFIFO(int blockSize,int blockNum);
	virtual ~DBufferFIFO();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	virtual bool FindBlock(int fileId,int segId);
protected:

	list<FIFOBlock> m_blockList;

	int m_curBlockNum;
};


#endif /* DBUFFERFIFO_H_ */
