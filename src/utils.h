#ifndef UTILS_H_
#define UTILS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <map>
#include <iostream>
#include <cerrno>
#include <cstdlib>

#include "sock.h"

using namespace std;

void ParseConfigFile(string, map<string, string> &);

double Randomf(int a,int b);
int Randomi(int a,int b);

string numToString(int num);
int stringToInt(const string &value);

int mysleep(unsigned int usec);

int getCurrentTime(struct timeval *tv);
double getRelativeTime();
double getTimeSlips(struct timeval *a,struct timeval *b);

// int comp(const FileCount &a ,const FileCount &b);
// int compS(const ServerLoad &a ,const ServerLoad &b);

double  minDouble(double a,double b);

inline int Socket(int family, int type, int protocol)
{
	int n;
	if ((n = socket(family, type, protocol)) < 0) {
		cout << "socket error" << endl;
		exit(1);
	}
	return n;
}

inline void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0) {
		cout << "bind error" << endl;
		exit(1);
	}
}

inline void Listen(int fd, int backlog)
{
	if (listen(fd, backlog) < 0) {
		cout << "listen error" << endl;
		exit(1);
	}
}

inline int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int n;
again:
	if ((n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
		goto again;
		else {
			cout << "accept error" << endl;
			exit(1);
		}
	}
	return n;
}

inline void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (connect(fd, sa, salen) < 0) {
		cout << "connect error" << endl;
		exit(1);
	}
}

inline void Inet_pton(int family, const char *strptr, void *addrptr)
{
	int	n;
	if ((n = inet_pton(family, strptr, addrptr)) <= 0) {
		cout << "inet_pton " << strptr << " error" << endl;
		exit(1);
	}
}


inline void *Malloc(size_t size)
{
	void *ptr;
	if ((ptr = malloc(size)) == NULL) {
		cout << "malloc error" << endl;
		exit(1);
	}
	return(ptr);
}

inline void Pthread_create(pthread_t *tid, 
	const pthread_attr_t *attr, 
	void * (*func)(void *), 
	void *arg) 
{
	int	n;
	if ((n = pthread_create(tid, attr, func, arg)) == 0)
		return;
	errno = n;
	cout << "pthread_create error" << endl;
	exit(1);
}

inline void Pthread_detach(pthread_t tid)
{
	int	n;
	if ((n = pthread_detach(tid)) == 0)
		return;
	errno = n;
	cout << "pthread_detach error" << endl;
	exit(1);
}

inline int TcpConnect(const char* host, const char* port)
{
	int ret = tcp_connect(host, port);
	if(ret < 0){
		cout << "tcp connetc error" << endl;
		exit(1);
	}
	return ret;
}

#endif