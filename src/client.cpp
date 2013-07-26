#include "client.h"

#include <pthread.h>
#include <sys/socket.h>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "utils.h"
#include "message.h"

Client::Client()
{

}

Client::~Client()
{

}

bool Client::Init(map<string, string> &config)
{
	if (config.find("p2p") == config.end()) return false;
	if (config.find("serverport") == config.end()) return false;
	if (config.find("blocksize") == config.end()) return false;
	if (config.find("blocknum") == config.end()) return false;
	if (config.find("serverip") == config.end()) return false;

	m_p2p = static_cast<bool>(atoi(config["p2p"].c_str()));
	m_server_port = atoi(config["serverport"].c_str());
	m_block_size = atoi(config["blocksize"].c_str());
	m_block_num = atoi(config["blocknum"].c_str());
	m_block_num = atoi(config["blocknum"].c_str());
	m_serverip = config["serverip"];

	return true;
}

void Client::Run()
{
	int socketid = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serveraddr;
	char message[MESSAGELEN];
	char buffer[MESSAGELEN];
	int *ptr = (int *)message;
	ptr[0] = MSG_CLIENT_JOIN;
	ptr[1] = 0;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//Inet_pton(AF_INET, m_serverip, &serveraddr.sin_addr);
	serveraddr.sin_addr.s_addr = inet_addr(m_serverip.c_str());
	serveraddr.sin_port = htons(m_server_port);
	Connect(socketid, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	send(socketid, message, MESSAGELEN, 0);
	while (true) {
		read(socketid, buffer, MESSAGELEN);
	}
}