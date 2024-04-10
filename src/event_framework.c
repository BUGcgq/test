#include "event_framework.h"
#include "thpool.h"

static SubscriberList subscriber_list;
static threadpool threadPool;
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-10
 * 函 数 名：task_event_handler
 * 描    述: 异步遍历链表
 *
 * 参    数: arg - [参数说明]
 * 返回类型：void*
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void *task_event_handler(void *arg)
{
    pthread_mutex_lock(&subscriber_list.lock);

    EventNode *current = subscriber_list.events;
    while (current)
    {
        SubscriberNode *subscriber_current = subscriber_list.subscribers;
        while (subscriber_current)
        {
            if (subscriber_current->subscriber.eventType == current->event.eventType)
            {
                subscriber_current->subscriber.callback(current->event.data); // 调用订阅者的回调函数
            }
            subscriber_current = subscriber_current->next;
        }

        EventNode *temp = current;
        current = current->next;
        free(temp->event.data);
        free(temp);
    }

    subscriber_list.events = NULL;

    pthread_mutex_unlock(&subscriber_list.lock);

    return NULL;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-10
 * 函 数 名：task_consumer_handler
 * 描    述: 异步处理事件
 *
 * 参    数: arg - [参数说明]
 * 返回类型：void*
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static void *task_consumer_handler(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&subscriber_list.lock);

        while (subscriber_list.events == NULL)
        {
            pthread_cond_wait(&subscriber_list.cond, &subscriber_list.lock);
        }

        thpool_add_work(threadPool, (void *)task_event_handler, NULL);

        pthread_mutex_unlock(&subscriber_list.lock);
    }

    return NULL;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：init_event_framework
 * 描    述: 初始化事件管理系统
 *
 * 参    数: PoolNum - [事件类型]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void init_event_framework(int PoolNum)
{
    if (PoolNum < 2)
    {
        PoolNum = 2;
    }

    subscriber_list.subscribers = NULL;
    subscriber_list.next_id = 1;
    subscriber_list.events = NULL;
    pthread_mutex_init(&subscriber_list.lock, NULL);
    pthread_cond_init(&subscriber_list.cond, NULL);
    threadPool = thpool_init(PoolNum);
    thpool_add_work(threadPool, (void *)task_consumer_handler, NULL);
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：cleanup_event_framework
 * 描    述: 反初始化事件管理系统
 *
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void cleanup_event_framework()
{
    pthread_mutex_lock(&subscriber_list.lock);

    SubscriberNode *current_subscriber = subscriber_list.subscribers;
    while (current_subscriber != NULL)
    {
        SubscriberNode *temp_subscriber = current_subscriber;
        current_subscriber = current_subscriber->next;
        free(temp_subscriber);
    }

    EventNode *current_event = subscriber_list.events;
    while (current_event != NULL)
    {
        EventNode *temp_event = current_event;
        current_event = current_event->next;
        free(temp_event->event.data);
        free(temp_event);
    }

    subscriber_list.subscribers = NULL;
    subscriber_list.next_id = 1;
    subscriber_list.events = NULL;

    pthread_mutex_unlock(&subscriber_list.lock);

    thpool_destroy(threadPool);

    pthread_mutex_destroy(&subscriber_list.lock);
    pthread_cond_destroy(&subscriber_list.cond);
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：subscribe_event_topic
 * 描    述: 订阅事件主题
 *
 * 参    数: eventType - [事件类型]
 * 参    数: priority - [优先级]
 * 参    数: callback - [事件回调]
 * 返回类型：int 成功返回事件者id，失败返回-1
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int subscribe_event_topic(int eventType, int priority, void *callback)
{
    pthread_mutex_lock(&subscriber_list.lock);

    SubscriberNode *newSubNode = (SubscriberNode *)malloc(sizeof(SubscriberNode));
    if (newSubNode == NULL)
    {
        perror("无法分配内存");
        pthread_mutex_unlock(&subscriber_list.lock);
        return -1;
    }

    newSubNode->subscriber.id = subscriber_list.next_id++;
    newSubNode->subscriber.eventType = eventType;
    newSubNode->subscriber.callback = callback;
    newSubNode->subscriber.priority = priority;

    if (subscriber_list.subscribers == NULL || subscriber_list.subscribers->subscriber.priority < priority)
    {
        newSubNode->next = subscriber_list.subscribers;
        subscriber_list.subscribers = newSubNode;
    }
    else
    {
        SubscriberNode *current = subscriber_list.subscribers;
        while (current->next != NULL && current->next->subscriber.priority >= priority)
        {
            current = current->next;
        }
        newSubNode->next = current->next;
        current->next = newSubNode;
    }

    pthread_mutex_unlock(&subscriber_list.lock);

    return newSubNode->subscriber.id;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：unsubscribe_event_topic
 * 描    述: 根据事件者id取消订阅
 *
 * 参    数: subscriberId - [事件者id]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void unsubscribe_event_topic(int subscriberId)
{
    pthread_mutex_lock(&subscriber_list.lock);

    SubscriberNode *current = subscriber_list.subscribers;
    SubscriberNode *prev = NULL;

    while (current != NULL && current->subscriber.id != subscriberId)
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL)
    {
        if (prev == NULL)
        {
            subscriber_list.subscribers = current->next;
        }
        else
        {
            prev->next = current->next;
        }
        free(current);
    }

    pthread_mutex_unlock(&subscriber_list.lock);
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：publish_event_message
 * 描    述: 推送事件
 *
 * 参    数: eventType - [事件类型]
 * 参    数: data - [数据]
 * 参    数: dataSize - [长度]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void publish_event_message(int eventType, void *data, size_t dataSize)
{
    pthread_mutex_lock(&subscriber_list.lock);

    EventNode *newNode = (EventNode *)malloc(sizeof(EventNode));
    if (newNode == NULL)
    {
        perror("无法分配内存");
        pthread_mutex_unlock(&subscriber_list.lock);
        return;
    }

    newNode->event.eventType = eventType;

    newNode->event.data = malloc(dataSize);
    if (newNode->event.data == NULL)
    {
        perror("无法分配内存");
        free(newNode);
        pthread_mutex_unlock(&subscriber_list.lock);
        return;
    }
    memcpy(newNode->event.data, data, dataSize);

    newNode->next = subscriber_list.events;
    subscriber_list.events = newNode;

    pthread_cond_signal(&subscriber_list.cond);
    pthread_mutex_unlock(&subscriber_list.lock);
}
