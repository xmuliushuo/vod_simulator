#include "spreadstrategy.h"

#include <algorithm>

bool LFUCompare(FileNode a, FileNode b)
{
	if (a.hitnew > b.hitnew) return true;
	else if (a.hitnew == b.hitnew && a.fileid < b.fileid) return true;
	else return false;
}

int LFUSpreadStrategy::GetSpreadFile(vector<FileNode> &filelist, vector<int> &output) 
{
	vector<FileNode>::iterator iter;
	sort(filelist.begin(), filelist.end(), LFUCompare);
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		if (iter->status == NORMAL) {
			output.push_back(iter->fileid);
		}
	}
	return 0;
}

void LFUSpreadStrategy::ReadFile(FileNode &file, double vtime) 
{
	++file.hitnew;
}

bool LRUCompare(FileNode a, FileNode b)
{
	if (a.vtime > b.vtime) return true;
	else if (a.vtime == b.vtime && a.fileid < b.fileid) return true;
	else return false;
}

int LRUSpreadStrategy::GetSpreadFile(vector<FileNode> &filelist, vector<int> &output) 
{
	vector<FileNode>::iterator iter;
	sort(filelist.begin(), filelist.end(), LRUCompare);
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		if (iter->status == NORMAL)
			output.push_back(iter->fileid);
	}
	return 0;
}

void LRUSpreadStrategy::ReadFile(FileNode &file, double vtime) 
{
	file.vtime = vtime;
}

bool DWCompare(FileNode a, FileNode b)
{
	if (a.weigth > b.weigth) return true;
	else if (a.weigth == b.weigth && a.fileid < b.fileid) return true;
	else return false;
}

void DWSpreadStrategy::ReadFile(FileNode &file, double vtime)
{
	++file.hitnew;
	file.vtime = vtime;
}

void DWSpreadStrategy::Reset(vector<FileNode> &filelist)
{
	vector<FileNode>::iterator iter;
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		iter->hitold = iter->hitnew;
		iter->hitnew = 0;
	}
}

int DWSpreadStrategy::GetSpreadFile(vector<FileNode> &filelist, vector<int> &output)
{
	vector<FileNode>::iterator iter;
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		iter->weigth = iter->hitnew + iter->hitold;
	}
	sort(filelist.begin(), filelist.end(), DWCompare);
	for (iter = filelist.begin(); iter != filelist.end(); ++iter) {
		if (iter->status == NORMAL)
			output.push_back(iter->fileid);
	}
	return 0;
}
