#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>

#include <string>
#include <map>

using namespace std;

class Server;
class DBuffer;

struct RequestArgs{
	Server *server;     /**< 指向Server */
	struct sockaddr_in cliaddr; /**< 客户端的地址结构*/
	int connfd;                 /**< 客户端的socket套接字*/
};

class Server {
public:
	Server();
	~Server();
	bool Init(map<string, string> &);
	void Run();
	void ThreadPerClient(int connfd);
	void ThreadEvent();
private:
	void BufferReset();
	bool m_p2p;
	int m_port;
	int m_block_size;
	int m_block_num;
	int m_event_fd;
	int m_buffer_reset_fd[2];
	int m_period;
	DBuffer *m_buffer;
	string m_buffer_strategy;
};

#endif