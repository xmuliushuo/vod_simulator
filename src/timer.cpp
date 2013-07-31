#include "timer.h"

#include <sys/time.h>

#include <cassert>
#include <iostream>

#include "utils.h"

static void* _TimerThread(void *arg)
{
	((Timer *)arg)->TimerThread();
	return NULL;
}

Timer::Timer()
{
	pthread_t tid;
	pthread_mutex_init(&m_timer_mutex, NULL);
	pthread_cond_init(&m_timer_cond, NULL);
	Pthread_create(&tid, NULL, _TimerThread, this);
}

Timer::~Timer()
{
	// TODO
}

void Timer::WakeTimer(TimerEvent &event)
{
	int ret = write(event.sockfd, event.buffer, MESSAGELEN);
	assert(ret == MESSAGELEN);
}

void Timer::RegisterTimer(TimerEvent &event){
	pthread_mutex_lock(&m_timer_mutex);
	assert(event.left_time >= 0);
	m_timerlist.push_back(event);
	pthread_cond_signal(&m_timer_cond);
	pthread_mutex_unlock(&m_timer_mutex);
}

void Timer::TimerThread()
{
	int sleep_time = -1;
	long long min_left_time;
	list<TimerEvent>::iterator iter, miniter;
	timeval begin_time, end_time;
	timespec tm;
	int temp;
	while (true) {
		pthread_mutex_lock(&m_timer_mutex);
		if (!m_timerlist.empty()) {
			if (sleep_time != -1) {
				for (iter = m_timerlist.begin(); iter != m_timerlist.end();) {
					iter->left_time -= sleep_time;
					if (iter->left_time < 0) {
						WakeTimer(*iter);
						iter = m_timerlist.erase(iter);
					}
					else {
						++iter;
					}
				}
			}
		}
		if (!m_timerlist.empty()) {
			min_left_time = m_timerlist.begin()->left_time;
			for (iter = m_timerlist.begin(); iter != m_timerlist.end(); ++iter) {
				if (iter->left_time < min_left_time) {
					min_left_time = iter->left_time;
					miniter = iter;
				}
			}
			gettimeofday(&begin_time, NULL);
			temp = min_left_time;
			tm.tv_nsec = (temp % 1000000 + begin_time.tv_usec) * 1000;
			tm.tv_sec = (temp) / 1000000 + begin_time.tv_sec;
			tm.tv_sec = tm.tv_nsec / 1000000000 + tm.tv_sec;
			tm.tv_nsec = tm.tv_nsec % 1000000000;
			pthread_cond_timedwait(&m_timer_cond, &m_timer_mutex, &tm);
			gettimeofday(&end_time, NULL);
			sleep_time = (end_time.tv_sec - begin_time.tv_sec) * 1000000
				+ end_time.tv_usec - begin_time.tv_usec;
			assert(sleep_time >= 0);
		}
		else {
			sleep_time = -1;
			pthread_cond_wait(&m_timer_cond, &m_timer_mutex);
		}
		pthread_mutex_unlock(&m_timer_mutex);
	}
}

#if 0
#include <sys/epoll.h>

#define THREAD_NUM 10

int startTime = 1000000;
int epollfd;
Timer timer;

void *ThreadToRegister(void *arg){
	TimerEvent event;
	event.isnew = true;
	event.left_time = *((int *)arg);
	cout << event.left_time << endl;

	int sockfd[2];
	socketpair(AF_UNIX,SOCK_STREAM,0,sockfd);

	event.sockfd = sockfd[1];
	epoll_event ev;
	ev.data.fd = sockfd[0];
	ev.events = EPOLLIN;

	epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd[0],&ev);
	timer.RegisterTimer(event);
//	cout << sockfd[0] << " has regishter" << endl;
}

int main(){

	epollfd = epoll_create(10);
	epoll_event events[10];

	pthread_t tid[THREAD_NUM];
	int sleepTime[THREAD_NUM];
	for(int i = 0;i < THREAD_NUM;i++){
		sleepTime[i] = startTime * (i + 1);
		pthread_create(&tid[i],NULL,ThreadToRegister,&(sleepTime[i]));
	}

	while(true){
		int fds = epoll_wait(epollfd,events,10,-1);
		for(int i = 0;i < fds;i++){
			char nullChar;
			read(events[i].data.fd,&nullChar,1);
			cout << events[i].data.fd << " wake up from timer" << endl;
		}
	}

	for(int i = 0;i < THREAD_NUM;i++)
		pthread_join(tid[i],NULL);

	return 0;
}

#endif