#ifndef SPREADSTRATEGY_H_
#define SPREADSTRATEGY_H_

#include "spreadserver.h"

class SpreadStrategy {
public:
	virtual void ReadFile(FileNode &file, double vtime) {}
	virtual void Reset(vector<FileNode> &filelist) {}
	virtual int GetSpreadFile(vector<FileNode> &filelist, vector<int> &output) { return 0; }
};

class DWSpreadStrategy : public SpreadStrategy {
public:
	virtual void ReadFile(FileNode &file, double vtime);
	virtual void Reset(vector<FileNode> &filelist);
	virtual int GetSpreadFile(vector<FileNode> &filelist, vector<int> &output);
};

class LRUSpreadStrategy : public SpreadStrategy {
public:
	virtual void ReadFile(FileNode &file, double vtime);
	virtual int GetSpreadFile(vector<FileNode> &filelist, vector<int> &output);
};

class LFUSpreadStrategy : public SpreadStrategy {
public:
	virtual void ReadFile(FileNode &file, double vtime);
	virtual int GetSpreadFile(vector<FileNode> &filelist, vector<int> &output);
};

#endif