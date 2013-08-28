#ifndef LOG_H_
#define LOG_H_

#include <iostream>

#include <sys/time.h>

#include "utils.h"

using namespace std;
extern pthread_mutex_t log_mutex;

#define LOG(level, msg) \
{ \
	pthread_mutex_lock(&log_mutex);\
	if (level == 1) \
		cout << "At:" << getRelativeTime() << "\t " << msg << endl; \
	else if (level == 2) \
		cerr << msg << endl; \
	pthread_mutex_unlock(&log_mutex);\
}

#define LOG_INFO(msg) LOG(1, msg)
#define LOG_ERR(msg) LOG(2, msg)


#define LOG_NOENDL(level, msg) \
{ \
	pthread_mutex_lock(&log_mutex);\
	if (level == 1) \
		cout << "At:" << getRelativeTime() << "\t " << msg; \
	else if (level == 2) \
		cout << msg; \
	pthread_mutex_unlock(&log_mutex);\
}
#define LOG_INFO_NOENDL(msg) LOG_NOENDL(1, msg)
#define LOG_NOTIME_NOENDL(msg) LOG_NOENDL(2, msg)


#define LOG_WRITE(msg, ofs) \
{ \
	ofs << "At:" << getRelativeTime() << "\t " << msg << endl; \
}

#endif
