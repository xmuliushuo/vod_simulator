#ifndef MESSAGE_H_
#define MESSAGE_H_

const int MESSAGELEN = 20;
const int MAX_LISTEN_NUM = 100; 
const int LBSERVER=1000000;

enum MessageType{
    RESOURCEREQUEST,
    REQUESTACK,
    EXIT,
    EXITACK
};


typedef struct Message{
    unsigned int type;
    unsigned int clientId;
    unsigned int serverId;
    union{
        unsigned int fileId;
        unsigned int playLen;
    }info;
}Message;


#endif
