#include "client.h"

#include <pthread.h>
#include <sys/socket.h>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "utils.h"
#include "message.h"

Client::Client(int id): m_id(id)
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
	int length;
	struct sockaddr_in serveraddr;
	char buffer[MESSAGELEN];
	char response[MESSAGELEN];
	int *ptr = (int *)buffer;
	int *resptr = (int *)response;
	ptr[0] = MSG_CLIENT_JOIN;
	ptr[1] = m_id;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(m_serverip.c_str());
	serveraddr.sin_port = htons(m_server_port);
	Connect(socketid, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	send(socketid, buffer, MESSAGELEN, 0);
	while (true) {
		length = read(socketid, buffer, MESSAGELEN);
		assert(length == MESSAGELEN);
		switch (ptr[0]) {
		case MSG_JOIN_ACK:
			resptr[0] = MSG_SEG_ASK;
			resptr[0] = m_id;
			break;
		}
	}
}