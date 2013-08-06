#include <iostream>
#include "utils.h"
#include "server.h"
#include "timer.h"

using namespace std;

int main()
{
	Timer::GetTimer();
	string config_file_name = "./config/simulator.cfg";
	map<string, string> config_info;
	ParseConfigFile(config_file_name, config_info);
	Server *server = new Server();
	if (server->Init(config_info))
		server->Run();
	else {
		cout << "server init error" << endl;
	}
	return 0;
}
