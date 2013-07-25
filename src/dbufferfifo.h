#ifndef DBUFFERFIFO_H_
#define DBUFFERFIFO_H_

#include "dbuffer.h"

#include <list>

using namespace std;

class DBufferFIFO : public DBuffer{
public:
	DBufferFIFO(int blockSize,int blockNum);
	virtual ~DBufferFIFO();
	virtual bool Read(int fileId,int segId);
	virtual void Write(int fileId,int segId,int &ofileId,int &osegId);
	virtual void Strategy(int fileId,int segId,int &ofileId,int &osegId);
	virtual bool FindBlock(int fileId,int segId);
protected:
	list<Block> m_blockList;
	int m_curBlockNum;
};


#endif /* DBUFFERFIFO_H_ */
