#ifndef DBUFFERDW_H
#define DBUFFERDW_H

#include "dbuffer.h"

#include <map>
#include <set>

using namespace std;

class DWBlockInfo {
public:
	DWBlockInfo();
	DWBlockInfo(int _fileId, int _segId): hitNew(1), hitOld(0),weight(0),fileId(_fileId), segId(_segId){};
	int hitNew;
	int hitOld;
	int weight;
	int fileId;
	int segId;
};

class DBufferDW: public DBuffer {
public:
	DBufferDW(int blockSize, int blockNum, int period);
	virtual ~DBufferDW();
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
	list<DWBlockInfo> mDWQueue;
	map<int, set<int> > mFileVistors;
};

#endif
