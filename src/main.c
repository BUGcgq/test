#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "thpool.h"
#include "event_framework.h"

#define MAX_STRING_LENGTH 10

void eventHandler(void *data)
{
    char *st_data = (char *)data;
    printf("st_data = %s\n",st_data);
}

void generateRandomString(char *randomString, int length)
{
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int i;
    for (i = 0; i < length; ++i)
    {
        int index = rand() % (sizeof(charset) - 1);
        randomString[i] = charset[index];
    }
    randomString[length] = '\0';  // 添加字符串结尾
}



void *task1(void *arg)
{
    while (1)
    {
        char randomString[MAX_STRING_LENGTH + 1];  // +1 是为了容纳字符串结尾的 '\0'
        generateRandomString(randomString, MAX_STRING_LENGTH);  // 生成随机字符串
        publish_event_message(1, randomString, sizeof(randomString));
        sleep(2);
    }

    return NULL;

}

void *task2(void *arg)
{

    subscribe_event_topic(1,eventHandler);
    while (1)
    {
        sleep(1);
    }

    return NULL;
}

int main()
{
    init_event_framework();
    threadpool thpool = thpool_init(2);

    thpool_add_work(thpool, (void *)task1, NULL);
    thpool_add_work(thpool, (void *)task2, NULL);

    int	num = thpool_num_threads_working(thpool);
    printf("开启了 %d 条线程\n", num);

    thpool_wait(thpool);
    thpool_destroy(thpool);

    cleanup_event_framework();


    return 0;
}