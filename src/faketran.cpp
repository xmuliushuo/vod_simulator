#include "faketran.h"

FakeTran::FakeTran()
{
	socketpair(AF_UNIX, SOCK_STREAM, 0, m_faketranfd);
}

FakeTran::~FakeTran()
{
	close(mSockFd[0]);
	close(mSockFd[1]);
}