#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include <map>

#include "facktran.h"

using namespace std;

class DBuffer;

class Client {
public:
	Client(int);
	~Client();
	bool Init(map<string, string> &);
	void Run();
	void ThreadOfClient();
private:
	void BufferReset();
	int m_id;
	bool m_p2p;
	int m_server_port;
	string m_serverip;
	int m_block_size;
	int m_block_num;
	int m_epoll_fd;
	int m_buffer_reset_fd[2];
	int m_period;
	int m_fileid;
	int m_segid;
	DBuffer *m_buffer;
	string m_buffer_strategy;
	FakeTran m_faketran;
};

#endif