#ifndef SPREADCOMM_H
#define SPREADCOMM_H

class LbServer;

class SpreadComm
{
public:
	SpreadComm();
	~SpreadComm();

	// 消息处理
	void MessageProcess(int connfd, void *buf, size_t bufLen);
	/* data */
public:
	LbServer *lbserver;
};




#endif