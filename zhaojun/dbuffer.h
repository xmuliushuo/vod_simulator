#ifndef __D_BUFFER_H__
#define __D_BUFFER_H__

#include "globalfunction.h"

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <list>
#include <time.h>
#include <stdlib.h>
#include <signal.h>

using namespace std;

//struct Block{
//	Block(int fileId,int segId){//,bool isLocked){
//		this->fileId = fileId;
//		this->segId = segId;
////		this->isLocked = isLocked;
//	}
//	int fileId;
//	int segId;
////	bool isLocked;
////	int freq;
//};

class BlockInfo {
public:
	BlockInfo();
	BlockInfo(int _fileId, int _segId): hitNew(1), hitOld(0),weight(0),fileId(_fileId), segId(_segId){};
	int hitNew;
	int hitOld;
	int weight;
	int fileId;
	int segId;
	double hitTime;
	unsigned int vtime;
};

class DBuffer{
public:
	DBuffer(int blockSize,int blockNum);
	virtual ~DBuffer();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
//	virtual bool FindAndAdjustBlock(int fileId,int segId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	bool IsBlockReset();
	virtual void BlockReset(){}
//	virtual void PrintBuffer();
//	void MutexLock();
//	void MutexUnLock();
//	void NotFullSignal();
//	void NotEmptySignal();
	virtual bool FindBlock(int fileId,int segId);
	virtual void AddVistors(int fileId, int clientID);
	virtual void DeleteVistors(int fileId, int clientID);
protected:
//	pthread_cond_t m_notEmptyCond;
//	pthread_cond_t m_notFullCond;
//	pthread_mutex_t m_mutex;
//	list<Block> m_blockList;
	int mBlockSize;//单位为KB
	int mBlockNums;
//	int m_curBlockNum;
	bool m_isblockReset;
};
#endif
