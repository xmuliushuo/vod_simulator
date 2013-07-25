#include "server.h"

#include <pthread.h>
#include <sys/socket.h>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "vod.h"

void *ThreadPerClient_(void *arg)
{
	Pthread_detach(pthread_self());
	struct RequestArgs *ptr = (struct RequestArgs *)arg;
	ptr->server->ThreadPerClient();
	free(ptr);
	return (void *)NULL;
}

Server::Server()
{

}

Server::~Server()
{

}

bool Server::Init(map<string, string> &config)
{
	if (config.find("p2p") == config.end()) return false;
	if (config.find("serverport") == config.end()) return false;
	if (config.find("blocksize") == config.end()) return false;
	if (config.find("blocknum") == config.end()) return false;
	if (config.find("serverstrategy") == config.end()) return false;

	m_p2p = static_cast<bool>(atoi(config["p2p"].c_str()));
	m_port = atoi(config["serverport"].c_str());
	m_block_size = atoi(config["blocksize"].c_str());
	m_block_num = atoi(config["blocknum"].c_str());
	// if (config["serverstrategy"] == "fifo")
	// 	buffer_ = new DbufferFIFO();
	// else assert(0);
	return true;
}

void Server::Run()
{
	int socketid = Socket(AF_INET, SOCK_STREAM, 0);
	int connfd;
	pthread_t tid;
	struct sockaddr_in serveraddr;
	socklen_t clilen;

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(m_port);
	Bind(socketid, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	Listen(socketid, 3);
	struct RequestArgs* args;

	while (true) {
		args = (struct RequestArgs *)Malloc(sizeof(struct RequestArgs));
		clilen = sizeof(args->cliaddr);
		connfd = Accept(socketid, (struct sockaddr *)&(args->cliaddr), &clilen);
		args->server = this;
		args->connfd = connfd;
		Pthread_create(&tid, NULL, ThreadPerClient_, args);
	}
}

void Server::ThreadPerClient()
{

}