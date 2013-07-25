#ifndef DBUFFERDW_H
#define DBUFFERDW_H

#include "dbuffer.h"

#include <map>
#include <set>
#include <list>

using namespace std;

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
	list<Block> mDWQueue;
	map<int, set<int> > mFileVistors;
};

#endif
