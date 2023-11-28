#include "list.h"

RecvMessageQueue recvQueue; // 接收队列

void initRecvMessageQueue() // 初始化消息队列
{
    recvQueue.head = NULL;
    recvQueue.tail = NULL;
    pthread_mutex_init(&recvQueue.mutex, NULL);
    recvQueue.nodeCount = 0;
}

void enqueueRecvMessage(const char *topicName, const char *id, const char *version, const char *method, const char *params)
{
    if (topicName == NULL || id == NULL || version == NULL || method == NULL || params == NULL)
    {
        return;
    }
    RecvMessage *newMessage = (RecvMessage *)malloc(sizeof(RecvMessage));
    if (newMessage == NULL)
    {
        perror("Failed to allocate memory for new message");
        return;
    }

    strncpy(newMessage->topicName, topicName, sizeof(newMessage->topicName));
    strncpy(newMessage->id, id, sizeof(newMessage->id));
    strncpy(newMessage->version, version, sizeof(newMessage->version));
    strncpy(newMessage->method, method, sizeof(newMessage->method));
    strncpy(newMessage->params, params, sizeof(newMessage->params));
    newMessage->next = NULL;

    pthread_mutex_lock(&recvQueue.mutex);
    if (recvQueue.tail == NULL)
    {
        recvQueue.head = newMessage;
        recvQueue.tail = newMessage;
    }
    else
    {
        recvQueue.tail->next = newMessage;
        recvQueue.tail = newMessage;
    }
    recvQueue.nodeCount++;

    pthread_mutex_unlock(&recvQueue.mutex);
}

bool dequeueRecvMessage(char *topicName, char *id, char *version, char *method, char *params) // 从队列中获取消息
{
    if (topicName == NULL || id == NULL || version == NULL || method == NULL || params == NULL)
    {
        return false;
    }
    pthread_mutex_lock(&recvQueue.mutex);
    if (recvQueue.nodeCount == 0)
    {
        pthread_mutex_unlock(&recvQueue.mutex);
        return false;
    }

    RecvMessage *oldestMessage = recvQueue.head;
    recvQueue.head = oldestMessage->next;
    recvQueue.nodeCount--;

    if (recvQueue.nodeCount == 0)
    {
        recvQueue.tail = NULL;
    }

    pthread_mutex_unlock(&recvQueue.mutex);

    strncpy(topicName, oldestMessage->topicName, sizeof(oldestMessage->topicName));
    strncpy(id, oldestMessage->id, sizeof(oldestMessage->id));
    strncpy(version, oldestMessage->version, sizeof(oldestMessage->version));
    strncpy(method, oldestMessage->method, sizeof(oldestMessage->method));
    strncpy(params, oldestMessage->params, sizeof(oldestMessage->params));

    free(oldestMessage);

    return true;
}

/**
 * @description: 判断是否有未读取信息
 */
bool IsRecvMessage()
{
    int cnt = 0;
    pthread_mutex_lock(&recvQueue.mutex);
    cnt = recvQueue.nodeCount;
    pthread_mutex_unlock(&recvQueue.mutex);

    if (cnt > 0)
        return true;
    else
        return false;
}