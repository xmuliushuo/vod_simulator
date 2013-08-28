#include "spreadserver.h"

#include <cassert>

#include "spreadstrategy.h"
#include "utils.h"
#include "log.h"

FileNode::FileNode()
{
	hitnew = 1;
	hitold = 0;
	vtime = getRelativeTime();
	status = NORMAL;
	load = 0;
}

SpreadServer::SpreadServer(int serverid, ConfigType &config):Server(serverid)
{
	mMaxLoad = config.maxLoad;
	mMaxCapacity = config.maxCapacity;
	mThreshold = config.loadThresh;
	mCurrentLoad = 0;
	mCurrentCapacity = 0;
	m_max_copy_flow = config.maxCopyFlow;
	m_load_thresh_high = config.loadThreshHigh;
	m_copy_flow = 0;
	m_max_in_flow = config.maxInFlow;
	m_in_flow = 0;
	assert(m_max_copy_flow >= 0);
	if (config.spreadAlgorithm == "DW")
		m_spread_strategy = new DWSpreadStrategy();
	else if (config.spreadAlgorithm == "LRU")
		m_spread_strategy = new LRUSpreadStrategy();
	else if (config.spreadAlgorithm == "LFU")
		m_spread_strategy = new LFUSpreadStrategy();
	else assert(0);
	LOG_INFO("SpreadServer::SpreadServer: server " << mServerId << " loadthresh " \
		<< mThreshold << " : " << m_load_thresh_high);	
}

SpreadServer::~SpreadServer()
{
	delete m_spread_strategy;
}

bool SpreadServer::Init(ConfigType &config)
{
	return true;
}


void SpreadServer::Run()
{
	
}


bool SpreadServer::AddFile(int fileid)
{
	LOG_INFO("SpreadServer::AddFile: server" << mServerId << " add file: " \
		<< fileid);
	assert(mCurrentCapacity < mMaxCapacity && mCurrentCapacity >= 0);
	FileNode newfile;
	newfile.fileid = fileid;
	m_filelist.push_back(newfile);
	++mCurrentCapacity;
	assert((unsigned int)mCurrentCapacity == m_filelist.size());
	return true;
}

int SpreadServer::IncreaseLoad(int fileid)
{
	++mCurrentLoad;
	assert(mCurrentLoad <= mMaxLoad);
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	++iter->load;
	m_spread_strategy->ReadFile(*iter, getRelativeTime());
	return mCurrentLoad;
}

bool SpreadServer::SearchFile(int fileid)
{
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) return true;
	}
	return false;
}

int SpreadServer::GetSpreadFile(vector<int> &output)
{
	return m_spread_strategy->GetSpreadFile(m_filelist, output);
}

bool SpreadServer::IsOverCapacity()
{
	return (mCurrentCapacity >= mMaxCapacity);
}

bool SpreadServer::IsNeedSpread()
{
	vector<FileNode>::iterator iter;
	int spreaded_load = 0;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->status == SPREADED) {
			spreaded_load += iter->load;
		}
	}
	double threshhold = (mCurrentLoad - spreaded_load) * 1.0 
		/ (mMaxLoad - spreaded_load);
	LOG_INFO("SpreadServer::IsNeedSpread: threshhold: " << threshhold \
		<< ", copy flow: " << m_copy_flow << ", lowthreshold: " << mThreshold \
		<< ", highthreshold: " << m_load_thresh_high);
	if (threshhold >= mThreshold && 
		(m_copy_flow < m_max_copy_flow) && 
		(threshhold <= m_load_thresh_high))
		return true;
	return false;
}

int SpreadServer::GetCurrentLoad()
{
	return mCurrentLoad;
}

bool SpreadServer::IsOverLoad()
{
	return mCurrentLoad >= mMaxLoad;
}

int SpreadServer::DecreaseLoad(int fileid)
{
	assert(mCurrentLoad <= mMaxLoad && mCurrentLoad > 0);
	--mCurrentLoad;
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	return mCurrentLoad;
}

void SpreadServer::Reset()
{
	m_spread_strategy->Reset(m_filelist);
}

int SpreadServer::IncreaseCopyFlow(int fileid)
{
	assert(m_copy_flow < m_max_copy_flow);
	++m_copy_flow;
	//++mCurrentLoad;
	vector<FileNode>::iterator iter;
	for (iter = m_filelist.begin(); iter != m_filelist.end(); ++iter) {
		if (iter->fileid == fileid) break;
	}
	assert(iter != m_filelist.end());
	iter->status = SPREADED;
	return m_copy_flow;
}

int SpreadServer::DecreaseCopyFlow()
{
	assert(m_copy_flow <= m_max_copy_flow && m_copy_flow > 0);
	--m_copy_flow;
	//--mCurrentLoad;
	return m_copy_flow;
}

int SpreadServer::GetCopyFlow()
{
	return m_copy_flow;
}

int SpreadServer::IncreaseInFlow()
{
	++mCurrentLoad;
	++m_in_flow;
	assert(mCurrentLoad <= mMaxLoad);
	return mCurrentLoad;
}

int SpreadServer::DecreaseInFlow()
{
	--mCurrentLoad;
	--m_in_flow;
	assert(mCurrentLoad >= 0);
	return mCurrentLoad;
}

bool SpreadServer::IsCanBeTarget()
{
	if (IsOverLoad()) return false;
	if (IsOverCapacity()) return false;
	if (m_in_flow >= m_max_in_flow) return false;
	double threshold = mCurrentLoad * 1.0 / mMaxLoad;	
	if (threshold >= m_load_thresh_high) return false;
	return true;
}
