#include "dbuffer.h"

DBuffer::DBuffer(int blockSize,int blockNum){
//	m_blockSize = blockSize;
//	m_blockNum = blockNum;
////	pthread_cond_init(&m_notEmptyCond,NULL);
////	pthread_cond_init(&m_notFullCond,NULL);
////	pthread_mutex_init(&m_mutex,NULL);
//
//	m_blockList.clear();
//	m_curBlockNum = 0;
	m_isblockReset = false;
	mBlockSize = blockSize;
	mBlockNums = blockNum;
}

DBuffer::~DBuffer(){
//	pthread_cond_destroy(&m_notEmptyCond);
//	pthread_cond_destroy(&m_notFullCond);
//	pthread_mutex_destroy(&m_mutex);

//	m_blockList.clear();
}

//void DBuffer::MutexLock(){
//	pthread_mutex_lock(&m_mutex);
//}
//
//void DBuffer::MutexUnLock(){
//	pthread_mutex_unlock(&m_mutex);
//}
//
//void DBuffer::NotEmptySignal(){
//	pthread_cond_signal(&m_notEmptyCond);
//}
//
//void DBuffer::NotFullSignal(){
//	pthread_cond_signal(&m_notFullCond);
//}

bool DBuffer::IsBlockReset(){
	return m_isblockReset;
}

bool DBuffer::FindBlock(int fileId,int segId){//,bool locked){
////	pthread_mutex_lock(&m_mutex);
//	if(!m_blockList.empty()){
//		std::list<Block>::iterator listIter = m_blockList.begin();
////		listIter++;
//		while(listIter != m_blockList.end()){
//			if(listIter->fileId == fileId && listIter->segId == segId){
//				return true;
//			}
//			listIter++;
//		}
//	}
////	pthread_mutex_unlock(&m_mutex);
	return false;
}

//bool DBuffer::FindAndAdjustBlock(int fileId,int segId){
////	if(!m_blockList.empty()){
////		std::list<Block>::iterator listIter = m_blockList.begin();
////		while(listIter != m_blockList.end()){
////			if(listIter->fileId == fileId && listIter->segId == segId){
////				Block block = *listIter;
////				m_blockList.erase(listIter);
////				m_blockList.push_front(block);
////				return true;
////			}
////			listIter++;
////		}
////	}
//
//	return false;
//}

bool DBuffer::Read(int fileId,int segId){

////	pthread_mutex_lock(&m_mutex);
//	if(m_blockList.empty()){
////		pthread_mutex_unlock(&m_mutex);
//		return false;
//	}
//	if(!(m_blockList.front().fileId == fileId && m_blockList.front().segId == segId)){
////		pthread_cond_signal(&m_notFullCond);
//		if(!FindAndAdjustBlock(fileId,segId)){
////			NS_LOG_UNCOND("Not find fileId:" << fileId << ",segId" << segId);
////			pthread_mutex_unlock(&m_mutex);
//			return false;
////			pthread_cond_wait(&m_notEmptyCond,&m_mutex);
//		}
//	}
//
////	pthread_mutex_unlock(&m_mutex);
	return true;
}

void DBuffer::Write(int fileId,int segId,int &ofileId,int &osegId){

//	pthread_mutex_lock(&m_mutex);
//	ofileId = -1;
//	osegId = -1;
//	if(m_curBlockNum < m_blockNum){
//		m_blockList.push_front(Block(fileId,segId));
//		m_curBlockNum++;
//	}
//	else{
//		Strategy(fileId,segId,ofileId,osegId);
//	}
//	pthread_cond_signal(&m_notEmptyCond);
//	if(m_curBlockNum == m_blockNum)
//		pthread_cond_wait(&m_notFullCond,&m_mutex);
//	pthread_mutex_unlock(&m_mutex);
}

//LRU算法
void DBuffer::Strategy(int fileId,int segId,int &ofileId,int &osegId){
//	list<Block>::iterator listIter = m_blockList.end();
//	listIter--;
//	ofileId = listIter->fileId;
//	osegId = listIter->segId;
//	m_blockList.erase(listIter);
//	m_blockList.push_front(Block(fileId,segId));
}


void DBuffer::AddVistors(int fileId, int clientID)
{

}


void DBuffer::DeleteVistors(int fileId, int clientID)
{

}


#if 0
//测试用
static bool isRunning = true;

static int requestFileId = -1;
static int requestSegId = -1;
pthread_mutex_t requestMutex;

int RandomI(int i){
	return (rand() / (RAND_MAX * 1.0)) * i;
}

void *ThreadToWrite(void *arg){
	DBuffer *ptr = (DBuffer *)arg;
	int times = 0;
	while(isRunning){
		int fileId = RandomI(10);
		int segId = RandomI(1000);
		pthread_mutex_lock(&requestMutex);
		if(requestFileId != -1){
			fileId = requestFileId;
			segId = requestSegId;
			requestFileId = -1;
			requestSegId = -1;
		}
		pthread_mutex_unlock(&requestMutex);
		ptr->Write(NULL,0,fileId,segId);
		std::cout << "WRITE:" << times << std::endl;
		times++;
		ptr->PrintBuffer();
	}
	ptr->NotEmptySignal();
	std::cout << "Write Finish" << std::endl;
	return 0;
}

void *ThreadToRead(void *arg){
	DBuffer *ptr = (DBuffer *)arg;
	int times = 0;
	while(isRunning){
		int fileId = RandomI(10);
		int segId = RandomI(1000);
		ptr->MutexLock();
		if(!(ptr->FindAndAdjustBlock(fileId,segId))){
			pthread_mutex_lock(&requestMutex);
			requestFileId = fileId;
			requestSegId = segId;
			pthread_mutex_unlock(&requestMutex);
		}
		ptr->MutexUnLock();
		ptr->Read(NULL,0,fileId,segId);
		std::cout << "READ:" << times << std::endl;
		times++;
		ptr->PrintBuffer();
	}
	ptr->NotFullSignal();
	std::cout << "Read Finish" << std::endl;
	return 0;
}

void IntHandler(int sig){
	isRunning = false;
}

int main(int argc,char *argv[]){
	DBuffer dbuffer(20,10);
	pthread_t tidRead,tidWrite;

	signal(SIGINT,IntHandler);

	pthread_mutex_init(&requestMutex,NULL);

	pthread_create(&tidRead,NULL,ThreadToRead,&dbuffer);
	pthread_create(&tidWrite,NULL,ThreadToWrite,&dbuffer);

	pthread_join(tidRead,NULL);
	pthread_join(tidWrite,NULL);

	pthread_mutex_destroy(&requestMutex);
	std::cout << "Finish" << std::endl;

	return 0;
}
#endif


