#include <sys/time.h>

#include "config.h"
#include "clientmgr.h"
#include "servermgr.h"
#include "timer.h"
#include "log.h"

struct timeval startTime;

pthread_mutex_t log_mutex;


int main(int argc, char *argv[])
{
	// just to initialize the timer.
	Timer::GetTimer();
	gettimeofday(&startTime, NULL);
	pthread_mutex_init(&log_mutex, NULL);

	ConfigType config;
	read_vodspread_config("config/config.ini", config);

	int option;
	bool isserver = true;
	while ((option = getopt(argc, argv, "chso:i:a:l:")) != -1) {
		switch (option) {
		case 'c':
			isserver = false;
			break;
		case 's':
			isserver = true;
			break;
		case 'o':
			config.maxCopyFlow = atoi(optarg);
			LOG_INFO("main: set maxCopyFlow = " << config.maxCopyFlow);
			break;
		case 'i':
			config.maxInFlow = atoi(optarg);
			LOG_INFO("main: set maxInFlow = " << config.maxInFlow);
			break;
		case 'a':
			config.spreadAlgorithm = optarg;
			LOG_INFO("main: set spreadAlgorithm = " << config.spreadAlgorithm);
			break;
		case 'l':
			config.loadThresh = atoi(optarg) / 1000.0;
			LOG_INFO("main: set loadThresh = " << config.loadThresh);
			break;
		case 'h':	
			cout << "Usage: st [option]" << endl;
			cout << "-c, run as client" << endl;
			cout << "-s, run as server(default)" << endl;
			cout << "-h, show this message" << endl;
			cout << "-o, set maxCopyFlow" << endl;
			cout << "-i, set maxInFlow" << endl;
			cout << "-a, set spreadAlgorithm" << endl;
			cout << "-l, set loadThresh(0-1000)" << endl;
			return 0;
		default:
			break;
		}
	}

	if (isserver) {
		ServerManager *server_manager = new ServerManager(config);
		server_manager->Run();
	}
	else {
		ClientManager *client_manager = new ClientManager(config);
		client_manager->Run();
	}

	return 0;
}