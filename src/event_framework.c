#include "event_framework.h"

static Event eventQueue[MAX_EVENTS];
static SubscriberList subscriber_list;
static ThreadPool threadPool;

// 事件队列
static void consume_event(Event *event)
{
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

        consume_event(event);

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
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; ++i)
    {
        if (pthread_create(&threadPool.threads[i], NULL, event_consumer, NULL) != 0)
        {
            fprintf(stderr, "Error creating thread %d.\n", i);
            return;
        }
    }

    threadPool.front = 0;
    threadPool.rear = -1;
    threadPool.count = 0;
    threadPool.stop = 0;
    pthread_mutex_init(&threadPool.lock, NULL);
    pthread_cond_init(&threadPool.cond, NULL);
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

    pthread_mutex_lock(&threadPool.lock);
    threadPool.stop = 1;
    pthread_cond_broadcast(&threadPool.cond);
    pthread_mutex_unlock(&threadPool.lock);
    int i;
    for (i = 0; i < THREAD_POOL_SIZE; ++i)
    {
        pthread_join(threadPool.threads[i], NULL);
    }

    pthread_mutex_destroy(&subscriber_list.lock);
    pthread_cond_destroy(&subscriber_list.cond);

    pthread_mutex_destroy(&threadPool.lock);
    pthread_cond_destroy(&threadPool.cond);
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
int subscribe_event_topic(int eventType, void (*callback)(void *))
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




// 订阅者处理函数
// 观察者的事件处理函数
// 示例回调函数，模拟事件处理行为

// struct mydata
// {
// 	int data_int;
// 	char data_str[32];
// };

// void eventHandler(void *data)
// {
// 	struct mydata recvdata = *(struct mydata *)data;
// 	printf("1接收到的数据data_str :%s,data_int :%d\n", recvdata.data_str,recvdata.data_int);
// }
// void eventHandler2(void *data)
// {
// 	int receivedData = *(int *)data;
// 	printf("2接收到的数据: %d\n", receivedData);
// }

// // 发布者线程函数
// void *publisherThread(void *arg)
// {

// 	char *data_str = "cgq"; // 模拟要发布的数据
// 	int data_int = 50;
// 	struct mydata senddata;
// 	senddata.data_int = 60;
// 	memcpy(senddata.data_str,"inc",sizeof(senddata.data_str));
// 	printf("发布者线程: 准备发布数据...\n");
// 	while (1)
// 	{
// 		publish_event_message(1, &senddata,sizeof(senddata));
// 		sleep(1); // 模拟准备数据的时间
// 		publish_event_message(2, &data_int,sizeof(data_int));
// 	}

// 	printf("发布者线程: 数据发布完成。\n");
// 	return NULL;
// }

// // 订阅者线程函数
// void *subscriberThread(void *arg)
// {
// 	printf("订阅者线程: 订阅事件...\n");
// 	int id = subscribe_event_topic(1, eventHandler);
// 	while (1)
// 	{
// 		sleep(1);
// 	}

// 	return NULL;
// }

// // 订阅者线程函数
// void *subscriberThread1(void *arg)
// {
// 	printf("订阅者线程: 订阅事件...\n");
// 	subscribe_event_topic(2, eventHandler2);
// 	while (1)
// 	{
// 		sleep(1);
// 	}

// 	return NULL;
// }

// int main()
// {

// 	init_event_framework();
// 	pthread_t pubThreadId, subThreadId1, subThreadId2;

// 	// 创建订阅者线程，它会订阅事件并等待处理它们
// 	if (pthread_create(&subThreadId1, NULL, subscriberThread, NULL) != 0)
// 	{
// 		perror("创建订阅者线程失败");
// 		return 1;
// 	}

// 	if (pthread_create(&subThreadId2, NULL, subscriberThread1, NULL) != 0)
// 	{
// 		perror("创建订阅者线程失败");
// 		return 1;
// 	}

// 	// 创建发布者线程，它将在稍后发布一个事件
// 	if (pthread_create(&pubThreadId, NULL, publisherThread, NULL) != 0)
// 	{
// 		perror("创建发布者线程失败");
// 		return 1;
// 	}

// 	// 等待两个线程完成
// 	pthread_join(pubThreadId, NULL);
// 	pthread_join(subThreadId1, NULL);

// 	// 清理订阅列表和其他资源
// 	cleanup_event_framework();

// 	return 0;
// }