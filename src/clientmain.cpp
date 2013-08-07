#include <iostream>
#include <cstdlib>

#include "utils.h"
#include "client.h"
#include "timer.h"

using namespace std;

int main()
{
	// call Timer::GetTimer() to initialize the timer.
	Timer::GetTimer();
	string config_file_name = "./config/simulator.cfg";
	map<string, string> config_info;
	ParseConfigFile(config_file_name, config_info);
	int clientnum = atoi(config_info["clientnum"].c_str());
	for (int i = 0; i < clientnum; ++i) {
		Client *client = new Client(i);
		if (client->Init(config_info))
			client->Run();
		else {
			cout << "client init failed" << endl;
			exit(1);
		}
	}
	return 0;
}