#ifndef DBUFFER_H_
#define DBUFFER_H_

struct Block {
	Block();
	Block(int _fileid, int _segid): fileId(_fileid), segId(_segid), hitNew(1),
		hitOld(0), weight(0) {}
	int fileId;
	int segId;
	int hitNew;
	int hitOld;
	int weight;
};

class DBuffer{
public:
	DBuffer(int blocksize,int blocknum): 
		mBlockSize(blocksize), 
		mBlockNums(blocknum),
		m_isblockReset(false) {}
	virtual ~DBuffer() {}
	virtual bool Read(int fileid,int segid) = 0;
	virtual void Write(int fileid,int segid,int &ofileid,int &osegid) = 0;
	virtual void Strategy(int fileid,int segid,int &ofileid,int &osegid) = 0;
	inline bool IsBlockReset()
	{
		return m_isblockReset;
	}
	virtual void BlockReset() {}
	virtual bool FindBlock(int fileid,int segid) = 0;
	virtual void AddVistors(int fileid, int clientid) {}
	virtual void DeleteVistors(int fileid, int clientid) {}
protected:
	int mBlockSize;
	int mBlockNums;
	bool m_isblockReset;
};
#endif
