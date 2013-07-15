#ifndef SERVER_H_
#define SERVER_H_

#include <map>
#include <string>

class Server {
public:
	Server();
	~Server();
	bool Init(std::map<std::string, std::string> &);
	void Run();
private:
	bool p2p_;
};

#endif