#ifndef SPREADCLIENT_H
#define SPREADCLIENT_H

#include "client.h"

class SpreadClient : public Client {
public:
	SpreadClient(int id);
	~SpreadClient() {}

	virtual bool Init(ConfigType &);
	// 向服务器发送视频文件请求信息，结束
	virtual void Run();
	virtual void Exit();
private:
	string m_serverip;
	string m_serverport;
};

#endif