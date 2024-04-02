#include "event_framework.h"
#include "thpool.h"

static Event eventQueue[MAX_EVENTS];
static SubscriberList subscriber_list;
static threadpool threadPool;

// 新的事件处理函数，作为线程池任务
void *task_event_handler(void *arg)
{
    Event *event = (Event *)arg;
    Subscriber *current = subscriber_list.head;
    while (current)
    {
        if (current->eventType == event->eventType)
        {
            current->callback(event->data); // 调用订阅者的回调函数
        }
        current = current->next;
    }
    free(event->data); // 释放事件数据内存
}

/**
 * 事件消费者线程的入口函数
 */
static void *event_consumer(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&subscriber_list.lock);

        while (subscriber_list.front == -1)
        {
            pthread_cond_wait(&subscriber_list.cond, &subscriber_list.lock);
        }

        Event *event = &eventQueue[subscriber_list.front];

        thpool_add_work(threadPool, (void *)task_event_handler, event);

        if (subscriber_list.front == subscriber_list.rear)
        {
            subscriber_list.front = -1;
            subscriber_list.rear = -1;
        }
        else
        {
            subscriber_list.front = (subscriber_list.front + 1) % MAX_EVENTS;
        }

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
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void init_event_framework()
{
    subscriber_list.head = NULL;
    subscriber_list.next_id = 1;
    subscriber_list.front = -1;
    subscriber_list.rear = -1;
    pthread_mutex_init(&subscriber_list.lock, NULL);
    pthread_cond_init(&subscriber_list.cond, NULL);
    threadPool = thpool_init(THREAD_POOL_SIZE);
    thpool_add_work(threadPool, (void *)event_consumer, NULL);
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

    Subscriber *current = subscriber_list.head;
    while (current != NULL)
    {
        Subscriber *temp = current;
        current = current->next;
        free(temp);
    }

    subscriber_list.head = NULL;
    subscriber_list.next_id = 1;
    subscriber_list.front = -1;
    subscriber_list.rear = -1;

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
 * 参    数: callback - [事件回调]
 * 返回类型：int 成功返回事件者id，失败返回-1
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int subscribe_event_topic(int eventType, void * callback)
{
    pthread_mutex_lock(&subscriber_list.lock);

    Subscriber *newSub = (Subscriber *)malloc(sizeof(Subscriber));
    if (newSub == NULL)
    {
        perror("无法分配内存");
        pthread_mutex_unlock(&subscriber_list.lock);
        return -1; // 返回-1表示订阅失败
    }

    newSub->id = subscriber_list.next_id++;
    newSub->eventType = eventType;
    newSub->callback = callback;

    newSub->next = subscriber_list.head;
    subscriber_list.head = newSub;

    pthread_mutex_unlock(&subscriber_list.lock);

    return newSub->id;
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

    Subscriber *current = subscriber_list.head;
    Subscriber *prev = NULL;

    while (current != NULL && current->id != subscriberId)
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL)
    {
        if (prev == NULL)
        {
            subscriber_list.head = current->next;
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

    if ((subscriber_list.rear + 1) % MAX_EVENTS == subscriber_list.front)
    {
        printf("Event queue is full. Event not published.\n");
        pthread_mutex_unlock(&subscriber_list.lock);
        return;
    }

    if (subscriber_list.front == -1)
    {
        subscriber_list.front = 0;
        subscriber_list.rear = 0;
    }
    else
    {
        subscriber_list.rear = (subscriber_list.rear + 1) % MAX_EVENTS;
    }

    Event *event = &eventQueue[subscriber_list.rear];
    event->eventType = eventType;

    event->data = malloc(dataSize);
    if (event->data == NULL)
    {
        perror("无法分配内存");
        subscriber_list.rear = (subscriber_list.rear - 1 + MAX_EVENTS) % MAX_EVENTS;
        pthread_mutex_unlock(&subscriber_list.lock);
        return;
    }
    memcpy(event->data, data, dataSize);

    pthread_cond_signal(&subscriber_list.cond);
    pthread_mutex_unlock(&subscriber_list.lock);
}
