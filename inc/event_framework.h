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

typedef void (*CallbackFunction)(void *);

typedef struct Subscriber
{
    int id;
    int eventType;
    int priority;
    CallbackFunction callback;
} Subscriber;

typedef struct SubscriberNode
{
    Subscriber subscriber;
    struct SubscriberNode *next;
} SubscriberNode;


typedef struct Event
{
    int eventType;
    void *data;
    size_t dataSize;
} Event;

typedef struct EventNode
{
    Event event;
    struct EventNode *next;
} EventNode;

typedef struct SubscriberList
{
    pthread_mutex_t lock;
    pthread_cond_t cond;
    EventNode *events;
    SubscriberNode *subscribers;
    int next_id;
} SubscriberList;

void init_event_framework(int PoolNum);
void cleanup_event_framework();
int subscribe_event_topic(int eventType, int priority, void *callback);
void unsubscribe_event_topic(int subscriberId);
void publish_event_message(int eventType, void *data, size_t dataSize);

#ifdef __cplusplus
}
#endif

#endif
