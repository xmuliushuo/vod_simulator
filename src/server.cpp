#include "server.h"

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

void *ThreadPerClient_(void *arg)
{
	Pthread_detach(pthread_self());
	struct RequestArgs *ptr = (struct RequestArgs *)arg;
	ptr->server->ThreadPerClient(ptr->connfd);
	free(ptr);
	return (void *)NULL;
}

void *ThreadEvent_(void *arg)
{
	Pthread_detach(pthread_self());
	((Server *)arg)->ThreadEvent();
	return (void *)NULL;
}

Server::Server()
{
}

Server::~Server()
{
	delete m_buffer;
}

bool Server::Init(map<string, string> &config)
{
	m_event_fd = epoll_create(MAX_LISTEN_NUM);
	epoll_event ev;
	
	if (config.find("p2p") == config.end()) return false;
	if (config.find("serverport") == config.end()) return false;
	if (config.find("blocksize") == config.end()) return false;
	if (config.find("blocknum") == config.end()) return false;
	if (config.find("serverstrategy") == config.end()) return false;
	if (config.find("period") == config.end()) return false;

	m_p2p = static_cast<bool>(atoi(config["p2p"].c_str()));
	m_port = atoi(config["serverport"].c_str());
	m_block_size = atoi(config["blocksize"].c_str());
	m_block_num = atoi(config["blocknum"].c_str());
	m_period = atoi(config["period"].c_str());
	if (config["serverstrategy"] == "fifo")
		m_buffer = new DBufferFIFO(m_block_size, m_block_num);
	else if (config["serverstrategy"] == "dw")
		m_buffer = new DBufferDW(m_block_size, m_block_num, m_period);
	else assert(0);

	socketpair(AF_UNIX, SOCK_STREAM, 0, m_buffer_reset_fd);
	ev.data.fd = m_buffer_reset_fd[0];
	ev.events = EPOLLIN;
	epoll_ctl(m_event_fd, EPOLL_CTL_ADD, m_buffer_reset_fd[0], &ev);
	return true;
}

void Server::Run()
{
	int socketid = Socket(AF_INET, SOCK_STREAM, 0);
	int connfd;
	pthread_t tid;
	struct sockaddr_in serveraddr;
	socklen_t clilen;
	TimerEvent timer_event;

	Pthread_create(&tid, NULL, ThreadEvent_, this);

	if(m_buffer->IsBlockReset()) {
		timer_event.sockfd = m_buffer_reset_fd[1];
		timer_event.left_time = m_period * 1000000;
		Timer::GetTimer()->RegisterTimer(timer_event);
	}

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
			break;
		}
	}
}

void Server::ThreadEvent()
{
	epoll_event events[MAX_LISTEN_NUM];
	int nfds;
	int i;
	char buffer[20];
	int length;
	while(true) {
		nfds = epoll_wait(m_event_fd, events, MAX_LISTEN_NUM, -1);
		for(i = 0; i < nfds; ++i) {
			if (events[i].data.fd == m_buffer_reset_fd[0]) {
				length = recv(events[i].data.fd, buffer, 20, 0);
				assert(length == 20);
				BufferReset();
			}
			else {
				assert(0);
			}
		}
	}
}

void Server::BufferReset() 
{
	cout << "buffer reset" << endl;
	TimerEvent event;
	event.sockfd = m_buffer_reset_fd[1];
	event.left_time = m_period * 1000000;
	Timer::GetTimer()->RegisterTimer(event);
	m_buffer->BlockReset();
}