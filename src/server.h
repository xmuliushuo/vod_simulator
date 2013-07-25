#ifndef SERVER_H_
#define SERVER_H_

#include <netinet/in.h>

#include <string>
#include <map>

using namespace std;

class Server;

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
	void ThreadPerClient();
private:
	bool m_p2p;
	int m_port;
	int m_block_size;
	int m_block_num;
	string m_buffer_strategy;
};

#endif