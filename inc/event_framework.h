#ifndef EVENT_FRAMEWORK_H
#define EVENT_FRAMEWORK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "thpool.h"

typedef void (*EVENT_TRIGGER_F)();

typedef struct OBSER_INFO_T
{
    int id;
    int eventType;
    int priority;
    EVENT_TRIGGER_F callback;
} OBSER_INFO_T;

typedef struct OBSER_NODE_T
{
    struct OBSER_INFO_T obServer;
    struct OBSER_NODE_T *next;
} OBSER_NODE_T;

typedef struct OBSER_LIST_T
{
    int num;
    threadpool threadPool;
    pthread_mutex_t eventLock;
    struct OBSER_NODE_T *head;
} OBSER_LIST_T;



void init_event_framework(int PoolNum);
void cleanup_event_framework();
int subscribe_event_topic(int eventType, int priority, EVENT_TRIGGER_F callback);
void unsubscribe_event_topic(int subscriberId);
void publish_event_message(int eventType);

#ifdef __cplusplus
}
#endif

#endif
