#ifndef __GLOBAL_FUNCTION_H__
#define __GLOBAL_FUNCTION_H__

#include <fstream>
#include <string>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <cassert>

#define UNUSED(a) if(a){}


void Trim(std::string &s);

void ParseConfigFile(char *configFileName,std::map<std::string,std::string> &keyMap);
int RandomI(int first,int second);
double RandomF(int a,int b);
long int getTimeInterval(struct timeval *a, struct timeval *b);
double GetTime();
#endif
