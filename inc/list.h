#ifndef __LIST__H__
#define __LIST__H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR "\033[36m"
#define WHITE_COLOR "\033[37m"

#define RESET_COLOR "\033[0m"

#define MAX_DATA_SIZE                 512//缓冲区大小 


//发送
typedef struct Message 
{
    char id[40];
    char topic[128];
    char payload[MAX_DATA_SIZE];
    int status;//0:未发送，1：已发送，2超时
    struct Message *next;
} SendMessage;

typedef struct SendMessageQueue 
{
    SendMessage *head;
    SendMessage *tail;
    int nodeCount; // 节点数量
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SendMessageQueue;


void initSendMessageQueue();// 初始化消息队列
void enqueueSendMessage(const char *id,const char *topic,const char *payload);// 添加消息到队列
bool dequeueSendMessage(char *id, char *topic,char *payload,int *status);
void updateSendMessageStatus(const char *id, int status);// 更新消息状态
void deleteSendMessage(const char *id);

//接收
typedef struct RecvMessage
{
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

void enqueueRecvMessage(const char *id, const char * version, const char * method,const char * params);

bool dequeueRecvMessage(char *id, char *version,char *method,char *params);// 从队列中获取消息

bool IsRecvMessage();//


#ifdef __cplusplus
}
#endif

#endif