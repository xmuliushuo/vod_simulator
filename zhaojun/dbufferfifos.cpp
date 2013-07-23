#include "dbufferfifos.h"

DBufferFIFOS::DBufferFIFOS(int blockSize, int blockNum): DBuffer(blockSize, blockNum)
{
	// dws need refresh time window to execute BlockReset()
	// so set true
	m_isblockReset = false;
	mFIFOQueue.clear();
}

DBufferFIFOS::~DBufferFIFOS()
{
	mFIFOQueue.clear();
}

bool DBufferFIFOS::Read(int fileId, int segId)
{
	assert(mFIFOQueue.size() <= mBlockNums);
	for(list<FIFOSBlockInfo>::iterator it = mFIFOQueue.begin(); it != mFIFOQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			return true;
		}
	}
	return false;
}

void DBufferFIFOS::Write(int fileId, int segId, int &ofileId, int &osegId)
{
	ofileId = -1;
	osegId = -1;
	assert(mFIFOQueue.size() <= mBlockNums);
	if (mFIFOQueue.size() == mBlockNums)
		Strategy(fileId, segId, ofileId, osegId);
	else
		AddBlock(fileId, segId);
}

void DBufferFIFOS::Strategy(int fileId, int segId, int &ofileId, int &osegId)
{
	assert(mFIFOQueue.size() == mBlockNums);
	const int MAX = 99999999;
	int minWeight = MAX;
	list<FIFOSBlockInfo>::iterator iter, minIter;

	for(iter = mFIFOQueue.begin(); iter != mFIFOQueue.end(); ++iter){
		iter->weight = GetVistorNum(iter->fileId);
		if (iter->weight < minWeight) {
			minWeight = iter->weight;
			minIter = iter;
		}
	}
	ofileId = minIter->fileId;
	osegId = minIter->segId;
	mFIFOQueue.erase(minIter);
	AddBlock(fileId, segId);
}

/*
 * time window end, block reset
 */
void DBufferFIFOS::BlockReset()
{
}

bool DBufferFIFOS::FindBlock(int fileId,int segId)
{
	for(list<FIFOSBlockInfo>::iterator it = mFIFOQueue.begin(); it != mFIFOQueue.end(); ++it) {
		if (it->fileId == fileId && it->segId == segId) {
			return true;
		}
	}
	return false;
}

void DBufferFIFOS::AddBlock(int fileId, int segId)
{
	FIFOSBlockInfo newBlock(fileId, segId);
	mFIFOQueue.push_back(newBlock);
}

void DBufferFIFOS::AddVistors(int fileId, int clientID)
{
	mFileVistors[fileId].insert(clientID);
}

void DBufferFIFOS::DeleteVistors(int fileId, int clientID)
{
	mFileVistors[fileId].erase(clientID);
}

int DBufferFIFOS::GetVistorNum(int fileId)
{
	return mFileVistors[fileId].size();
}