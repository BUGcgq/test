#ifndef __OCPP_LIST__H__
#define __OCPP_LIST__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR "\033[36m"
#define WHITE_COLOR "\033[37m"

#define RESET_COLOR "\033[0m"

#define MAX_DATA_SIZE                 512//缓冲区大小             



/////////////////接收/////////////////////////////
typedef struct RecvMessage
{
    char topicName[40];
    char id[40];
    char version[40];
    char method[40];
    char params[MAX_DATA_SIZE];
    struct RecvMessage *next;
}RecvMessage;

typedef struct RecvMessageQueue 
{
    RecvMessage *head;
    RecvMessage *tail;
    pthread_mutex_t mutex;
    int nodeCount; // 节点数量
} RecvMessageQueue;

void initRecvMessageQueue();// 初始化消息队列

void enqueueRecvMessage(const char *topicName, const char *id, const char * version, const char * method,const char * params);

bool dequeueRecvMessage(char *topicName, char *id, char *version,char *method,char *params);// 从队列中获取消息

bool IsRecvMessage();//是否有要发送的信息

#ifdef __cplusplus
}
#endif

#endif