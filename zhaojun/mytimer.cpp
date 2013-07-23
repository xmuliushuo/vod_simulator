/*
 * mytimer.cpp
 *
 *  Created on: 2013-2-19
 *      Author: zhaojun
 */

#include "mytimer.h"
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <pthread.h>

#include <iostream>

#include "myheap.cpp"

using namespace std;

void *ThreadToTimer(void *arg){
	MyTimer *mytimer = (MyTimer *)arg;
	mytimer->Run();
	return NULL;
}

MyTimer::MyTimer(int multiple){
	m_multiple = multiple;
	pthread_mutex_init(&m_vectMutex,NULL);
	pthread_cond_init(&m_vectCond,NULL);

	pthread_create(&m_tid,NULL,ThreadToTimer,this);
}

MyTimer::~MyTimer(){
	pthread_mutex_destroy(&m_vectMutex);
	pthread_cond_destroy(&m_vectCond);

	pthread_cancel(m_tid);
}

void MyTimer::RegisterTimer(TimerEvent event){
	pthread_mutex_lock(&m_vectMutex);
	event.leftTime = event.leftTime * m_multiple;
	m_heap.PushHeap(event);
	pthread_cond_signal(&m_vectCond);
	pthread_mutex_unlock(&m_vectMutex);
}

void MyTimer::WakeTimer(TimerEvent event){
//	char nullChar = '\0';
	write(event.sockfd,event.buffer,20);
}

void MyTimer::CancelTimer(TimerEvent event){
	//暂不提供
}

void MyTimer::Run(){
	int hasSleepTime = -1;
	while(true){
		pthread_mutex_lock(&m_vectMutex);
		if(!m_heap.IsEmpty()){
			for(int i = m_heap.Size() - 1;i >= 0;i--){
				if(!m_heap.GetElement(i).isNew && hasSleepTime != -1){
//					cout << "element i lefttime:" << m_heap.GetElement(i).leftTime << ",hasleeptime:"
//							<< hasSleepTime << endl;
					m_heap.GetElement(i).leftTime -= hasSleepTime;
					if(m_heap.GetElement(i).leftTime <= 0){
						WakeTimer(m_heap.GetElement(i));
						m_heap.PopHeap(i);
						if(i < m_heap.Size()){
							if(m_heap.GetElement(i).isNew){
								m_heap.GetElement(i).isNew = false;
								cout << "bad thing happen" << endl;
							}
						}
					}
				}
				else
					m_heap.GetElement(i).isNew = false;
			}
			m_heap.AdjustHeap();
			TimerEvent event = m_heap.Front();
			timespec tm;
			timeval beginTime;
			timeval endTime;
			gettimeofday(&beginTime,NULL);
			int sleepTime = event.leftTime;
			tm.tv_nsec = (sleepTime % 1000000 + beginTime.tv_usec) * 1000;
			tm.tv_sec = (sleepTime) / 1000000 + beginTime.tv_sec;
			tm.tv_sec = tm.tv_nsec / 1000000000 + tm.tv_sec;
			tm.tv_nsec = tm.tv_nsec % 1000000000;
			pthread_cond_timedwait(&m_vectCond,&m_vectMutex,&tm);
			gettimeofday(&endTime,NULL);
			hasSleepTime = (endTime.tv_sec - beginTime.tv_sec) * 1000000
					+ endTime.tv_usec - beginTime.tv_usec;

		}
		else{
			pthread_cond_wait(&m_vectCond,&m_vectMutex);
			hasSleepTime = -1;
		}
		pthread_mutex_unlock(&m_vectMutex);
	}
}

#if 0
#include <sys/epoll.h>

#define THREAD_NUM 10

int startTime = 1000000;
int epollfd;
MyTimer timer(1);

void *ThreadToRegister(void *arg){
	TimerEvent event;
	event.isNew = true;
	event.leftTime = *((int *)arg);

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
