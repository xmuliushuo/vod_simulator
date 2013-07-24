#include <iostream>
#include "utils.h"
#include "server.h"

using namespace std;

int main()
{
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
