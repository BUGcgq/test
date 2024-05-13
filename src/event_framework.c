#include "event_framework.h"

static OBSER_LIST_T observerList;

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

    // 初始化观察者列表
    observerList.num = 0;
    observerList.head = NULL;
    observerList.threadPool = thpool_init(PoolNum);
    pthread_mutex_init(&observerList.eventLock, NULL);
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
    pthread_mutex_lock(&observerList.eventLock);

    // 清理观察者列表
    OBSER_NODE_T *current_subscriber = observerList.head;
    while (current_subscriber != NULL)
    {
        OBSER_NODE_T *temp_subscriber = current_subscriber;
        current_subscriber = current_subscriber->next;
        free(temp_subscriber);
    }

    observerList.head = NULL;
    observerList.num = 0;

    pthread_mutex_unlock(&observerList.eventLock);

    thpool_destroy(observerList.threadPool);

    pthread_mutex_destroy(&observerList.eventLock);
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
int subscribe_event_topic(int eventType, int priority, EVENT_TRIGGER_F callback)
{
    pthread_mutex_lock(&observerList.eventLock);

    // 创建新的订阅者节点
    OBSER_NODE_T *newSubNode = (OBSER_NODE_T *)malloc(sizeof(OBSER_NODE_T));
    if (newSubNode == NULL)
    {
        perror("无法分配内存");
        pthread_mutex_unlock(&observerList.eventLock);
        return -1;
    }

    // 更新订阅者列表数量
    observerList.num++;

    newSubNode->obServer.id = observerList.num;
    newSubNode->obServer.eventType = eventType;
    newSubNode->obServer.priority = priority;
    newSubNode->obServer.callback = callback;
    newSubNode->next = observerList.head;
    observerList.head = newSubNode;

    pthread_mutex_unlock(&observerList.eventLock);

    return newSubNode->obServer.id;
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
    pthread_mutex_lock(&observerList.eventLock);

    OBSER_NODE_T *current = observerList.head;
    OBSER_NODE_T *prev = NULL;

    while (current != NULL && current->obServer.id != subscriberId)
    {
        prev = current;
        current = current->next;
    }

    if (current != NULL)
    {
        if (prev != NULL)
        {
            prev->next = current->next;
            free(current);
        }
    }

    pthread_mutex_unlock(&observerList.eventLock);
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-17
 * 函 数 名：event_task_handler
 * 描    述: 线程池任务
 *
 * 参    数: arg - [void* 事件数据]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static void event_task_handler(void *arg)
{
    OBSER_NODE_T *task = (OBSER_NODE_T *)arg;
    task->obServer.callback();
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

void publish_event_message(int eventType)
{
    pthread_mutex_lock(&observerList.eventLock);

    OBSER_NODE_T *subscriber_current = observerList.head;
    while (subscriber_current)
    {
        if (subscriber_current->obServer.eventType == eventType && subscriber_current->obServer.callback != NULL)
        {
            // 将任务提交给线程池
            thpool_add_work(observerList.threadPool, (void *)event_task_handler, subscriber_current);
        }

        subscriber_current = subscriber_current->next;
    }

    pthread_mutex_unlock(&observerList.eventLock);
}
