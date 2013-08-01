#ifndef __LOG_H__
#define __LOG_H__

#include <iostream>

#include <sys/time.h>

using namespace std;

#define MY_LOG_UNCOND 0
#define MY_LOG_INFO 1
#define MY_LOG_ERR 2

#ifndef CUR_LOG_LEVEL
#define CUR_LOG_LEVEL 0
#endif

extern timeval globalStartTime;

#define LOG(level,msg) \
		{ \
	timeval localEndTime; \
	gettimeofday(&localEndTime,NULL); \
	if(level == 1) \
	cout << "At:" << (((localEndTime.tv_sec - globalStartTime.tv_sec) + \
			(localEndTime.tv_usec - globalStartTime.tv_usec) / 1000000.0)) \
			<< " " << msg << endl; \
			else if(level == 2) \
			cerr << msg << endl; \
		}

#define LOG_INFO(msg) LOG(1,msg)

#define LOG_WRITE(msg,ofs)\
		{ \
	timeval localEndTime; \
	gettimeofday(&localEndTime,NULL); \
	ofs << "At:" << (((localEndTime.tv_sec - globalStartTime.tv_sec) + \
			(localEndTime.tv_usec - globalStartTime.tv_usec) / 1000000.0)) \
			<< " " << msg << endl; \
		}

#define LOG_DISK(ofs,msg) \
		{ \
	timeval localEndTime; \
	gettimeofday(&localEndTime,NULL); \
	ofs << (((localEndTime.tv_sec - globalStartTime.tv_sec) + \
			(localEndTime.tv_usec - globalStartTime.tv_usec) / 1000000.0)) \
			<< " " << msg << endl; \
		}

#define LOG_ERR(msg) LOG(2,msg)

#endif
