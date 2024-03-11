#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "log.h"
#include "ftp.h"
#include "base64.h"
// int main()
// {
// 	LogConfig config;
// 	memcpy(config.log_dir, "/mnt/mmcblk0p1/log", sizeof(config.log_dir));
// 	memcpy(config.zero_dir, "/mnt/mmcblk0p1/can", sizeof(config.zero_dir));
// 	config.max_log_size = 3 * 1024 * 1024, // 3MB
// 	config.max_save_days = 10,		   // 最大保存天数
// 	config.extra_delete = 5,		   // 超天数删除最老的5条
// 	config.max_msg_num = 20,		   // 消息队列最大写入条数
// 	config.write_msg_interval = 10;     //写缓冲区时间
// 	config.log_level = LOG_LEVEL_INFO; // 设置为需要的日志级别

// 	// 写入日志
// 	init_log_system(&config);
// 	// while (1)
// 	// {
// 	// 	LOG_INFO("LOG_INFO");
// 	// 	sleep(10);
// 	// }

// 	// 关闭日志系统

// 	if(ocpp_download_file("ftp://inc02:za%40g0Pg6J@14.29.244.45:21/inc/kltt/KE-6720-M.bin.tar.bz2","/app/core/KE-6720-M.bin.tar.bz2",1) == 0)
// 	LOG_INFO("下载成功");
// 	else
// 	LOG_INFO("下载失败");
// 	close_log_system();
// 	return 0;
// }
// 定义回调函数的结构体

// 创建发布者线程

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

int main()
{
    const char* original_url = "ftp://inc02:za%40g0Pg6J@14.29.244.45:21/EVCM-SD10.tar.gz";
    // Base64编码URL
    char* encoded_url = base64_encode((const unsigned char*)original_url, strlen(original_url));
    if (encoded_url) {
        printf("Base64 Encoded URL: %s\n", encoded_url);
        free(encoded_url);
    }
    // Base64解码URL
    const char* base64_encoded_url = "ZnRwOi8vaW5jMDI6emElNDBnMFBnNkpAMTQuMjkuMjQ0LjQ1OjIxL0VWQ00tU0QxMC50YXIuZ3o=";
    char* decoded_url = base64_decode(base64_encoded_url);
    if (decoded_url) {
        printf("Base64 Decoded URL: %s\n", decoded_url);
        free(decoded_url);
    }

    return 0;
}