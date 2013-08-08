/*
 * myserver.cpp
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */

#include "myserver.h"
#include "log.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <stdlib.h>

#include <sstream>

void *ThreadToServer(void *arg){
	MyServer *server = (MyServer *)arg;
	server->Run();
	return NULL;
}

void *ThreadToSearch(void *arg){
	MyServer *server = (MyServer *)arg;
	server->ReadFromDevice();
	return NULL;
}

MyServer::MyServer(double bandWidth,int blockSize,int perSendSize,bool isP2POpen,
		int fileNum,int maxLength,int minLength,double minbitRate,double maxBitRate,
		int serverPort,int clientPort,int devNums,int clientNums,
		char **clusterAddress,int takeSampleFre,bool isUseRealDevice,int period,double lrfuLambda,
		char *bufferStrategy,int blockNums){
	mBand = bandWidth;
	mPerSendSize = perSendSize;

	mLrfuLambda = lrfuLambda;

	mMaxBandSize = (mBand / 8.0) * 1000 * 4;
	mUsedBandSize = 0;
	mNeedSendBandSize = 0;
	mLinkedNums = 0;

	mNeverShow = false;

	mIsUseRealDevice = isUseRealDevice;

	mCurReqBlock = mReqList.end();
	mReqList.clear();
	mPreReqList.clear();

	mFileNum = fileNum;
	mMinLength = minLength;
	mMaxLength = maxLength;
	mMinBitRate = minbitRate;
	mMaxBitRate = maxBitRate;
	mDataServer = new DataServer(mFileNum,mMinLength,mMaxLength,blockSize,mMinBitRate,mMaxBitRate);

	mTakeSampleFre = takeSampleFre;

	mDevNums = devNums;
	mClientNums = clientNums;
	mServerPort = serverPort;
	mClientPort = clientPort;

	//add by mjq@2013.6.8
	mHitTimes = 0;
	mTotalTimes = 0;// not used
	mTotalRequest = 0;
	mReadFromServer = 0;
	//end by mjq

	for(int i = 0;i < devNums;i++){
		mClusterAddress[i] = clusterAddress[i];
	}

	for(int i = 0;i <= MAX_CLIENT_NUM;i++){
//		mClientInfo[i].listenFd = -1;
		mClientInfo[i].recvFd = -1;
//		mClientLinks[i] = 0;
	}

	mBufferStrategy = bufferStrategy;

	LOG_INFO("");
	LOG_INFO("Buffer strategy = "<<mBufferStrategy);
	LOG_INFO("Period = "<<mPeriod);
	LOG_INFO("LrfuLambda ="<< mLrfuLambda);

	for(int i = 0;i < devNums;i++){
		LOG_INFO("Cluster " << i << " address is:" << mClusterAddress[i]);
	}

	Init();

	mOFs.open("data/result.log",ios::out);
	mBufferOFs.open("data/buffer.log", ios::out);

	pthread_create(&mtid,NULL,ThreadToServer,this);
	pthread_create(&mRtid,NULL,ThreadToSearch,this);
}


MyServer::~MyServer(){
	delete mDataServer;
	mCurReqBlock = mReqList.end();
	for(int i = 0;i <= MAX_CLIENT_NUM;i++){
		if(mClientInfo[i].recvFd != -1){
			close(mClientInfo[i].recvFd);
		}
	}
	mReqList.clear();
	mPreReqList.clear();

	close(mNetSockFd);
	close(mEpollFd);

	mOFs.close();

	for(int i = 1;i < mClientNums;i++){
		mClientDelayOfs[i].close();
	}
}

