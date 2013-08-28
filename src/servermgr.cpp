#include "servermgr.h"
#include "lbserver.h"

ServerManager::ServerManager(ConfigType &config):m_config(config)
{
	//
	m_lbserver = new LBServer(0, m_config);
}

ServerManager::~ServerManager()
{
	delete m_lbserver;
}

void ServerManager::Run()
{
	// m_lbserver->Init(m_config);
	m_lbserver->Run();
}
