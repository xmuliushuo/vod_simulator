#ifndef VOD_H_
#define VOD_H_

#include <sys/socket.h>

#include <iostream>
#include <cerrno>

using namespace std;

const int MESSAGELEN = 20;

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

#endif
