#include "dbufferdw.h"

#include <cassert>

DBufferDW::DBufferDW(int blockSize, int blockNum, int period): DBuffer(blockSize,blockNum)
{
	
	// dws need refresh time window to execute BlockReset()
	// so set true
	m_isblockReset = true;
	mDWQueue.clear();
}

DBufferDW::~DBufferDW()
{
	mDWQueue.clear();
}

bool DBufferDW::Read(int fileId, int segId)
{
	for(list<Block>::iterator it = mDWQueue.begin(); it != mDWQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			++(it->hitNew);
	/*		Block block = *it;
			mDWQueue.erase(it);
			mDWQueue.push_back(block);*/
			return true;
		}
	}
	return false;
}

void DBufferDW::Write(int fileId, int segId, int &ofileId, int &osegId)
{
	ofileId = -1;
	osegId = -1;
	assert(mBlockNums >= 0);
	if (mDWQueue.size() == static_cast<unsigned int>(mBlockNums))
		Strategy(fileId, segId, ofileId, osegId);
	else
		AddBlock(fileId, segId);
}

void DBufferDW::Strategy(int fileId, int segId, int &ofileId, int &osegId)
{
	const int MAX = 99999999;
	int minWeight = MAX;
	list<Block>::iterator iter, minIter;

	for(iter = mDWQueue.begin(); iter != mDWQueue.end(); ++iter){
		iter->weight = (iter->hitNew + iter->hitOld);
		if (iter->weight < minWeight) {
			minWeight = iter->weight;
			minIter = iter;
		}
	}
	ofileId = minIter->fileId;
	osegId = minIter->segId;
	mDWQueue.erase(minIter);
	AddBlock(fileId,segId);
}

/*
 * time window end, block reset
 */
void DBufferDW::BlockReset()
{
	for(list<Block>::iterator it = mDWQueue.begin(); it != mDWQueue.end(); ++it) {
		it->hitOld = it->hitNew;
		it->hitNew = 0;
	}
}

bool DBufferDW::FindBlock(int fileId,int segId)
{
	for(list<Block>::iterator it = mDWQueue.begin(); it != mDWQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			return true;
		}
	}
	return false;
}

void DBufferDW::AddBlock(int fileId, int segId)
{
	Block newBlock(fileId, segId);
	mDWQueue.push_back(newBlock);
}

void DBufferDW::AddVistors(int fileId, int clientID)
{
	mFileVistors[fileId].insert(clientID);
}

void DBufferDW::DeleteVistors(int fileId, int clientID)
{
	mFileVistors[fileId].erase(clientID);
}

int DBufferDW::GetVistorNum(int fileId)
{
	return mFileVistors[fileId].size();
}
