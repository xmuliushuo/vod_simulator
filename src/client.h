#ifndef CLIENT_H
#define CLIENT_H

#include "config.h"

class Client {
public:
	Client(int id) : m_clientid(id) { }
	virtual ~Client() { }

	void SetFileID(int id)
	{
		m_fileid = id;
	}

	virtual bool Init(ConfigType &) = 0;
	virtual void Run() = 0;
	virtual void Exit() = 0;

protected:
	int m_clientid;
	int m_fileid;
};

#endif