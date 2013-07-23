/*
 * DataServer.cpp
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */

#include "dataserver.h"
#include "globalfunction.h"

DataServer::DataServer(int fileNum,int minLength,int maxLength,int blockSize,
		double minBitRate,double maxBitRate){
	mFileNum = fileNum;

	for(int i = 1;i <= MAX_FILE_NUM;i++){
		mFileInfo[i].bitRate = RandomF(minBitRate,maxBitRate);
		mFileInfo[i].fileId = i;
		mFileInfo[i].segNum = 0;
		mFileInfo[i].info.clear();
	}

	for(int i = 1;i <= fileNum;i++){
		int length = RandomI(minLength,maxLength);
		mFileInfo[i].segNum = (length * 1000) / blockSize;
	}
}

DataServer::~DataServer(){
	for(int i = 1;i <= mFileNum;i++){
		mFileInfo[i].info.clear();
	}
}

int DataServer::SearchBestClient(int fileId,int segId){
	int bestClient = -1;
	int minLinked = 1000000;
	if(!mFileInfo[fileId].info.empty()){
		list<FileInfoBlock>::iterator iter = mFileInfo[fileId].info.begin();
		while(iter != mFileInfo[fileId].info.end()){
			if(iter->segId == segId && mClientLinks[iter->clientNum] < minLinked &&
					mClientLinks[iter->clientNum] <= (MAX_CLIENT_LINKS * 2)){
				minLinked = mClientLinks[iter->clientNum];
				bestClient = iter->clientNum;
			}
			iter++;
		}
	}
	if(bestClient != -1){
		mClientLinks[bestClient]++;
	}
	return bestClient;
}

void DataServer::GetFileInfo(int fileId,double *bitRate,int *segNum){
	*bitRate = mFileInfo[fileId].bitRate;
	*segNum = mFileInfo[fileId].segNum;
}

void DataServer::InsertIntoIndex(int fileId,int segId,int clientNum,int linkedNum){
	DeleteFromIndex(fileId,segId,clientNum);

	FileInfoBlock fileInfoBlock;
//	fileInfoBlock.linkedNum = linkedNum;
	fileInfoBlock.clientNum = clientNum;
	mClientLinks[clientNum] = linkedNum;
	fileInfoBlock.segId = segId;

	mFileInfo[fileId].info.push_back(fileInfoBlock);
}

void DataServer::DeleteFromIndex(int fileId,int segId,int clientNum){
	list<FileInfoBlock>::iterator iter = mFileInfo[fileId].info.begin();
	while(iter != mFileInfo[fileId].info.end()){
		if(iter->segId == segId && iter->clientNum == clientNum){
			list<FileInfoBlock>::iterator tmpIter = iter;
			iter++;
			mFileInfo[fileId].info.erase(tmpIter);
			continue;
		}
		iter++;
	}
}

