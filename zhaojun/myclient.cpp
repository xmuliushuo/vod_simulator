
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include <sstream>

#include "log.h"

void *ThreadToClient(void *arg){
	MyClient *client = (MyClient *)arg;
	client->Run();
	return NULL;
}

MyClient::MyClient(ModelAssemble *model,int blockSize,int perSendSize,
		double bandWidth,int blockNums,int clientNum,int serverPort,int clientPort,int devNums,
		int clientNums,char **clusterAddress,char *serverAddress,char *bufferStrategy,
		int period,double lrfuLambda,bool isRepeat,int preFetch,bool special,bool isStartTogether){
//	mServerFd = serverFd;
	mIsStartTogether = isStartTogether;
	mModel = model;
	mOldFileId = -1;
	mOldSegId = -1;
	mBlockSize = blockSize;
	mPerSendSize = perSendSize;
	mBand = bandWidth;
	mBlockNum = blockNums;
	mClientNum = clientNum;

	mPreFetch = preFetch;

	mBufferStrategy = bufferStrategy;

	mPeriod = period;
	mLrfuLambda = lrfuLambda;

	mIsRepeat = isRepeat;

	stringstream sstream;
	sstream.str("");
	sstream << "data/clientHit" << clientNum << ".log";
	string fileName = sstream.str();
	mOFs.open(fileName.c_str(),ios::out);

	mDelay = false;

	sstream.str("");
	sstream << "data/client" << clientNum << ".log";
	fileName = sstream.str();

	mRecordFs.open(fileName.c_str(),ios::out);

	mIsReadFin = true;
	mAskNum = 0;

	mLinkedNums = 0;

	mSpecial = special;
	mIsFirstStart = true;

	if(strcmp(mBufferStrategy,"LRU") == 0){
		mDbuffer = new DBufferLRU(blockSize,mBlockNum);
	}
	else if(strcmp(mBufferStrategy,"LRUT") == 0){
		mDbuffer = new DBufferLRUT(blockSize,mBlockNum,mClientNum);
	}
	else if(strcmp(mBufferStrategy,"LFUT") == 0){
		mDbuffer = new DBufferLFUT(blockSize,mBlockNum,mClientNum);
	}
	else {
		assert(0);
	}

	mReqList.clear();

	mServerPort = serverPort;
	mClientPort = clientPort;
	mDevNums = devNums;
	mClientNums = clientNums;

	mServerAddress = serverAddress;

	mMyPort = mClientPort + ((mClientNum - 1) % (mClientNums / mDevNums));
	LOG_WRITE("Client " << mClientNum << " port is:" << mMyPort,mRecordFs);

	for(int i = 0;i < devNums;i++){
		mClusterAddress[i] = clusterAddress[i];
	}

	for(int i = 0;i <= MAX_CLIENT_NUM;i++){
//		mClientInfo[i].listenFd = -1;
		mClientInfo[i].recvFd = -1;
	}

	mCurReqBlock = mReqList.end();

	Init();

	LOG_WRITE("",mRecordFs);
	LOG_WRITE("client " << mClientNum << " init finished",mRecordFs);


	mIsPlaying = false;
	mJumpSeg = 0;
	mPlayerStatus = PLAY;

	mHitTimes = 0;
	mTotalTimes = 0;
	
	mDload1 = -1;
	mDload2 = -1;
	pthread_create(&mtid,NULL,ThreadToClient,this);
}

MyClient::~MyClient(){
	mReqList.clear();
	delete mDbuffer;
	mModel = NULL;
	mCurReqBlock = mReqList.end();
	for(int i = 0;i <= MAX_CLIENT_NUM;i++){
		if(mClientInfo[i].recvFd != -1){
//			close(mClientInfo[i].listenFd);
			close(mClientInfo[i].recvFd);
//			mClientInfo[i].listenFd = -1;
			mClientInfo[i].recvFd = -1;
		}
	}

	mOFs.close();
	mRecordFs.close();
	close(mNetSockFd);
	close(mEpollFd);
//	mOFs.close();
}

void MyClient::Init(){
	mEpollFd = epoll_create(MAX_LISTEN_NUM);
	epoll_event ev;

	mNetSockFd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in clientAddress;
	bzero(&clientAddress,sizeof(clientAddress));
	clientAddress.sin_addr.s_addr = INADDR_ANY;
	clientAddress.sin_port = htons(mMyPort);
	clientAddress.sin_family = AF_INET;

	int val = 1;
	setsockopt(mNetSockFd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));

	if(bind(mNetSockFd,(struct sockaddr *)&clientAddress,sizeof(clientAddress)) == -1){
		LOG_WRITE("Client " << mClientNum << " Bind Error!",mRecordFs);
		exit(1);
	}

	if(listen(mNetSockFd,20) == -1){
		LOG_WRITE("Client " << mClientNum << " Listen Error!",mRecordFs);
		exit(1);
	}

	ev.data.fd = mNetSockFd;
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mNetSockFd,&ev);

	ev.data.fd = mFakeTran.GetFd();
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mFakeTran.GetFd(),&ev);

	socketpair(AF_UNIX,SOCK_STREAM,0,mPlaySockFd);
	ev.data.fd = mPlaySockFd[0];
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mPlaySockFd[0],&ev);
	
	socketpair(AF_UNIX,SOCK_STREAM,0,mBufferResetFd);
	ev.data.fd = mBufferResetFd[0];
	ev.events = EPOLLIN;
	epoll_ctl(mEpollFd,EPOLL_CTL_ADD,mBufferResetFd[0],&ev);
}

