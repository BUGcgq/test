#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "thpool.h"
#include "config.h"
void *task1(void *arg)
{

    while (1)
    {
        printf("task1 !!!!!!\n");
        sleep(1);
    }

    return NULL;
}

void *task2(void *arg)
{

    while (1)
    {
        printf("task2 !!!!!!\n");
        sleep(2);
    }
}

void *task3(void *arg)
{
    while (1)
    {
        printf("task3 !!!!!!\n");
         sleep(3);
    }
}

int main()
{
    // threadpool thpool = thpool_init(3);

    // thpool_add_work(thpool, (void *)task1, NULL);
    // #ifdef CGQ
    //     thpool_add_work(thpool, (void *)task2, NULL);
    // #endif
    // thpool_add_work(thpool, (void *)task3, NULL);

    // thpool_wait(thpool);

    stCFG_PROTOCOLINFO protocol;

    CFG_DefaultProtocol(&protocol);    

    CFG_SaveProtocol("/app/config/protocol.json",&protocol);

    return 0;
}