void MyServer::Init(){
	mREpollFd = epoll_create(MAX_LISTEN_NUM);

	mNetSockFd = socket(AF_INET,SOCK_STREAM,0);

	socketpair(AF_UNIX,SOCK_STREAM,0,mTakeSample);

	socketpair(AF_UNIX,SOCK_STREAM,0,mReadDevice);
	ev.data.fd = mReadDevice[0];
	ev.events = EPOLLIN;
	epoll_ctl(mREpollFd,EPOLL_CTL_ADD,mReadDevice[0],&ev);

	ev.data.fd = mReadDevice[1];
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mReadDevice[1],&ev);

	ev.data.fd = mTakeSample[0];
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mTakeSample[0],&ev);

	ev.data.fd = mNetSockFd;
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mNetSockFd,&ev);

	ev.data.fd = mFakeTran.GetFd();
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mFakeTran.GetFd(),&ev);

	struct sockaddr_in serverAddr;
	bzero(&serverAddr,sizeof(serverAddr));
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(mServerPort);

	int val = 1;
	setsockopt(mNetSockFd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

	if(bind(mNetSockFd,(struct sockaddr *)&serverAddr,sizeof(struct sockaddr)) == -1){
		LOG_INFO("Server Bind Error!");
		exit(1);
	}

	if(listen(mNetSockFd,20) == -1){
		LOG_INFO("Server Listen Error!");
		exit(1);
	}

	LOG_INFO("before");
	stringstream sstream;
	for(int i = 1;i <= mClientNums;i++){
		sstream.str("");
		sstream << "data/clientdelay" << i << ".log";
		string fileName = sstream.str();
		mClientDelayOfs[i].open(fileName.c_str(),ios::out);
	}
	LOG_INFO("after");
}

void MyServer::ReadFromDevice(){
	while(true){
		for(int i = 0;i < nfds;i++){
			if(events[i].data.fd == mReadDevice[0]){
				char buffer[20];
				read(events[i].data.fd,buffer,20);

				int *ptr = (int *)buffer;
				int type = *ptr;
				ptr++;
				switch(type){
				case MSG_SEARCH_DEVICE:{
					int clientNum = *ptr;
					ptr++;
					int fileId = *ptr;
					ptr++;
					int segId = *ptr;
					double bitRate;
					int segNum;

					bool isOut = false;

					if(!mIsUseRealDevice){
						mDataServer->GetFileInfo(fileId,&bitRate,&segNum);
					}
					else{
						//add by mjq @2013.6.8
						mDataServer->GetFileInfo(fileId,&bitRate,&segNum);
						// mTotalTimes++;
						bool isRead = mDbuffer->Read(fileId,segId);
						int ofileId = -1,osegId = -1;
						if(!isRead){
							++mMissTimes;
							mDbuffer->Write(fileId,segId,ofileId,osegId);
							LOG_INFO("");
							LOG_INFO("server DELETE " << ofileId << "," << osegId << " and ADD " << fileId << "," << segId);
						}
						else
						{
							//命中次数自增
							mHitTimes++;
						}
						//cout to server.log,then sed
						// cout << "BufferHitStat:" <<mTotalTimes << ":" << mHitTimes << endl;
						//end by mjq @2013.6.8
					}

					if(segId > segNum){
						isOut = true;
					}

					ptr = (int *)buffer;
					*ptr = MSG_SEARCH_DEVICE;
					ptr++;
					*ptr = clientNum;
					ptr++;
					bool *boolPtr = (bool *)ptr;
					*boolPtr = isOut;
					boolPtr++;
					char *chPtr = (char *)boolPtr;
					*chPtr = (segNum >> 16) & 0xFF;
					chPtr++;
					*chPtr = (segNum >> 8) & 0xFF;
					chPtr++;
					*chPtr = segNum & 0xFF;
					chPtr++;
					double *dbPtr = (double *)chPtr;
					*dbPtr = bitRate;

					send(mReadDevice[0],buffer,20,0);

					break;
				}
				}
			}
		}
	}
}

void MyServer::TakeSample(){
	LOG_DISK(mOFs,mNeedSendBandSize);
	TimerEvent event;
	event.isNew = true;
	event.sockfd = mTakeSample[1];
	event.leftTime = mTakeSampleFre * 1000000;
	globalTimer.RegisterTimer(event);
}

