#include "client.h"

#include <pthread.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "utils.h"
#include "message.h"
#include "timer.h"
#include "dbuffer.h"
#include "dbufferdw.h"
#include "dbufferfifo.h"

void *ThreadOfClient_(void *arg){
	Client *client = (Client *)arg;
	client->Run();
	return NULL;
}

Client::Client(int id): m_id(id)
{

}

Client::~Client()
{
	delete m_buffer;
}

bool Client::Init(map<string, string> &config)
{

	m_epoll_fd = epoll_create(MAX_LISTEN_NUM);
	epoll_event ev;

	if (config.find("p2p") == config.end()) return false;
	if (config.find("serverport") == config.end()) return false;
	if (config.find("blocksize") == config.end()) return false;
	if (config.find("blocknum") == config.end()) return false;
	if (config.find("serverip") == config.end()) return false;
	if (config.find("clientstrategy") == config.end()) return false;
	if (config.find("period") == config.end()) return false;

	m_p2p = static_cast<bool>(atoi(config["p2p"].c_str()));
	m_server_port = atoi(config["serverport"].c_str());
	m_block_size = atoi(config["blocksize"].c_str());
	m_block_num = atoi(config["blocknum"].c_str());
	m_block_num = atoi(config["blocknum"].c_str());
	m_serverip = config["serverip"];

	if (config["clientstrategy"] == "fifo")
		m_buffer = new DBufferFIFO(m_block_size, m_block_num);
	else if (config["clientstrategy"] == "dw")
		m_buffer = new DBufferDW(m_block_size, m_block_num, m_period);
	else assert(0);

	socketpair(AF_UNIX, SOCK_STREAM, 0, m_buffer_reset_fd);
	ev.data.fd = m_buffer_reset_fd[0];
	ev.events = EPOLLIN;
	epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_buffer_reset_fd[0], &ev);

	return true;
}

void Client::Run()
{
	pthread_t tid;
	Pthread_create(&tid, NULL, ThreadOfClient_, this);
}

void Client::BufferReset() 
{
	cout << "buffer reset" << endl;
	TimerEvent event;
	event.sockfd = m_buffer_reset_fd[1];
	event.left_time = m_period * 1000000;
	Timer::GetTimer()->RegisterTimer(event);
	m_buffer->BlockReset();
}

void Client::ThreadOfClient()
{
	int socketid = Socket(AF_INET, SOCK_STREAM, 0);
	int length;
	struct sockaddr_in serveraddr;
	char buffer[MESSAGELEN];
	char response[MESSAGELEN];
	int *ptr = (int *)buffer;
	int *resptr = (int *)response;
	epoll_event ev;
	epoll_event events[MAX_LISTEN_NUM];
	int nfds;
	TimerEvent event;
	int i;

	ptr[0] = MSG_CLIENT_JOIN;
	ptr[1] = m_id;
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(m_serverip.c_str());
	serveraddr.sin_port = htons(m_server_port);
	Connect(socketid, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	send(socketid, buffer, MESSAGELEN, 0);

	ev.data.fd = socketid;
	ev.events = EPOLLIN;
	epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, socketid, &ev);

	if(m_buffer->IsBlockReset()){
		event.sockfd = m_buffer_reset_fd[1];
		event.left_time = m_period * 1000000;
		Timer::GetTimer()->RegisterTimer(event);
	}	

	while (true) {
		nfds = epoll_wait(m_epoll_fd, events, MAX_LISTEN_NUM, -1);
		length = read(socketid, buffer, MESSAGELEN);
		assert(length == MESSAGELEN);
		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == m_buffer_reset_fd[0]) {
				length = recv(events[i].data.fd, buffer, MESSAGELEN ,0);
				BufferReset();
			}
			assert(length == MESSAGELEN);
		}
	}
}