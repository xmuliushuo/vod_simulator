#ifndef SERVER_H
#define SERVER_H

#include "config.h"


class Server
{
public:
	Server(int serverId):mServerId(serverId){ };
	virtual ~Server(){};
	int GetServerId()
	{
		return mServerId;
	}

	virtual bool Init(ConfigType &) = 0;	
	virtual void Run() = 0;
	/* data */

protected:
	int mServerId;
};


#endif