//int MyClient::JudgeCluster(char *address){
//	for(int i = 0;i < mDevNums;i++){
//		if(strcmp(address,mClusterAddress[i]) == 0)
//			return i;
//	}
//	return -1;
//}

void MyClient::BufferReset(){
	TimerEvent timeEvent;
	timeEvent.isNew = true;
	timeEvent.sockfd = mBufferResetFd[1];
	timeEvent.leftTime = mPeriod * 1000000;
	globalTimer.RegisterTimer(timeEvent);
	mDbuffer->BlockReset();
}

void MyClient::Run(){
	fstream iofs;
	bool readOrWrite = false;

	TimerEvent timeEvent;

	if(mIsRepeat){
		stringstream sstream;
		sstream.str("");
		sstream << "data/requestFile" << mClientNum << ".log";
		string requestFilename = sstream.str();
		iofs.open(requestFilename.c_str(),ios::in);
		readOrWrite = true;
		if(iofs.fail()){
			iofs.open(requestFilename.c_str(),ios::out);
			readOrWrite = false;
		}
	}

//	LOG_WRITE("client " << mClientNum << " before");
	if(mIsRepeat && readOrWrite){
		iofs >> mFileId;
		iofs >> mSegId;
		iofs >> timeEvent.leftTime;
		iofs >> mSegId;
	}
	else{
		mFileId = mModel->GetStartFileId();
		mSegId = mModel->GetStartSegId();
		timeEvent.leftTime = mModel->GetStartTime(mClientNum) * 1000000;
		if(mIsRepeat){
			iofs << mFileId << endl;
			iofs << mSegId << endl;
			iofs << timeEvent.leftTime << endl;
		}
	}

	if(mIsStartTogether)
		timeEvent.leftTime = 0;

	cout << mFileId << " is client " << mClientNum << " request file" << endl;
//	LOG_WRITE("client " << mClientNum << " before1");
//	LOG_WRITE("client " << mClientNum << " before2");

	timeEvent.isNew = true;
	timeEvent.sockfd = mFakeTran.GetOtherFd();

	LOG_WRITE("",mRecordFs);
	LOG_WRITE("client " << mClientNum << " start time at:" << timeEvent.leftTime,mRecordFs);
	globalTimer.RegisterTimer(timeEvent);

	bool firstTime = true;
	int toggleSwitch = 0;

	epoll_event events[MAX_LISTEN_NUM];

	int oldSegId = 0;
	int readSegId = 1;

	bool isFromBuffer = false;


	while(true){
		int fds = epoll_wait(mEpollFd,events,MAX_LISTEN_NUM,-1);

		if(firstTime){
			int connectFd;
			char buffer[20];
			read(events[0].data.fd,buffer,20);

			struct sockaddr_in serverAddress;
			bzero(&serverAddress,sizeof(serverAddress));
			serverAddress.sin_addr.s_addr = inet_addr(mServerAddress);
			serverAddress.sin_family = AF_INET;
			serverAddress.sin_port = htons(mServerPort);
			connectFd = socket(AF_INET,SOCK_STREAM,0);

			mClientInfo[0].recvFd = connectFd;
			mClientInfo[0].address.sin_addr = serverAddress.sin_addr;
			mClientInfo[0].address.sin_port = serverAddress.sin_port;

			int flag = 1;
			setsockopt(connectFd,IPPROTO_TCP,TCP_NODELAY,&flag,sizeof(flag));

			if(connect(connectFd,(struct sockaddr *)&serverAddress,sizeof(struct sockaddr)) == -1){
				LOG_WRITE("Client " << mClientNum << " Connect 0 Error!",mRecordFs);
				exit(1);
			}

			epoll_event ev;
			ev.data.fd = connectFd;
			ev.events = EPOLLIN;
			epoll_ctl(mEpollFd,EPOLL_CTL_ADD,connectFd,&ev);

			int *tmpPtr = (int *)buffer;
			*tmpPtr = mClientNum;
			send(connectFd,buffer,20,0);

			if(mDbuffer->IsBlockReset()){
				timeEvent.isNew = true;
				timeEvent.sockfd = mBufferResetFd[1];
				timeEvent.leftTime = mPeriod * 1000000;
				globalTimer.RegisterTimer(timeEvent);
				LOG_WRITE("",mRecordFs);
				LOG_WRITE("client " << mClientNum << " has blockreset reset period:" << mPeriod,mRecordFs);
			}	

			firstTime = false;

			unsigned int length = mModel->GslRandLogNormal();
			mJumpSeg = length / mBlockSize;

			if(readOrWrite)
			{
				LOG_WRITE("",mRecordFs);
				LOG_WRITE("client " << mClientNum << " read",mRecordFs);
				mJumpSeg = 1;
				LOG_WRITE("client " << mClientNum << " JumpSeg:" << mJumpSeg,mRecordFs);
			}

			LOG_WRITE("",mRecordFs);
			LOG_WRITE("client " << mClientNum << " start to join the network request fileId:" <<
					mFileId,mRecordFs);

			continue;
		}
		

		for(int i = 0;i < fds;i++){
			if(events[i].events == EPOLLIN){
				if(events[i].data.fd != mPlaySockFd[0]){
					if(events[i].data.fd == mFakeTran.GetFd()){
						char buffer[20];
						read(mFakeTran.GetFd(),buffer,20);
						DealWithMessage(buffer,20);
					}
					else if(events[i].data.fd == mNetSockFd){
						struct sockaddr_in clientAddr;
						unsigned int length;
						epoll_event ev;
						length = sizeof(clientAddr);
						int clientFd = accept(mNetSockFd,(struct sockaddr *)&clientAddr,&length);
//						mLinkedNums++;

						char buffer[20];
						int *ptr = (int *)buffer;
						recv(clientFd,buffer,20,0);
						int clientNum = *ptr;

						int port = mClientPort + ((clientNum - 1) % (mClientNums / mDevNums));

						if(mClientInfo[clientNum].recvFd == -1){
							mClientInfo[clientNum].address.sin_addr = clientAddr.sin_addr;
							mClientInfo[clientNum].address.sin_port = htons(port);
							mClientInfo[clientNum].recvFd = clientFd;
						}

						LOG_WRITE("",mRecordFs);
						LOG_WRITE("client " << mClientNum << " response MSG_CONNECT_FIN to " <<
								clientNum << " fd is:" << clientFd,mRecordFs);

						ev.data.fd = clientFd;
						ev.events = EPOLLIN;

						epoll_ctl(mEpollFd,EPOLL_CTL_ADD,clientFd,&ev);

						ptr = (int *)buffer;
						*ptr = MSG_CONNECT_FIN;
						ptr++;
						*ptr = mClientNum;
						send(mClientInfo[clientNum].recvFd,buffer,20,0);
					}
					else if(events[i].data.fd == mBufferResetFd[0]){
						char buffer[20];
						int length = recv(events[i].data.fd,buffer,20,0);
						BufferReset();	
						LOG_WRITE("",mRecordFs);
						LOG_WRITE("client " << mClientNum << " reset buffer",mRecordFs);
					}
					else{
						char buffer[20];
						memset(buffer,0,20);
						int length = recv(events[i].data.fd,buffer,20,0);
						DealWithMessage(buffer,length);
//						int asdf = events[i].events == EPOLLIN ? 1 : 0;
//						LOG_WRITE("my fd:" << events[i].data.fd << "    epollin:" << asdf << " length:" << length);
					}
				}
				else{
					char buffer[20];
					read(mPlaySockFd[0],&buffer,20);

					LOG_WRITE("",mRecordFs);
					LOG_WRITE("client " << mClientNum << "JumpSeg:" << mJumpSeg,mRecordFs);
					
					if(mJumpSeg == 0){
						mPlayerStatus = mModel->GetNextStatus(mPlayerStatus);

						if(mPlayerStatus == STOP){
							LOG_WRITE("",mRecordFs);
							LOG_WRITE("client " << mClientNum << " status is STOP",mRecordFs);
							break;
						}

						unsigned int length = mModel->GslRandLogNormal();
						mJumpSeg = length / mBlockSize;

						LOG_WRITE("",mRecordFs);
						LOG_WRITE("client " << mClientNum << " currentstatus is:" << mPlayerStatus <<
								",JumpSeg:" << mJumpSeg << ",length:" << length,mRecordFs);

						if(mJumpSeg == 0)
							mJumpSeg++;
						if(mPlayerStatus == FORWARD){
							mSegId += mJumpSeg;
							if(mSegId > mMaxSegId){
								mSegId = 1;
//								mSegId = mMaxSegId;
							}
							mJumpSeg = 0;
//							char nullChar = '\0';
							send(mPlaySockFd[1],buffer,20,0);
//							continue;
						}
						else if(mPlayerStatus == BACKWARD){
							mSegId -= mJumpSeg;
							if(mSegId < 1)
								mSegId = 1;
							mJumpSeg = 0;
//							char nullChar = '\0';
							send(mPlaySockFd[1],buffer,20,0);
//							continue;
						}
					}

					bool isPreFetch;
					if(mPreFetch > 0)
						isPreFetch = true;
					else
						isPreFetch = false;

					if(mJumpSeg != 0 || readOrWrite){
						bool isJustStart = false;
						if(oldSegId == 0)
							isJustStart = true;

						if(!mDelay){
							mDelay = true;
							gettimeofday(&mDelayBeginTime,NULL);
							if(mSpecial && mOldFileId != -1){
								int tmpFileId,tmpSegId;
								char buffer[20];
								int *tmpPtr = (int *)buffer;
								*tmpPtr = MSG_SEG_FIN;
								tmpPtr++;
								*tmpPtr = mClientNum;
								tmpPtr++;
								*tmpPtr = mOldFileId;
								tmpPtr++;
								*tmpPtr = mOldSegId;
								send(mClientInfo[mAskNum].recvFd,buffer,20,0);
								mDbuffer->Write(mOldFileId,mOldSegId,tmpFileId,tmpSegId);
								if(tmpFileId != -1){
									char delbuffer[20];
									tmpPtr = (int *)delbuffer;
									*tmpPtr = MSG_DELETE_SEG;
									tmpPtr++;
									*tmpPtr = mClientNum;
									tmpPtr++;
									*tmpPtr = tmpFileId;
									tmpPtr++;
									*tmpPtr = tmpSegId;
									LOG_WRITE("",mRecordFs);
									LOG_WRITE("client " << mClientNum << " send MSG_DELETE_SEG fileId:" << tmpFileId <<
											",segId" << tmpSegId,mRecordFs);
									send(mClientInfo[0].recvFd,delbuffer,20,0);
								}
								char addbuffer[20];
								tmpPtr = (int *)addbuffer;
								*tmpPtr = MSG_ADD_SEG;
								tmpPtr++;
								*tmpPtr = mClientNum;
								tmpPtr++;
								*tmpPtr = mOldFileId;
								tmpPtr++;
								*tmpPtr = mOldSegId;
								tmpPtr++;
								*tmpPtr = mLinkedNums;
								LOG_WRITE("",mRecordFs);
								LOG_WRITE("client " << mClientNum << " send MSG_ADD_SEG fileId:" << mOldFileId <<
										",segId" << mOldSegId,mRecordFs);
								send(mClientInfo[0].recvFd,addbuffer,20,0);
								mAskNum = 0;
							}
						}

						oldSegId = mSegId;
						mSegId = readSegId;
						bool isRead;
						if(mSpecial){
							mTotalTimes++;
							isRead = mDbuffer->Read(mFileId,mSegId);
							if(isRead){
								isFromBuffer = true;
								mHitTimes++;
							}
							else{
								if(mSegId == mDload1 || mSegId == mDload2)
									isRead = true;
								else
									isRead = false;

								isFromBuffer = false;
//								LOG_WRITE("now isRead ")
							}
						}
						else
							isRead = mDbuffer->Read(mFileId,mSegId);

						if(readOrWrite){
							if(!mSpecial){
								if((mSegId - oldSegId) != 0 && isRead && oldSegId != 0){
									mHitTimes++;
								}
								else if((mSegId - oldSegId) != 0 && !isRead){
									mTotalTimes--;
								}
							}
						}

						double delayTime = 0;

						if(mSpecial && !isRead && !isJustStart){
//							mAskNum = 0;
							int *ptr;
							ptr = (int *)buffer;
							*ptr = MSG_SEG_ASK;
							ptr++;
							*ptr = mClientNum;
							ptr++;
							*ptr = mFileId;
							ptr++;
							*ptr = mSegId;
							send(mClientInfo[mAskNum].recvFd,buffer,20,0);
							LOG_WRITE("",mRecordFs);
							LOG_WRITE("client " << mClientNum << " send MSG_SEG_ASK to " << mAskNum
									<< " fileId:" << mFileId << " segId:" << mSegId,mRecordFs);
							//
						}

						if(isRead || mSpecial){

							if(!isFromBuffer){
								mOldFileId = mFileId;
								mOldSegId = mSegId;
							}
							else
								mOldFileId = -1;

							mDelay = false;
							gettimeofday(&mDelayEndTime,NULL);
							delayTime = ((mDelayEndTime.tv_sec - mDelayBeginTime.tv_sec) +
									(mDelayEndTime.tv_usec - mDelayBeginTime.tv_usec) / 1000000.0);
							if(delayTime < 0.1)
								delayTime = 0;

							mOFs << mTotalTimes << " " << mHitTimes << endl;

							timeEvent.isNew = true;
							timeEvent.leftTime = ((mBlockSize * 8) * 1000000) / mBitRate;
							timeEvent.sockfd = mPlaySockFd[1];
							globalTimer.RegisterTimer(timeEvent);
							LOG_WRITE("",mRecordFs);
							LOG_WRITE("client " << mClientNum << " play fileId:" << mFileId << ",segId:" << mSegId <<
									",playtime:" << timeEvent.leftTime << " Hit times:" << mHitTimes << " TotalTimes:" << mTotalTimes,mRecordFs);
							

							if(!readOrWrite){
								LOG_WRITE("",mRecordFs);
								LOG_WRITE("client " << mClientNum << " write",mRecordFs);
								mJumpSeg--;
								if(mIsRepeat)
									iofs << mSegId << endl;
							}
							else{
								iofs >> readSegId;
//								iofs >> mSegId;
							}
							if(!mSpecial){
								mSegId++;
							}

//							isSegChange = true;

							if(mSegId > mMaxSegId)
								mSegId = 1;

							mIsPlaying = true;
						}
						else{
							mIsPlaying = false;
							isPreFetch = true;
						}

						bool isInBuffer = false;

						if(isPreFetch && !mSpecial)
							isInBuffer = mDbuffer->FindBlock(mFileId,mSegId);
						if(mIsReadFin && !isInBuffer && !mSpecial){
							mTotalTimes++;
							int *ptr;
							if(!mDelay){
								ptr = (int *)buffer;
								*ptr = MSG_CLIENT_DELAY;
								ptr++;
								*ptr = mClientNum;
								ptr++;
								double *dbptr = (double *)ptr;
								*dbptr = delayTime;
								send(mClientInfo[0].recvFd,buffer,20,0);
							}
							//							if(!SearchTheReqList(mFileId,mSegId)){

							mIsReadFin = false;
							ptr = (int *)buffer;
							*ptr = MSG_SEG_ASK;
							ptr++;
							*ptr = mClientNum;
							ptr++;
							*ptr = mFileId;
							ptr++;
							*ptr = mSegId;
							send(mClientInfo[mAskNum].recvFd,buffer,20,0);
							LOG_WRITE("",mRecordFs);
							LOG_WRITE("client " << mClientNum << " send MSG_SEG_ASK to " << mAskNum << " TWO",mRecordFs);
							//							}
						}
						else if(isInBuffer){
							if(mSpecial){
								mHitTimes++;
							}
							else{
								if((mSegId - readSegId) == 0){
									mHitTimes++;
								}
							}
							mTotalTimes++;
						}
					}
				}
			}
		}
	}

	if(mIsRepeat)
		iofs.close();
}

