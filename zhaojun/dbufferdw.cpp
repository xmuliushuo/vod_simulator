#include "dbufferdw.h"

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
	assert(mDWQueue.size() <= mBlockNums);
	for(list<DWBlockInfo>::iterator it = mDWQueue.begin(); it != mDWQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			++(it->hitNew);
	/*		DWBlockInfo block = *it;
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
	assert(mDWQueue.size() <= mBlockNums);
	if (mDWQueue.size() == mBlockNums)
		Strategy(fileId, segId, ofileId, osegId);
	else
		AddBlock(fileId, segId);
}

void DBufferDW::Strategy(int fileId, int segId, int &ofileId, int &osegId)
{
	assert(mDWQueue.size() == mBlockNums);
	const int MAX = 99999999;
	int minWeight = MAX;
	list<DWBlockInfo>::iterator iter, minIter;

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
	for(list<DWBlockInfo>::iterator it = mDWQueue.begin(); it != mDWQueue.end(); ++it) {
		it->hitOld = it->hitNew;
		it->hitNew = 0;
	}
}

bool DBufferDW::FindBlock(int fileId,int segId)
{
	for(list<DWBlockInfo>::iterator it = mDWQueue.begin(); it != mDWQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			return true;
		}
	}
	return false;
}

void DBufferDW::AddBlock(int fileId, int segId)
{
	DWBlockInfo newBlock(fileId, segId);
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
