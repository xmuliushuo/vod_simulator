#ifndef SERVERMGR_H
#define SERVERMGR_H

#include "config.h"
#include "server.h"


class ServerManager
{
public:
	ServerManager(ConfigType &);
	~ServerManager();

	void Run();
	/* data */
public:
	ConfigType m_config;
	Server *m_lbserver;
};



#endif