void MyServer::Run(){
	TimerEvent timerEvent;
	timerEvent.isNew = true;
	timerEvent.sockfd = mTakeSample[1];
	timerEvent.leftTime = mTakeSampleFre * 1000000;
	globalTimer.RegisterTimer(timerEvent);

	epoll_event events[MAX_LISTEN_NUM];

	while(true){
		int fds = epoll_wait(mEpollFd,events,MAX_LISTEN_NUM,-1);
		for(int i = 0;i < fds;i++){
			if(events[i].events == EPOLLIN){
				if(events[i].data.fd == mFakeTran.GetFd()){
					char buffer[20];
					read(mFakeTran.GetFd(),buffer,20);
					DealWithMessage(buffer,20);
				}
				else if(events[i].data.fd == mTakeSample[0]){
					char buffer[20];
					read(mTakeSample[0],buffer,20);
					TakeSample();
				}
				else if(events[i].data.fd == mNetSockFd){
					struct sockaddr_in clientAddr;
					unsigned int length;
					bzero(&clientAddr,sizeof(struct sockaddr_in));
					length = sizeof(clientAddr);
					int clientFd = accept(mNetSockFd,(struct sockaddr *)(&clientAddr),&length);
					epoll_event ev;
					bzero(&ev,sizeof(ev));
					ev.data.fd = clientFd;
					ev.events = EPOLLIN;

					char buffer[20];
					recv(clientFd,buffer,20,0);
					int *ptr = (int *)buffer;
					int clientNum = *ptr;

					int port = mClientPort + ((clientNum - 1) % (mClientNums / mDevNums));

					mClientInfo[clientNum].address.sin_addr = clientAddr.sin_addr;
					mClientInfo[clientNum].address.sin_port = htons(port);
					mClientInfo[clientNum].recvFd = clientFd;

					epoll_ctl(mEpollFd,EPOLL_CTL_ADD,clientFd,&ev);
					//				char buffer[20];

					mLinkedNums++;

					LOG_INFO("");
					LOG_INFO("server response MSG_CONNECT_FIN to " << clientNum << " fd is:" << clientFd);

					ptr = (int *)buffer;
					*ptr = MSG_CONNECT_FIN;
					ptr++;
					*ptr = 0;
					ptr++;
					*ptr = 19;
					send(clientFd,buffer,20,0);
					//				}
				}
				else{
					char buffer[20];
					int length = recv(events[i].data.fd,buffer,20,0);
//					LOG_INFO("server recv message length:" << length);
					DealWithMessage(buffer,length);
				}
			}
		}
	}
}

list<ClientReqBlock>::iterator MyServer::SearchTheReqList(int fileId,int segId){
	list<ClientReqBlock>::iterator reqIter = mReqList.begin();
	while(reqIter != mReqList.end()){
		if(reqIter->fileId == fileId && reqIter->segId == segId)
			return reqIter;
		reqIter++;
	}
	return reqIter;
}

