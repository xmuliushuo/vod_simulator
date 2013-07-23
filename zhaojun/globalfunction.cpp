/*
 * globalfunction.cc
 *
 *  Created on: 2013-1-1
 *      Author: zhaojun
 */

#include "globalfunction.h"
#include "mytimer.h"
#include <sys/time.h>

extern MyTimer globalTimer;
extern timeval globalStartTime;

int RandomI(int first,int second){
	double temp = random() / (RAND_MAX * 1.0);
	return (int)(first + (second - first) * temp);
}

double RandomF(int a,int b){
	double temp = random() / (RAND_MAX * 1.0);
	return a + (b - a) * temp;
}

double RandomF(double a,double b){
	double temp = random() / (RAND_MAX * 1.0);
	return a + (b - a) * temp;
}

long int getTimeInterval(struct timeval *a, struct timeval *b)
{
	long int usec;
	usec = a->tv_sec - b->tv_sec;
	usec = usec * 1000000;
	usec += a->tv_usec - b->tv_usec;
	return usec;
}

double  GetTime(){
	double retVal;
	timeval localEndTime;
	gettimeofday(&localEndTime,NULL);
    retVal = (
				(localEndTime.tv_sec - globalStartTime.tv_sec) +
                        (localEndTime.tv_usec - globalStartTime.tv_usec)/1000000.0) / globalTimer.getMultiple();
	
	return retVal;
}

