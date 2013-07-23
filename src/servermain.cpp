#include "utils.h"
#include <iostrem>

int main()
{
	string config_file_name = "./config/simulator.cfg";
	map<string, string> config_info;
	ParseConfigFile(config_file_name, config_info);
	Server *server = new Server();
	if (server->Init())
		server->Run();
	else {
		cout << "server init error" << endl;
	}
	return 0;
}