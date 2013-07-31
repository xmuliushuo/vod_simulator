#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include <map>

using namespace std;

class Client {
public:
	Client(int);
	~Client();
	bool Init(map<string, string> &);
	void Run();
private:
	int m_id;
	bool m_p2p;
	int m_server_port;
	string m_serverip;
	int m_block_size;
	int m_block_num;
	string m_buffer_strategy;
};

#endif