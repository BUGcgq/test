#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "thpool.h"
// #include "event_framework.h"
#include <modbus/modbus.h>
// #define MAX_STRING_LENGTH 10

// void eventHandler(void *data)
// {
//     char *st_data = (char *)data;
//     printf("eventHandler = %s\n",st_data);
// }

// void eventHandler1(void *data)
// {
//     char *st_data = (char *)data;
//     printf("延时 = %s\n",st_data);
// }

// void generateRandomString(char *randomString, int length)
// {
//     const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
//     int i;
//     for (i = 0; i < length; ++i)
//     {
//         int index = rand() % (sizeof(charset) - 1);
//         randomString[i] = charset[index];
//     }
//     randomString[length] = '\0';  // 添加字符串结尾
// }

// void *task1(void *arg)
// {
//     while (1)
//     {
//         char randomString[MAX_STRING_LENGTH + 1];  // +1 是为了容纳字符串结尾的 '\0'
//         generateRandomString(randomString, MAX_STRING_LENGTH);  // 生成随机字符串
//         publish_event_message(1, randomString, sizeof(randomString));
//         sleep(2);
//     }

//     return NULL;

// }

// void *task2(void *arg)
// {

//     subscribe_event_topic(1,eventHandler);
//     while (1)
//     {
//         sleep(1);
//     }

//     return NULL;
// }

// void *task3(void *arg)
// {

//     subscribe_event_topic(1,eventHandler1);
//     while (1)
//     {
//         sleep(1);
//     }

//     return NULL;
// }

// int main()
// {
//     init_event_framework();
//     threadpool thpool = thpool_init(3);

//     thpool_add_work(thpool, (void *)task1, NULL);
//     thpool_add_work(thpool, (void *)task2, NULL);
//     thpool_add_work(thpool, (void *)task3, NULL);

//     int	num = thpool_num_threads_working(thpool);
//     printf("开启了 %d 条线程\n", num);

//     thpool_wait(thpool);
//     thpool_destroy(thpool);

//     cleanup_event_framework();

//     return 0;
// }

int main()
{
    modbus_t *ctx;
    uint16_t data = 0; // 初始数据
    int rc;
    int errno;

    // 创建 Modbus RTU 上下文
    ctx = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);
    if (ctx == NULL)
    {
        fprintf(stderr, "Unable to create the libmodbus context\n");
        return -1;
    }

    // 设置从站地址
    modbus_set_slave(ctx, 1);

    // 打开 Modbus RTU 串口
    if (modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    while (1)
    {
        // 每隔 10 秒发送一次数据
        sleep(10);

        // 写入保持寄存器值
        rc = modbus_write_register(ctx, 0, data);
        if (rc == -1)
        {
            fprintf(stderr, "Write failed: %s\n", modbus_strerror(errno));
            modbus_close(ctx);
            modbus_free(ctx);
            return -1;
        }

        // 输出写入的数据
        printf("Write data: %d (0x%X)\n", data, data);

        // 数据递增
        data++;

        // 如果超过了 65535，重新开始
        if (data > 65535)
        {
            data = 0;
        }
    }

    // 关闭 Modbus RTU 串口
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}