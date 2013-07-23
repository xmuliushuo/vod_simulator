#ifndef DBUFFERFIFOS_H
#define DBUFFERFIFOS_H

#include "dbuffer.h"

#include <map>
#include <set>

using namespace std;

class FIFOSBlockInfo {
public:
	FIFOSBlockInfo();
	FIFOSBlockInfo(int _fileId, int _segId): fileId(_fileId), segId(_segId), weight(0) {};
	int fileId;
	int segId;
	int weight;
};

class DBufferFIFOS: public DBuffer {
public:
	DBufferFIFOS(int blockSize, int blockNum);
	virtual ~DBufferFIFOS();
	virtual bool Read(int fileId, int segId);
	virtual void Write(int fileId, int segId, int &ofileId, int &osegId);
	virtual void Strategy(int fileId, int segId, int &ofileId, int &osegId);
	virtual void BlockReset();
	virtual bool FindBlock(int fileId,int segId);
	virtual void AddVistors(int fileId, int clientID);
	virtual void DeleteVistors(int fileId, int clientID);
private:
	void AddBlock(int fileId, int segId);
	int GetVistorNum(int fileId);
	list<FIFOSBlockInfo> mFIFOQueue;
	//int mBlockNum;
	map<int, set<int> > mFileVistors;
};

#endif