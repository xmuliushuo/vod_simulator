#include "spreadclient.h"

#include "time.h"

#include <iostream>

#include "message.h"
#include "utils.h"
#include "log.h"

using namespace std;

SpreadClient::SpreadClient(int id) : Client(id)
{

}

bool SpreadClient::Init(ConfigType &config)
{
	m_serverip = config.serverIpAddress;
	m_serverport = config.serverPort;
	return true;
}
void SpreadClient::Run()
{
	LOG_INFO("Client " << m_clientid << " request file " << m_fileid);
	int sockfd;
	int len;
	Message message;
	message.type = RESOURCEREQUEST;
	message.clientId = m_clientid;
	message.serverId = LBSERVER;
	message.info.fileId = m_fileid;
	sockfd = TcpConnect(m_serverip.c_str(), m_serverport.c_str());
	if (writen(sockfd, (void*)&message, sizeof(message)) != sizeof(message)) {
		LOG_ERR("Write RESOURCEREQUST message error");
		exit(1);
	}
	if ((len = readn(sockfd, (void*)&message, sizeof(message))) != sizeof(message)) {
		LOG_ERR("Read RESOURCEREQUEST ACK message error");
		exit(1);
	}
	close(sockfd);
}
void SpreadClient::Exit()
{

}
