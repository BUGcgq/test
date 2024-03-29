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

#define MAX_EVENTS 10
#define THREAD_POOL_SIZE 2

typedef struct Subscriber
{
    int id;
    int eventType;
    void (*callback)(void *);
    struct Subscriber *next;
} Subscriber;

typedef struct
{
    Subscriber *head;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int next_id;
    int front, rear;
} SubscriberList;

typedef struct
{
    int eventType;
    void *data;
    size_t dataSize;
} Event;

typedef struct
{
    void (*function)(void *);
    void *data;
} Task;

typedef struct
{
    pthread_t threads[THREAD_POOL_SIZE];
    Task queue[MAX_EVENTS];
    int front, rear, count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int stop;
} ThreadPool;

void init_event_framework();
void cleanup_event_framework();
int subscribe_event_topic(int eventType, void (*callback)(void *));
void unsubscribe_event_topic(int subscriberId);
void publish_event_message(int eventType, void *data, size_t dataSize);

#ifdef __cplusplus
}
#endif

#endif