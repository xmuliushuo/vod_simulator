#include <sys/epoll.h>
#include <sys/socket.h>

#include <sstream>
#include <fstream>
#include <cassert>

#include "clientmgr.h"
#include "message.h"
#include "poisson.h"
#include "zipf.h"
#include "spreadclient.h"
#include "timer.h"
#include "log.h"

ClientManager::ClientManager(ConfigType &config) : m_config(config)
{
	m_clientnum = config.clientNumber;
	m_poisson = new Poisson(config.poissonLambda / 1000.0);
	m_zipf = new Zipf(config.clientNumber, 
				config.resourceNumber, config.zipfParameter / 1000.0);
	m_clientnum = config.clientNumber;
	m_request_file = config.requestListFile;
	m_poisson_lambda = config.poissonLambda;
	m_zipf_sita = config.zipfParameter;
	m_epollfd = epoll_create(MAX_LISTEN_NUM);
	epoll_event ev;
	socketpair(AF_UNIX, SOCK_STREAM, 0, m_eventfd);
	ev.data.fd = m_eventfd[0];
	ev.events = EPOLLIN;
	epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_eventfd[0], &ev);

	socketpair(AF_UNIX, SOCK_STREAM, 0, m_exitfd);
	ev.data.fd = m_exitfd[0];
	ev.events = EPOLLIN;
	epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_exitfd[0], &ev);
}

ClientManager::~ClientManager()
{
	delete m_poisson;
	delete m_zipf;
}

void ClientManager::Run()
{
	vector<int> filelist;
	vector<int> interval;
	int index = m_request_file.find('.');
	ostringstream oss;
	oss << m_request_file.substr(0, index) << "_"
		<< m_zipf_sita << "_"
		<< m_poisson_lambda << "_"
		<< m_clientnum << m_request_file.substr(index);
	string request_file = oss.str();
	ifstream ifs(request_file.c_str());
	if (!ifs) {
		cout << request_file << " does not exist, create one" << endl;
		m_zipf->CreateZipfList(filelist);
		m_poisson->GetPoissonList(m_clientnum, interval);
		ofstream ofs(request_file.c_str());
		for(int i = 0; i < m_clientnum; ++i) {
			ofs << filelist.at(i) << " " << interval.at(i) << endl;
		}
		ofs.close();
	} else {
		int a, b;
		while (!ifs.eof()) {
			ifs >> a >> b;
			filelist.push_back(a);
			interval.push_back(b);
		}
		ifs.close();
	}

	for (int i = 0; i < m_clientnum; ++i) {
		Client *client = new SpreadClient(i);
		client->Init(m_config);
		client->SetFileID(filelist[i]);
		m_clientlist.push_back(client);
	}
	m_clientlist[0]->Run();
	TimerEvent timer_event;
	assert(m_clientnum > 1);
	timer_event.left_time = interval[1];
	timer_event.sockfd = m_eventfd[1];
	Timer::GetTimer()->RegisterTimer(timer_event);

	timer_event.left_time = (long long)m_config.runTime * 1000000;
	timer_event.sockfd = m_exitfd[1];
	Timer::GetTimer()->RegisterTimer(timer_event);

	char buffer[MESSAGELEN];
	int cur_clientid = 1;
	int nfds;
	int length;
	epoll_event events[MAX_LISTEN_NUM];
	while (true) {
		nfds = epoll_wait(m_epollfd, events, MAX_LISTEN_NUM, -1);
		for (int i = 0; i < nfds; ++i) {
			if (events[i].data.fd == m_eventfd[0]) {
				length = recv(events[i].data.fd, buffer, MESSAGELEN ,0);
				m_clientlist[cur_clientid]->Run();
				++cur_clientid;
				if (cur_clientid < m_clientnum) {
					timer_event.left_time = interval[cur_clientid];
					// timer_event.left_time = 100;
					timer_event.sockfd = m_eventfd[1];
					Timer::GetTimer()->RegisterTimer(timer_event);
				}
			} else if (events[i].data.fd == m_exitfd[0]) {
				recv(events[i].data.fd, buffer, MESSAGELEN ,0);
				LOG_INFO("Exit");
				goto out;
			}
			assert(length == MESSAGELEN);
		}
	}
out:
	return;
}
