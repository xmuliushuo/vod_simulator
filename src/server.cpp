#include "server.h"

#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cstdlib>
#include <cstring>

#include "vod.h"

Server::Server()
{

}

Server::~Server()
{

}

bool Server::Init(map<string, string> &config)
{
	if (config.find("p2p") == config.end()) return false;
	p2p_ = static_cast<bool>(atoi(config["p2p"].c_str()));
	if (config.find("serverport") == config.end()) return false;
	port_ = atoi(config["serverport"].c_str());
	return true;
}

void Server::Run()
{
	int socketid = Socket(AF_INET, SOCK_STREAM, 0);
	int connfd;
	pthread_t tid;
	struct sockaddr_in serveraddr;

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(port_);
	Bind(socketid, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	Listen(socketid, 3);

	while (true) {
		//connfd = Accept(socketID, (struct sockaddr *)&(args->cliaddr), &clilen);
	}

    // socklen_t clilen;
    // struct requestArgs* args;
    // while (1)
    // {
    //     args = (struct requestArgs *)Malloc(sizeof(struct requestArgs));
    //     clilen = sizeof(args->cliaddr);
    //     connfd = Accept(socketID, (struct sockaddr *)&(args->cliaddr), &clilen);
    //     args->topNode = this;
    //     args->connfd = connfd;
    //     Pthread_create(&tid, NULL, thread_handleMesg, args);
    // }
    // return (void *)NULL;
}
