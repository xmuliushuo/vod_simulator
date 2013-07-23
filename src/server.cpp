#include "server.h"

Server::Server()
{

}

Server::~Server()
{

}

bool Server::Init(map<string, string> &config)
{
	if (config.find("p2p") == config.end()) return false;
	p2p_ = static_cast<bool> atoi(config["p2p"].c_str());
	if (config.find("serverport") == config.end()) return false;
	port_ = atoi(config["serverport"].c_str());
}

void Server::Run()
{
	
}