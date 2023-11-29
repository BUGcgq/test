#include "list.h"

SendMessageQueue sendQueue; // 发送队列
RecvMessageQueue recvQueue; // 接收队列
void initSendMessageQueue()
{
    sendQueue.head = NULL;
    sendQueue.tail = NULL;
    sendQueue.nodeCount = 0;
    pthread_mutex_init(&sendQueue.mutex, NULL);
    pthread_cond_init(&sendQueue.cond, NULL);
}

/**
 * @description: 插入信息到队列
 * @param id 信息唯一标识符
 * @param message 发送信息
 * @param action 信息类型
 */
void enqueueSendMessage(const char *id, const char *topic, const char *payload)
{
    if (id == NULL || payload == NULL || topic == NULL || sendQueue.nodeCount > 10)
    {
        fprintf(stderr, "Invalid input: id or message is NULL\n");
        return; // 参数为NULL，返回避免继续执行
    }

    SendMessage *newMessage = (SendMessage *)malloc(sizeof(SendMessage));
    if (!newMessage)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return; // 内存分配失败，返回避免继续执行
    }

    strncpy(newMessage->id, id, sizeof(newMessage->id) - 1);
    strncpy(newMessage->payload, payload, sizeof(newMessage->payload) - 1);
    strncpy(newMessage->topic, topic, sizeof(newMessage->topic) - 1);
    newMessage->status = 0; // 0 表示未回复
    newMessage->next = NULL;

    pthread_mutex_lock(&sendQueue.mutex);

    if (sendQueue.tail == NULL)
    {
        sendQueue.head = newMessage;
        sendQueue.tail = newMessage;
    }
    else
    {
        sendQueue.tail->next = newMessage;
        sendQueue.tail = newMessage;
    }
    sendQueue.nodeCount++;
    // 唤醒等待的线程
    pthread_cond_signal(&sendQueue.cond);

    pthread_mutex_unlock(&sendQueue.mutex);
}

/**
 * @description: 读取队列最新的信息
 * @param id  读取到的id
 * @param message 读取到的message
 * @param action  读取到的action
 * @param status  读取到的姿态0-未发送，1-已发送，2-已超时
 */
bool dequeueSendMessage(char *id, char *topic, char *payload, int *status)
{
    if (id == NULL || payload == NULL || status == NULL || topic == NULL)
    {
        fprintf(stderr, "Invalid input: id, message, status, or startTime is NULL\n");
        return false; // 不处理无效输入
    }
    pthread_mutex_lock(&sendQueue.mutex);

    while (sendQueue.head == NULL)
    {
        pthread_cond_wait(&sendQueue.cond, &sendQueue.mutex); // 如果队列为空，等待新的节点插入才往下读取
    }

    SendMessage *messageNode = sendQueue.head;
    strncpy(id, messageNode->id, sizeof(messageNode->id) - 1);
    strncpy(payload, messageNode->payload, sizeof(messageNode->payload) - 1);
    strncpy(topic, messageNode->topic, sizeof(messageNode->topic) - 1);
    *status = messageNode->status;

    pthread_mutex_unlock(&sendQueue.mutex);
    return true;
}
/**
 * @description: 根据Unique删除信息
 * @param Unique Unique
 */
void deleteSendMessage(const char *id)
{
    if (id == NULL)
    {
        return;
    }
    pthread_mutex_lock(&sendQueue.mutex);

    SendMessage *current = sendQueue.head;
    SendMessage *prev = NULL;

    while (current != NULL)
    {
        if (strcmp(current->id, id) == 0)
        {
            // 找到匹配的消息节点
            if (prev != NULL)
            {
                prev->next = current->next;
            }
            else
            {
                sendQueue.head = current->next;
            }

            if (current == sendQueue.tail)
            {
                sendQueue.tail = prev;
            }
            sendQueue.nodeCount--;
            free(current);
            break;
        }

        prev = current;
        current = current->next;
    }

    // 检查是否还有其他消息
    if (sendQueue.head != NULL)
    {
        // 发出条件变量信号通知其他线程
        pthread_cond_signal(&sendQueue.cond);
    }

    pthread_mutex_unlock(&sendQueue.mutex);
}

void updateSendMessageStatus(const char *id, int status) // 更新消息状态
{
    if (id == NULL)
    {
        return;
    }

    pthread_mutex_lock(&sendQueue.mutex);

    SendMessage *current = sendQueue.head;

    while (current != NULL)
    {
        if (strcmp(current->id, id) == 0)
        {
            current->status = status;
            break;
        }

        current = current->next;
    }

    pthread_mutex_unlock(&sendQueue.mutex);
}

void initRecvMessageQueue() // 初始化消息队列
{
    recvQueue.head = NULL;
    recvQueue.tail = NULL;
    pthread_mutex_init(&recvQueue.mutex, NULL);
    recvQueue.nodeCount = 0;
}

void enqueueRecvMessage(const char *id, const char *version, const char *method, const char *params)
{
    if (id == NULL || version == NULL || method == NULL || params == NULL)
    {
        return;
    }
    RecvMessage *newMessage = (RecvMessage *)malloc(sizeof(RecvMessage));
    if (newMessage == NULL)
    {
        perror("Failed to allocate memory for new message");
        return;
    }

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

bool dequeueRecvMessage(char *id, char *version, char *method, char *params) // 从队列中获取消息
{
    if (id == NULL || version == NULL || method == NULL || params == NULL)
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