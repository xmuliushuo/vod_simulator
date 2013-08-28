#include "utils.h"

#include <fstream>

using namespace std;

extern struct timeval startTime;

static void Trim(string &str){
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
}

void ParseConfigFile(string config_filename, map<string, string> &keymap) {
	ifstream infile;
	infile.open(config_filename.c_str());

	string line;
	while (getline(infile, line)) {
		if(line.empty())
			continue;
		Trim(line);
		if(line.at(0) == '#')
			continue;
		int equalPos = line.find_first_of('=');
		string key = line.substr(0, equalPos);
		string value = line.substr(equalPos + 1);
		Trim(key);
		Trim(value);
		keymap.insert(make_pair(key, value));
	}
	infile.close();
}

double Randomf(int a,int b){
	double temp = random()/(RAND_MAX*1.0);
	return a + (b-a) * temp;
}

int Randomi(int a,int b){
	double  temp = random()/(RAND_MAX * 1.0);
	return (int)(a + (b - a ) * temp) ;
}

string numToString(int n){
	char buf[256];
	sprintf(buf,"%d",n);
	string temp(buf);
	return temp;
}

int stringToInt(const string &value){
	int result;
	string value1;
	bool isNegative = false;
	if(value.size()>0 && value[0]=='-'){
		isNegative = true;
		value1 = value.substr(1);
	}
	if(isNegative == true){
		result = atoi(value1.c_str());
		result = 0 - result;
	}else
		result = atoi(value.c_str());
//	cout<<"value = "<<value<<endl;
//	cout<<"result = "<<result<<endl;
	return result;

}

int mysleep(unsigned int usec){
	usleep(usec);
	return 0;
}


int getCurrentTime(struct timeval *tv){
	struct timeval temp;
	gettimeofday(&temp,NULL);
	tv->tv_sec = temp.tv_sec;
	tv->tv_usec = temp.tv_usec;
	return 0;
}
double getRelativeTime(){
	struct timeval curTime;
	gettimeofday(&curTime,NULL);
	return getTimeSlips(&curTime,&startTime);
}
double getTimeSlips(struct timeval *a,struct timeval *b){
	double sec;
	sec = a->tv_sec - b->tv_sec;
	sec += (a->tv_usec - b->tv_usec)/1000000.0;
	return sec;
}
// int comp(const FileCount &a ,const FileCount &b){
// 	return a.count > b.count;
// }
// int compS(const ServerLoad &a ,const ServerLoad &b){
// 	return a.load < b.load;
// }
double  minDouble(double a,double b){
	return a > b ? b:a;
}