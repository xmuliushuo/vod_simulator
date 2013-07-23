#ifndef __MY_MESSAGE_H__
#define __MY_MESSAGE_H__

#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>

using namespace std;

#define MSG_CLIENT_JOIN  		0
#define MSG_REQUEST_SEG			1
#define MSG_DELETE_SEG			2
#define MSG_FAKE_FIN			3
#define MSG_ADD_SEG				4
#define MSG_REMOTE_FAKE_FIN		5
#define MSG_BAD_REQ				6
#define MSG_REDIRECT			7
#define MSG_SEG_ASK				8
#define MSG_SEG_ACK				9
#define MSG_JOIN_ACK			10
#define MSG_CLIENT_LEAVE		11
#define MSG_CONNECT_FIN		12
#define MSG_SEG_FIN			13

#define MSG_TAKE_SAMPLE		20
#define MSG_CLIENT_DELAY	21
#define MSG_SEARCH_DEVICE	22
#define MSG_RESET_BUFFER	23

#define OPER_READ			0
#define OPER_WRITE			1
#define OPER_WAIT			2//先执行读，而后移除

#define SERVER_NUM			0

#define PER_SEND_PACKETS	1000

#define MAX_CLIENT_NUM 		300
#define MAX_FILE_NUM		1000
#define MAX_DEV_NUM		100

#define MAX_CLIENT_LINKS	4

//seconds
#define LOSS_CONNECT_TIME	5

#define LOCAL_FIN	0
#define REMOTE_FIN 1
#define NONE_FIN	2
#define START_FIN	3

enum HMMSTATUS{
	EMPTY,
	PLAY,
	STOP,
	FORWARD,
	BACKWARD,
	PAUSE
};

#define MAX_LISTEN_NUM	1000

struct ClientInfoBlock{
//	int listenFd;//被动
	int recvFd;//被动
//	int drivListenFd;
//	int drivrecvFd;
	struct sockaddr_in address;
};

struct ClientReqBlock{
	int oper;
	int leftSize;
	int fileId;
	int segId;
	int clientNum;
	int preOper;
	int segNums;
	double bitRate;
	int localfin;
};

//struct FileSegBlock{
//	list<ClientReqBlock>::iterator reqIter;
//	int segId;
//	int clientNum;
//	bool isIn;
//};
//
//struct FileBlock{
//	list<FileSegBlock> fileSegList;
//};

#endif