void MyServer::DealWithMessage(char *buf,int length){
	int *ptr = (int *)buf;
	int type = *ptr;
	ptr++;
	switch(type){
	case MSG_CLIENT_LEAVE:{ //客户端退出消息
		int clientNum = *ptr;
		epoll_event ev;
		ev.data.fd = mClientInfo[clientNum].recvFd;
		ev.events = EPOLLIN;
		epoll_ctl(mEpollFd,EPOLL_CTL_DEL,mClientInfo[clientNum].recvFd,&ev);
		close(mClientInfo[clientNum].recvFd);
		mClientInfo[clientNum].recvFd = -1;
//		mClientInfo[clientNum].listenFd = -1;
		LOG_INFO("");
		LOG_INFO("server receive MSG_CLIENT_LEAVE from " << clientNum <<
				" and remove the fd from epoll");
		break;
	}
	case MSG_SEARCH_DEVICE:{
		int clientNum = *ptr;
		ptr++;
		bool *boolPtr = (bool *)ptr;
		bool isOut = *boolPtr;
		boolPtr++;

		char chValue = *((char *)boolPtr);
		int segNum = (0x00FF0000 & (chValue << 16));
		boolPtr++;
		chValue = *((char *)boolPtr);
		segNum = (segNum | ((chValue << 8) & 0x0000FF00));
		boolPtr++;
		chValue = *((char *)boolPtr);
		boolPtr++;
		segNum = (segNum | (chValue & 0xFF));

		double *dbPtr = (double *)boolPtr;
		double bitRate = *dbPtr;

		if(isOut){
			char buffer[20];
			int *tmpPtr = (int *)buffer;
			*tmpPtr = MSG_BAD_REQ;
			tmpPtr++;
			*tmpPtr = 0;
			send(mClientInfo[clientNum].recvFd,buffer,20,0);
			LOG_INFO("");
			LOG_INFO("server receive MSG_SEG_ASK from " << clientNum <<
					"and response MSG_BAD_REQ");
			break;
		}

		//否则提供
		char buffer[20];
		int *tmpPtr = (int *)buffer;
		*tmpPtr = MSG_SEG_ACK;
		tmpPtr++;
		*tmpPtr = 0;
		tmpPtr++;
		*tmpPtr = segNum;
		tmpPtr++;
		dbPtr = (double *)tmpPtr;
		*dbPtr = bitRate;
		send(mClientInfo[clientNum].recvFd,buffer,20,0);
		LOG_INFO("");
		LOG_INFO("server receive MSG_SEARCH_DEVICE from " << clientNum <<
				" and response MSG_SEG_ACK segNum:" << segNum);

		break;
	}
	case MSG_SEG_ASK:{ // 客户端文件块查询消息
		int clientNum = *ptr;
		ptr++;
		int fileId = *ptr;
		ptr++;
		int segId = *ptr;
		double bitRate;
		int segNum;

		// 客户端请求文件块，此时总请求数加1
		++mTotalRequest;

		if(mIsP2POpen){
			int bestClient = mDataServer->SearchBestClient(fileId,segId);
			LOG_INFO("server find best client " << bestClient);
			if(bestClient != -1){
				char buffer[20];
				int *tmpPtr = (int *)buffer;
				*tmpPtr = MSG_REDIRECT;
				tmpPtr++;
				*tmpPtr = 0;
				tmpPtr++;
				*tmpPtr = bestClient;
				tmpPtr++;
				*tmpPtr = mClientInfo[bestClient].address.sin_addr.s_addr;
				tmpPtr++;
				*tmpPtr = mClientInfo[bestClient].address.sin_port;
				// if(strcmp(mBufferStrategy,"DWS") == 0) { // 如果使用DWS，还需要统计文件访问者信息
				LOG_INFO("");
				LOG_INFO("p2p is open, buffer DeleteVistors");
				mDbuffer->DeleteVistors(fileId, clientNum);
				// }
				send(mClientInfo[clientNum].recvFd,buffer,20,0);
				LOG_INFO("");
				LOG_INFO("server receive MSG_SEG_ASK from " << clientNum <<
						" and response MSG_REDIRECT to " << bestClient);
				break;
			}
		}

		// 此时表示没有找到合适的P2P服务客户端，因此从服务器直接读取文件块
		++mReadFromServer;
		mBufferOFs << clientNum << " " << fileId << " " << segId << " " << " " 
			<< mTotalRequest << " " << mReadFromServer<< " " << mHitTimes 
			<< " " << mMissTimes << endl;
		// if(strcmp(mBufferStrategy,"DWS") == 0) { // 如果使用DWS，还需要统计文件访问者信息
		LOG_INFO("");
		LOG_INFO("no bestClient, buffer AddVistors");
		mDbuffer->AddVistors(fileId, clientNum);
		// }

		char buffer[20];
		int *tmpPtr = (int *)buffer;
		*tmpPtr = MSG_SEARCH_DEVICE;
		tmpPtr++;
		*tmpPtr = clientNum;
		tmpPtr++;
		*tmpPtr = fileId;
		tmpPtr++;
		*tmpPtr = segId;
		send(mReadDevice[1],buffer,20,0);
		LOG_INFO("");
		LOG_INFO("server receive MSG_SEG_ASK from " << clientNum <<
				" and start to search the device");
		break;
	}
	case MSG_REQUEST_SEG:{
		int clientNum = *ptr;
		ptr++;
		int fileId = *ptr;
		ptr++;
		int segId = *ptr;

		ClientReqBlock reqBlock;
		reqBlock.clientNum = clientNum;
		reqBlock.fileId = fileId;
		reqBlock.segId = segId;
		reqBlock.oper = OPER_WRITE;
		reqBlock.leftSize = mBlockSize * 1000;
		reqBlock.preOper = OPER_WRITE;
		reqBlock.localfin = NONE_FIN;

		mNeedSendBandSize += reqBlock.leftSize;

		mPreReqList.push_back(reqBlock);

		if(mUsedBandSize < mMaxBandSize){
			ClientReqBlock tmpBlock = mPreReqList.front();
			mPreReqList.pop_front();
			mReqList.push_back(tmpBlock);
			mUsedBandSize += mBlockSize * 1000;
		}

		if(mUsedBandSize >= mMaxBandSize && !mNeverShow){
			LOG_INFO("server reach its max serve ability,the num is " << mLinkedNums);
			mNeverShow = true;
		}

		LOG_INFO("");
		LOG_INFO("server receive MSG_REQUEST_SEG from " << clientNum << " fileId:" << fileId << ",segId:" << segId <<
				" and start simulator transport");

		if(mCurReqBlock == mReqList.end()){
			GetNextBlock();
			mFakeTran.Active();
		}

		break;
	}
	case MSG_DELETE_SEG:{ //客户端缓冲区删除某一缓存块
		int clientNum = *ptr;
		ptr++;
		int fileId = *ptr;
		ptr++;
		int segId = *ptr;
		LOG_INFO("");
		LOG_INFO("server receive MSG_DELETE_SEG from " << clientNum <<
				" and delete the fileId:" << fileId << ",segId:" << segId <<
				" from database");
		mDataServer->DeleteFromIndex(fileId,segId,clientNum);
		break;
	}
	case MSG_ADD_SEG:{
		int clientNum = *ptr;
		ptr++;
		int fileId = *ptr;
		ptr++;
		int segId = *ptr;
		ptr++;
		int linkedNum = *ptr;
		LOG_INFO("");
		LOG_INFO("server receive MSG_ADD_SEG from " << clientNum <<
				" and add the fileId:" << fileId << ",segId:" << segId <<
				" into database");
		mDataServer->InsertIntoIndex(fileId,segId,clientNum,linkedNum);
		break;
	}
	case MSG_FAKE_FIN:{
		int toClientNum = *ptr;
		ptr++;
		int oper = *ptr;

		bool isNativeProduce = false;

		list<ClientReqBlock>::iterator tmpIter = mReqList.begin();
		while(oper == OPER_WRITE && tmpIter != mReqList.end()){
			if(tmpIter->clientNum == toClientNum){
				if(tmpIter->leftSize <= 0){
					char buffer[20];
					int *tmpPtr = (int *)buffer;
					*tmpPtr = MSG_SEG_FIN;
					tmpPtr++;
					*tmpPtr = 0;
					tmpPtr++;
					*tmpPtr = tmpIter->fileId;
					tmpPtr++;
					*tmpPtr = tmpIter->segId;
					send(mClientInfo[tmpIter->clientNum].recvFd,buffer,20,0);
					if(mCurReqBlock == tmpIter){
						GetNextBlock();
						isNativeProduce = true;
					}
					mReqList.erase(tmpIter);

					mUsedBandSize -= (mBlockSize * 1000);
					mNeedSendBandSize -= (mBlockSize * 1000);

					bool localTag = false;
					list<ClientReqBlock>::iterator preIter = mPreReqList.begin();
//					list<ClientReqBlock>::iterator tmpIter;
					while(preIter != mPreReqList.end()){
						bool tmpTag = false;
						if(mIsP2POpen){
							int bestClient = mDataServer->SearchBestClient(preIter->fileId,
									preIter->segId);

							//								LOG_INFO("server find best client " << bestClient);
							if(bestClient != -1){
//								char buffer[20];
								tmpPtr = (int *)buffer;
								*tmpPtr = MSG_REDIRECT;
								tmpPtr++;
								*tmpPtr = 0;
								tmpPtr++;
								*tmpPtr = bestClient;
								tmpPtr++;
								*tmpPtr = mClientInfo[bestClient].address.sin_addr.s_addr;
								tmpPtr++;
								*tmpPtr = mClientInfo[bestClient].address.sin_port;
								send(mClientInfo[preIter->clientNum].recvFd,buffer,20,0);
								LOG_INFO("");
								LOG_INFO("server send MSG_REDIRECT to " << preIter->clientNum <<
										" redirect to " << bestClient);
								mNeedSendBandSize -= (mBlockSize * 1000);
								tmpIter = preIter;
								preIter++;
								mPreReqList.erase(tmpIter);
								tmpTag = true;
								//									break;
							}
						}

						if(!tmpTag){
							tmpIter = mReqList.begin();
							int count = 0;
							while(tmpIter != mReqList.end()){
								if(preIter->fileId == tmpIter->fileId &&
										preIter->segId == tmpIter->segId)
									count++;
								tmpIter++;
							}
							if(count < 3){
								ClientReqBlock tmpReqBlock = *preIter;
								mReqList.push_back(tmpReqBlock);
								mPreReqList.erase(preIter);
								mUsedBandSize += (mBlockSize * 1000);
								break;
							}
							preIter++;
						}
					}

					if(!mPreReqList.empty() && preIter == mPreReqList.end()){
						ClientReqBlock tmpReqBlock = mPreReqList.front();
						mPreReqList.pop_front();
						mReqList.push_back(tmpReqBlock);
						mUsedBandSize += (mBlockSize * 1000);
					}
					break;
				}

				if(tmpIter->localfin == REMOTE_FIN){
					tmpIter->oper = tmpIter->preOper;
					tmpIter->localfin = NONE_FIN;
				}
				else{
					tmpIter->localfin = LOCAL_FIN;
				}
				break;
			}
			tmpIter++;
		}

		if(mCurReqBlock != mReqList.end() && !isNativeProduce)
			GetNextBlock();

		if(mCurReqBlock != mReqList.end() && mCurReqBlock->oper != OPER_WAIT){
			mCurReqBlock->oper = OPER_WAIT;
			mFakeTran.TranData(mBand,mPerSendSize,mCurReqBlock->clientNum,mCurReqBlock->preOper);
			mCurReqBlock->localfin = NONE_FIN;
			char buffer[20];
			int *tmpPtr = (int *)buffer;
			*tmpPtr = MSG_REMOTE_FAKE_FIN;
			tmpPtr++;
			*tmpPtr = 0;//服务器编号
			tmpPtr++;
			*tmpPtr = mCurReqBlock->preOper == OPER_READ ? OPER_WRITE : OPER_READ;
			send(mClientInfo[mCurReqBlock->clientNum].recvFd,buffer,20,0);

		}

		break;
	}
	case MSG_REMOTE_FAKE_FIN:{
		int clientNum = *ptr;
		ptr++;
		list<ClientReqBlock>::iterator iter = mReqList.begin();
//		list<ClientReqBlock>::iterator tmpIter = mReqList.begin();

		bool isFound = false;

		while(iter != mReqList.end()){
			if(iter->clientNum == clientNum){
				iter->leftSize -= mPerSendSize;
				if(iter->localfin == LOCAL_FIN){
					iter->oper = iter->preOper;
					iter->localfin = NONE_FIN;
					isFound = true;
				}
				else{
					iter->localfin = REMOTE_FIN;
					isFound = false;
				}
				if(iter->leftSize <= 0 && iter->localfin != REMOTE_FIN){
					//					mReqList.erase(iter);
					char buffer[20];
					int *tmpPtr = (int *)buffer;
					*tmpPtr = MSG_SEG_FIN;
					tmpPtr++;
					*tmpPtr = 0;
					tmpPtr++;
					*tmpPtr = iter->fileId;
					tmpPtr++;
					*tmpPtr = iter->segId;
					send(mClientInfo[iter->clientNum].recvFd,buffer,20,0);
					if(mCurReqBlock == iter){
						mCurReqBlock++;
					}
					mUsedBandSize -= (mBlockSize * 1000);
					mNeedSendBandSize -= (mBlockSize * 1000);

					mReqList.erase(iter);

					list<ClientReqBlock>::iterator preIter = mPreReqList.begin();
					list<ClientReqBlock>::iterator tmpIter;
					while(preIter != mPreReqList.end()){
						bool tmpTag = false;
						if(mIsP2POpen){
							int bestClient = mDataServer->SearchBestClient(preIter->fileId,
									preIter->segId);

							//								LOG_INFO("server find best client " << bestClient);
							if(bestClient != -1){
//								char buffer[20];
								tmpPtr = (int *)buffer;
								*tmpPtr = MSG_REDIRECT;
								tmpPtr++;
								*tmpPtr = 0;
								tmpPtr++;
								*tmpPtr = bestClient;
								tmpPtr++;
								*tmpPtr = mClientInfo[bestClient].address.sin_addr.s_addr;
								tmpPtr++;
								*tmpPtr = mClientInfo[bestClient].address.sin_port;
								send(mClientInfo[preIter->clientNum].recvFd,buffer,20,0);
								LOG_INFO("");
								LOG_INFO("server send MSG_REDIRECT to " << preIter->clientNum <<
										" redirect to " << bestClient);
								mNeedSendBandSize -= (mBlockSize * 1000);
								tmpIter = preIter;
								preIter++;
								mPreReqList.erase(tmpIter);
								tmpTag = true;
								//									break;
							}
						}

						if(!tmpTag){
							tmpIter = mReqList.begin();
							int count = 0;
							while(tmpIter != mReqList.end()){
								if(preIter->fileId == tmpIter->fileId &&
										preIter->segId == tmpIter->segId)
									count++;
								tmpIter++;
							}
							if(count < 3){
								ClientReqBlock tmpReqBlock = *preIter;
								mPreReqList.erase(preIter);
								mReqList.push_back(tmpReqBlock);
								break;
							}
							preIter++;
						}
					}

//					LOG_INFO("2");

					if(!mPreReqList.empty() && preIter == mPreReqList.end()){
						ClientReqBlock tmpReqBlock = mPreReqList.front();
						mPreReqList.pop_front();
						mReqList.push_back(tmpReqBlock);
					}

				}
				else{
					iter->oper = iter->preOper;
					isFound = true;
				}
				//				if(mCurReqBlock == mReqList.end())
				//					mCurReqBlock = iter;
				break;
			}
			iter++;
		}
//		if(mCurReqBlock != mReqList.end() && mCurReqBlock->)
//		GetNextBlock();
		if(isFound && mCurReqBlock == mReqList.end()){
			GetNextBlock();
			mFakeTran.Active();
		}
		break;
	}
	case MSG_CLIENT_DELAY:{
		int clientNum = *ptr;
		ptr++;
		double delayTime = *((double *)ptr);
		mClientDelayOfs[clientNum].setf(ios::fixed, ios::floatfield);
		mClientDelayOfs[clientNum].precision(2);
		mClientDelayOfs[clientNum] << delayTime << endl;
		break;
	}
	default:{
		LOG_INFO("");
		LOG_INFO("server receive unknow message");
		break;
	}
	}
}

void MyServer::GetNextBlock(){
	list<ClientReqBlock>::iterator iter = mCurReqBlock;
	if(iter == mReqList.end())
		mCurReqBlock = mReqList.begin();
//	else if(mCurReqBlock->oper != OPER_WAIT && mCurReqBlock->leftSize > 0){
//		return;
//	}
	else
		mCurReqBlock++;

	while(mCurReqBlock != iter){
		if(mCurReqBlock != mReqList.end() &&
				mCurReqBlock->oper != OPER_WAIT && mCurReqBlock->leftSize > 0){
			return;
		}
		if(mCurReqBlock == mReqList.end())
			mCurReqBlock = mReqList.begin();
		else
			mCurReqBlock++;
	}

	if(iter != mReqList.end() && mCurReqBlock->oper != OPER_WAIT && mCurReqBlock->leftSize > 0)
		return;

	mCurReqBlock = mReqList.end();
}

