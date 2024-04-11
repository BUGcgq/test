#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "thpool.h"


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

    }

    return NULL;
}

void *task2(void *arg)
{

    while (1)
    {

    }
}

void *task3(void *arg)
{
    while (1)
    {
    }
}

int main()
{
    threadpool thpool = thpool_init(3);

    thpool_add_work(thpool, (void *)task1, NULL);
    thpool_add_work(thpool, (void *)task2, NULL);
    thpool_add_work(thpool, (void *)task3, NULL);

    thpool_wait(thpool);

    return 0;
}