#ifndef SPREADSERVER_H_
#define SPREADSERVER_H_

#include <vector>

#include "server.h"

using namespace std;

class SpreadStrategy;

const int NORMAL = 0;
const int SPREADED = 1;

typedef struct FileNode {
	FileNode();
	unsigned int mServerId;//服务器id
	int fileid;//文件id
	bool  hadSpread;//文件是否
	// unsigned int mReqClients;
	unsigned int mCurLoad;//文件负载
	double vtime;
	int hitold;
	int hitnew;
	int weigth;
	int status;
	int load;
} FileNode;

class SpreadServer : public Server
{
public:
	SpreadServer(int serverid) : Server(serverid){ };
	SpreadServer(int serverid, ConfigType &config);
	~SpreadServer();

	bool Init(ConfigType &);	// 读取配置，初始化扩散服务器
	void Run();
	int IncreaseLoad(int fileid);
	int DecreaseLoad(int fileid);
	int IncreaseInFlow();
	int DecreaseInFlow();
	int IncreaseCopyFlow(int fileid);
	int DecreaseCopyFlow();
	int GetCopyFlow();	
	bool IsOverLoad();
	int GetCurrentLoad();		//获取当前负载
	int GetSpreadFile(vector<int> &output);
	// 获取当前扩散负载
	// 对其中的每个文件检查是否有扩散，然后把每个文件的当前负载相加
	// int GetSpreadedLoad();
	bool IsNeedSpread();		//是否需要扩散，由lbserver调用扩散服务器的
	bool AddFile(int fileid);	// 添加文件
	bool SearchFile(int fileid);
	bool IsOverCapacity();		// 是否超容量了
	void Reset();				// 重置文件统计信息，一些周期算法需要
	bool IsCanBeTarget();
private:
	int mMaxLoad;//最大负载
	int mMaxCapacity;//容量
	double mThreshold;//复制阈值
	double m_load_thresh_high;
	int mCurrentLoad;//当前负载
	int mCurrentCapacity;//当前容量
	int m_max_copy_flow;	// 最大复制流
	int m_copy_flow;		// 当前复制流
	int m_max_in_flow;
	int m_in_flow;
	// pthread_mutex_t mMutex;//锁
	vector<FileNode> m_filelist;
	SpreadStrategy *m_spread_strategy;
};



#endif