bool MyClient::SearchTheReqList(int fileId,int segId){
	list<ClientReqBlock>::iterator reqIter = mReqList.begin();
	while(reqIter != mReqList.end()){
		if(reqIter->fileId == fileId && reqIter->segId == segId && reqIter->preOper == OPER_READ)
			return true;
		reqIter++;
	}
	return false;
}

void MyClient::DealWithMessage(char *buf,int length){
	int *ptr = (int *)buf;
	int type = *ptr;
	ptr++;
	switch(type){
	case MSG_CONNECT_FIN:{
		int clientNum = *ptr;
		ptr++;
		int mytag = *ptr;
//		LOG_WRITE("my tag is:" << mytag);
		char buffer[20];
		ptr = (int *)buffer;
		*ptr = MSG_CLIENT_JOIN;
		ptr++;
		*ptr = mClientNum;
		ptr++;

		send(mClientInfo[clientNum].recvFd,buffer,20,0);
		LOG_WRITE("",mRecordFs);
		LOG_WRITE("Client " << mClientNum << " receive MSG_CONNECT_FIN from " << clientNum <<
				" and response MSG_CLIENT_JOIN " << mClientInfo[clientNum].recvFd,mRecordFs);
		break;
	}
	case MSG_CLIENT_JOIN:{
		int clientNum = *ptr;
		ptr++;
		epoll_event ev;


		char buffer[20];
		int *tmpPtr = (int *)buffer;
		*tmpPtr = MSG_JOIN_ACK;
		tmpPtr++;
		*tmpPtr = mClientNum;
		send(mClientInfo[clientNum].recvFd,buffer,20,0);
		LOG_WRITE("",mRecordFs);
		LOG_WRITE("client " << mClientNum << " response MSG_JOIN_ACK to " << clientNum,mRecordFs);

		break;
	}
	case MSG_JOIN_ACK:{
		int clientNum = *ptr;
		if(mAskNum == 0 && !mSpecial)
			mTotalTimes++;

		*tmpPtr = mFileId;
		tmpPtr++;
		*tmpPtr = mSegId;
		send(mClientInfo[clientNum].recvFd,buffer,20,0);

		LOG_WRITE("",mRecordFs);
		LOG_WRITE("client " << mClientNum << " receive MSG_JOIN_ACK from " << clientNum <<
				" and response MSG_SEG_ASK, ask fileId:" << mFileId << ",segId:" << mSegId,mRecordFs);
		break;
	}
	case MSG_CLIENT_LEAVE:{
		int clientNum = *ptr;
		epoll_event ev;
		ev.data.fd = mClientInfo[clientNum].recvFd;
		ev.events = EPOLLIN;
		epoll_ctl(mEpollFd,EPOLL_CTL_DEL,mClientInfo[clientNum].recvFd,&ev);
		close(mClientInfo[clientNum].recvFd);
		mClientInfo[clientNum].recvFd = -1;
		LOG_WRITE("",mRecordFs);
		LOG_WRITE("client " << mClientNum << " receive MSG_CLIENT_LEAVE from " << clientNum,mRecordFs);
		break;
	}
	case MSG_SEG_ACK:{
		int clientNum = *ptr;
		ptr++;
		mMaxSegId = *ptr;
		ptr++;
		mBitRate = *((double *)ptr);

		if(mAskNum != 0)
			mHitTimes++;

		if(mSpecial && mIsFirstStart){
			char buffer[20];
			send(mPlaySockFd[1],buffer,20,0);
			mIsFirstStart = false;
		}

		if(!mSpecial){
			ClientReqBlock reqBlock;
			reqBlock.clientNum = clientNum;
			reqBlock.fileId = mFileId;
			reqBlock.segId = mSegId;
			reqBlock.preOper = OPER_READ;
			reqBlock.oper = OPER_WAIT;
			reqBlock.leftSize = mBlockSize * 1000;
			reqBlock.localfin = NONE_FIN;
			mReqList.push_back(reqBlock);

			//		mLinkedNums++;

			char buffer[20];
			int *tmpPtr = (int *)buffer;
			*tmpPtr = MSG_REQUEST_SEG;
			tmpPtr++;
			*tmpPtr = mClientNum;
			tmpPtr++;
			*tmpPtr = mFileId;
			tmpPtr++;
			*tmpPtr = mSegId;
			send(mClientInfo[clientNum].recvFd,buffer,20,0);
			LOG_WRITE("",mRecordFs);
			LOG_WRITE("client " << mClientNum << " receive MSG_SEG_ACK from " << clientNum <<
					" and response MSG_REQUEST_SEG",mRecordFs);
		}
//		if(mCurReqBlock == NULL)
//			mFakeTran.Active();
		break;
	}
	case MSG_SEG_ASK:{
		int clientNum = *ptr;
		ptr++;
		int fileId = *ptr;
		ptr++;
		int segId = *ptr;
		if(!mSpecial)
			mLinkedNums = mReqList.size();
		bool isInBuffer = mDbuffer->Read(fileId,segId);
		if(mLinkedNums > MAX_CLIENT_LINKS || !isInBuffer){
			char buffer[20];
			int *tmpPtr = (int *)buffer;
			*tmpPtr = MSG_REDIRECT;
			tmpPtr++;
			*tmpPtr = mClientNum;
			tmpPtr++;
			*tmpPtr = 0;//服务器编号
			tmpPtr++;
			*tmpPtr = mClientInfo[0].address.sin_addr.s_addr;
			tmpPtr++;
			*tmpPtr = mClientInfo[0].address.sin_port;
			send(mClientInfo[clientNum].recvFd,buffer,20,0);
			LOG_WRITE("",mRecordFs);
			LOG_WRITE("client " << mClientNum << " receive MSG_SEG_ACK from " << clientNum <<
					" and response MSG_REDIRECT to server",mRecordFs);
			if(!isInBuffer){
				LOG_WRITE("",mRecordFs);
				LOG_WRITE("client " << mClientNum << " do no have fileId:" << fileId
						<< " segId:" << segId,mRecordFs);
			}
			break;
		}

		if(mSpecial)
			mLinkedNums++;
		char buffer[20];
		int *tmpPtr = (int *)buffer;
		*tmpPtr = MSG_SEG_ACK;
		tmpPtr++;
		*tmpPtr = mClientNum;
		tmpPtr++;
		*tmpPtr = mMaxSegId;
		tmpPtr++;
		double *dbPtr = (double *)tmpPtr;
		*dbPtr = mBitRate;

		send(mClientInfo[clientNum].recvFd,buffer,20,0);
		LOG_WRITE("",mRecordFs);
		LOG_WRITE("client " << mClientNum << " receive MSG_SEG_ASK from " << clientNum <<
				" and response MSG_SEG_ACK",mRecordFs);
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
		mReqList.push_back(reqBlock);
		if(mCurReqBlock == mReqList.end()){
			GetNextBlock();
			mFakeTran.Active();
		}
		LOG_WRITE("",mRecordFs);
		LOG_WRITE("client " << mClientNum << " receive MSG_REQUEST_SEG from " << clientNum,mRecordFs);
//		LOG_WRITE("client " << mClientNum << " request list size:" << mReqList.size());
		break;
	}
	case MSG_REDIRECT:{
		int fromClientNum = *ptr;
		ptr++;
		int clientNum = *ptr;
		ptr++;
//		int listenFd = *ptr;

		list<ClientReqBlock>::iterator iter = mReqList.begin();
		while(iter != mReqList.end()){
			if(iter->preOper == OPER_READ && iter->clientNum == fromClientNum){
				mReqList.erase(iter);
				break;
			}
			iter++;
		}

		mAskNum = clientNum;

		LOG_WRITE("",mRecordFs);
		LOG_WRITE("client " << mClientNum << " receive MSG_REDIRECT and response MSG_SEG_ASK to " <<
				clientNum,mRecordFs);

		if(mClientInfo[clientNum].recvFd == -1){
			struct sockaddr_in clientAddr;
			bzero(&clientAddr,sizeof(clientAddr));

			mClientInfo[clientNum].address.sin_addr.s_addr = *ptr;
			ptr++;
			mClientInfo[clientNum].address.sin_port = *ptr;

			clientAddr.sin_addr = mClientInfo[clientNum].address.sin_addr;
			clientAddr.sin_port = mClientInfo[clientNum].address.sin_port;
			clientAddr.sin_family = AF_INET;
			int connectFd = socket(AF_INET,SOCK_STREAM,0);

			epoll_event ev;
			ev.data.fd = connectFd;
			ev.events = EPOLLIN;

			mClientInfo[clientNum].recvFd = connectFd;
			epoll_ctl(mEpollFd,EPOLL_CTL_ADD,connectFd,&ev);

			if(connect(connectFd,(struct sockaddr *)&clientAddr,sizeof(clientAddr)) == -1){
				LOG_WRITE("client " << mClientNum << " MSG_REDIRECT " << clientNum << " error!!!",mRecordFs);
				exit(1);
			}

			char buffer[20];
			int *tmpPtr = (int *)buffer;
			*tmpPtr = mClientNum;
			send(connectFd,buffer,20,0);
			LOG_WRITE("",mRecordFs);
			LOG_WRITE("client " << mClientNum << " receive MSG_REDIRECT to "	<< clientNum,mRecordFs);
		}
		else{
			//mTotalTimes++;
			char buffer[20];
			int *tmpPtr = (int *)buffer;
			*tmpPtr = MSG_SEG_ASK;
			tmpPtr++;
			*tmpPtr = mClientNum;
			tmpPtr++;
			*tmpPtr = mFileId;
			tmpPtr++;
			*tmpPtr = mSegId;
			send(mClientInfo[clientNum].recvFd,buffer,20,0);
		}
		break;
	}
	case MSG_RESET_BUFFER:{
		mDbuffer->BlockReset();
		break;
	}
	case MSG_FAKE_FIN:{
		bool isNativeProduce = false;

		int toClientNum = *ptr;
		ptr++;
		int oper = *ptr;
		list<ClientReqBlock>::iterator tmpIter = mReqList.begin();
		while(oper == OPER_WRITE && tmpIter != mReqList.end()){
			if(tmpIter->clientNum == toClientNum && tmpIter->preOper == OPER_WRITE){
				if(tmpIter->leftSize <= 0){
					char buffer[20];
					int *tmpPtr = (int *)buffer;
					*tmpPtr = MSG_SEG_FIN;
					tmpPtr++;
					*tmpPtr = mClientNum;
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

		if(mCurReqBlock != mReqList.end()){
			if(mCurReqBlock->oper == OPER_WAIT && mCurReqBlock->preOper == OPER_READ){
				mCurReqBlock->leftSize -= mPerSendSize;
				int *tmpPtr;

				char buffer[20];
				tmpPtr = (int *)buffer;
				*tmpPtr = MSG_REMOTE_FAKE_FIN;
				tmpPtr++;
				*tmpPtr = mClientNum;
				tmpPtr++;
				*tmpPtr = mCurReqBlock->preOper == OPER_READ ? OPER_WRITE : OPER_READ;
				int sendLength;
				sendLength = send(mClientInfo[mCurReqBlock->clientNum].recvFd,buffer,20,0);

				if(mCurReqBlock->leftSize <= 0){
					list<ClientReqBlock>::iterator reqIter = mCurReqBlock;
					mCurReqBlock++;
					mReqList.erase(reqIter);
				}
				GetNextBlock();
			}
			else if(!isNativeProduce && mCurReqBlock->preOper == OPER_WRITE){
				GetNextBlock();
			}

			if(mCurReqBlock != mReqList.end()){
				mCurReqBlock->oper = OPER_WAIT;
				mFakeTran.TranData(mBand,mPerSendSize,mCurReqBlock->clientNum,mCurReqBlock->preOper);
				if(mCurReqBlock->preOper == OPER_WRITE){
					mCurReqBlock->localfin = NONE_FIN;
					char buffer[20];
					int *tmpPtr = (int *)buffer;
					*tmpPtr = MSG_REMOTE_FAKE_FIN;
					tmpPtr++;
					*tmpPtr = mClientNum;
					tmpPtr++;
					*tmpPtr = mCurReqBlock->preOper == OPER_READ ? OPER_WRITE : OPER_READ;
					send(mClientInfo[mCurReqBlock->clientNum].recvFd,buffer,20,0);
//					GetNextBlock();
				}
			}
		}
		break;
	}
	case MSG_REMOTE_FAKE_FIN:{
		int clientNum = *ptr;
		ptr++;
		int oper = *ptr;
		list<ClientReqBlock>::iterator iter = mReqList.begin();

		bool isFound = false;

		while(iter != mReqList.end()){
			if(iter->clientNum == clientNum && iter->preOper == oper){
				if(iter->preOper == OPER_WRITE){
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

					if(isFound && iter->leftSize <= 0){
						char buffer[20];
						int *tmpPtr = (int *)buffer;
						*tmpPtr = MSG_SEG_FIN;
						tmpPtr++;
						*tmpPtr = mClientNum;
						tmpPtr++;
						*tmpPtr = iter->fileId;
						tmpPtr++;
						*tmpPtr = iter->segId;
						send(mClientInfo[iter->clientNum].recvFd,buffer,20,0);
						if(mCurReqBlock == iter){
							mCurReqBlock++;
						}
//						mLinkedNums--;
						mReqList.erase(iter);
						isFound = false;
					}
				}
				else{
					iter->oper = iter->preOper;
					isFound = true;
				}

				break;
			}
			iter++;
		}

		if(isFound && mCurReqBlock == mReqList.end()){
			GetNextBlock();
			mFakeTran.Active();
		}
		break;
	}
	case MSG_SEG_FIN:{
		int clientNum = *ptr;
		ptr++;
		int fileId = *ptr;
		ptr++;
		int segId = *ptr;

		if(mSpecial){
			mLinkedNums--;
			LOG_WRITE("",mRecordFs);
			LOG_WRITE("client " << mClientNum << " receive MSG_SEG_FIN from " << clientNum
					<< " mLinkedNums:" << mLinkedNums,mRecordFs);

		}

		if(!mSpecial){
			int *tmpPtr;

			int ofileId = -1;
			int osegId = -1;

			LOG_WRITE("",mRecordFs);
			LOG_WRITE("client " << mClientNum << " receive MSG_SEG_FIN from " << clientNum,mRecordFs);

			char buffer[20];
			if(mSpecial){
				if(mDload1 != -1)
					mDload2 = mDload1;
				mDload1 = segId;
			}
			else{
				mDbuffer->Write(fileId,segId,ofileId,osegId);
			}

			//原播放的地方
			mIsReadFin = true;
			if(!mIsPlaying){
				//						char nullChar = '\0';
				send(mPlaySockFd[1],buffer,20,0);
				mIsPlaying = true;
				//LOG_WRITE("client " << mClientNum << " will play now segId:" << mCurReqBlock->segId << "," << "mSegId:" << mSegId);
			}

			if(ofileId != -1){
				char delbuffer[20];
				tmpPtr = (int *)delbuffer;
				*tmpPtr = MSG_DELETE_SEG;
				tmpPtr++;
				*tmpPtr = mClientNum;
				tmpPtr++;
				*tmpPtr = ofileId;
				tmpPtr++;
				*tmpPtr = osegId;
				LOG_WRITE("",mRecordFs);
				LOG_WRITE("client " << mClientNum << " send MSG_DELETE_SEG fileId:" << ofileId <<
						",segId" << osegId,mRecordFs);
				send(mClientInfo[0].recvFd,delbuffer,20,0);
			}
			if(!mSpecial){
				char addbuffer[20];
				tmpPtr = (int *)addbuffer;
				*tmpPtr = MSG_ADD_SEG;
				tmpPtr++;
				*tmpPtr = mClientNum;
				tmpPtr++;
				*tmpPtr = fileId;
				tmpPtr++;
				*tmpPtr = segId;
				tmpPtr++;
				*tmpPtr = mReqList.size() - 1;//mLinkedNums;
				LOG_WRITE("",mRecordFs);
				LOG_WRITE("client " << mClientNum << " send MSG_ADD_SEG fileId:" << fileId <<
						",segId" << segId,mRecordFs);
				send(mClientInfo[0].recvFd,addbuffer,20,0);
			}
		}
		break;
	}
	}
}

void MyClient::GetNextBlock(){
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
