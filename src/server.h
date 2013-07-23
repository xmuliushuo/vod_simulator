#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <string>

using namespace std;

class Server {
public:
	Server();
	~Server();
	bool Init(map<string, string> &);
	void Run();
private:
	bool p2p_;
	int port_;
};

#endif