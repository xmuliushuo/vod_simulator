#include "server.h"

#include <pthread.h>
#include <sys/socket.h>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "utils.h"
#include "message.h"

void *ThreadPerClient_(void *arg)
{
	Pthread_detach(pthread_self());
	struct RequestArgs *ptr = (struct RequestArgs *)arg;
	ptr->server->ThreadPerClient(ptr->connfd);
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
		connfd = Accept(socketid, 
			            (struct sockaddr *)&(args->cliaddr), 
			            &clilen);
		args->server = this;
		args->connfd = connfd;
		Pthread_create(&tid, NULL, ThreadPerClient_, args);
	}
}

void Server::ThreadPerClient(int connfd)
{
	int length;
	char buffer[MESSAGELEN];
	char response[MESSAGELEN];
	int *ptr, *resptr;
	int clientid;
	while (true) {
		length = read(connfd, buffer, MESSAGELEN);
		assert(length == MESSAGELEN);
		ptr = (int *)buffer;
		switch(ptr[0]) {
		case MSG_CLIENT_JOIN:
			clientid = ptr[1];
			cout << "server receive MSG_CLIENT_JOIN from " << clientid <<
				" and response MSG_JOIN_ACK" << endl;
			resptr = (int *)response;
			resptr[0] = MSG_JOIN_ACK;
			resptr[1] = 0;
			length = send(connfd, response, MESSAGELEN, 0);
			assert(length == MESSAGELEN);
		}
	}
}