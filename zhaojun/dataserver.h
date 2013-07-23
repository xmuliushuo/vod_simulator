/*
 * DataServer.h
 *
 *  Created on: 2013-2-20
 *      Author: zhaojun
 */

#ifndef DATASERVER_H_
#define DATASERVER_H_

#include <list>

#include "mymessage.h"

using namespace std;

struct FileInfoBlock{
	int segId;
//	double linkedNum;
	int clientNum;
};

struct DataBlock{
	int fileId;
	int segNum;
	double bitRate;
	list<FileInfoBlock> info;
};

class DataServer{
public:
	DataServer(int fileNum,int minLength,int maxLength,int blockSize,double minBitRate,double maxBitRate);
	~DataServer();
	int SearchBestClient(int fileId,int segId);
	void GetFileInfo(int fileId,double *bitRate,int *segNum);
	void InsertIntoIndex(int fileId,int segId,int clientNum,int linkedNum);
	void DeleteFromIndex(int fileId,int segId,int clientNum);
private:
	DataBlock mFileInfo[MAX_FILE_NUM + 1];
	int mClientLinks[MAX_CLIENT_NUM + 1];
	int mFileNum;
};


#endif /* DATASERVER_H_ */
