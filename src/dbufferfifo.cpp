#include "dbufferfifo.h"

DBufferFIFO::DBufferFIFO(int blockSize,int blockNum)
	: DBuffer(blockSize,blockNum)
{
	m_blockList.clear();
	m_curBlockNum = 0;
}

DBufferFIFO::~DBufferFIFO(){
	m_blockList.clear();
}

bool DBufferFIFO::FindBlock(int fileId,int segId){//,bool locked){
	if(!m_blockList.empty()){
		std::list<Block>::iterator listIter = m_blockList.begin();
		while(listIter != m_blockList.end()){
			if(listIter->fileId == fileId && listIter->segId == segId){
				return true;
			}
			listIter++;
		}
	}
	return false;
}

bool DBufferFIFO::Read(int fileId,int segId){
	return FindBlock(fileId,segId);
}

void DBufferFIFO::Write(int fileId,int segId,int &ofileId,int &osegId){
	ofileId = -1;
	osegId = -1;
	if(m_curBlockNum < mBlockNums){
		m_blockList.push_front(Block(fileId,segId));
		m_curBlockNum++;
	}
	else{
		Strategy(fileId,segId,ofileId,osegId);
	}
}


void DBufferFIFO::Strategy(int fileId,int segId,int &ofileId,int &osegId){
	list<Block>::iterator listIter = m_blockList.end();
	listIter--;
	ofileId = listIter->fileId;
	osegId = listIter->segId;
	m_blockList.erase(listIter);
	m_blockList.push_front(Block(fileId,segId));
}
