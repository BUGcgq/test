#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "thpool.h"
#include "event_framework.h"

void callback1(void *data)
{
    printf("callback1 = %s\n", (char *)data);
}

void callback2(void *data)
{
    printf("callback2 = %s\n", (char *)data);
}

void *task1(void *arg)
{

    char *data = "cgq";
    char *data1 = "inc";
    while (1)
    {
        publish_event_message(1, data, 4);
        sleep(3);
        publish_event_message(2, data1, 4);
        sleep(2);
    }

    return NULL;
}

void *task2(void *arg)
{
    subscribe_event_topic(1, 1, callback1);
    while (1)
    {

    }
}

void *task3(void *arg)
{
    subscribe_event_topic(2, 2, callback2);
    while (1)
    {
    }
}

int main()
{

    init_event_framework(3);
    threadpool thpool = thpool_init(3);

    thpool_add_work(thpool, (void *)task1, NULL);
    thpool_add_work(thpool, (void *)task2, NULL);
    thpool_add_work(thpool, (void *)task3, NULL);

    thpool_wait(thpool);

    return 0;
}