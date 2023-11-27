#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
// #include "mqtt.h"
#include "package.h"
// void menu()
// {
// 	printf("\n=== MQTT客户端 : 输入数字选择对应功能===\n");
// 	printf("1. 订阅主题\n");
// 	printf("2. 取消订阅\n");
// 	printf("3. 发送信息到主题\n");
// 	printf("4. Exit\n");
// 	printf("=======================\n");
// }

// // 回调函数：连接丢失
// void connectionLost(void *context, char *cause)
// {
// 	printf("Connection lost\n");
// }
// static const char *productKey = "1245";
// static const char *deviceName = "cgq";
// int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
// {
// 	printf("*****接收到信息*****\n");
// 	printf("主题: %s\n", topicName);
// 	printf("信息长度: %d\n", message->payloadlen);
// 	printf("信息内容: \n%s\n", (char *)message->payload);

// 	// 固件相关主题
// 	if (strstr(topicName, "/ota/device/inform/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL)
// 	{
// 		printf("Handling firmware version inform\n");
// 		// 在这里添加上报固件版本的处理逻辑
// 	}
// 	else if (strstr(topicName, "/ota/device/upgrade/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL)
// 	{
// 		printf("Handling firmware upgrade\n");
// 		// 在这里添加推送固件信息的处理逻辑
// 	}
// 	else if (strstr(topicName, "/ota/device/progress/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL)
// 	{
// 		printf("Handling upgrade progress\n");
// 		// 在这里添加上报升级进度的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/ota/firmware/get_reply") != NULL)
// 	{
// 		printf("Handling firmware get reply\n");
// 		// 在这里添加响应固件请求的处理逻辑
// 	}
// 	else if (strstr(topicName, "/ext/ntp/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/request") != NULL)
// 	{
// 		printf("Handling NTP request\n");
// 		// 在这里添加时钟同步请求的处理逻辑
// 	}
// 	else if (strstr(topicName, "/ext/ntp/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/response") != NULL)
// 	{
// 		printf("Handling NTP response\n");
// 		// 在这里添加时钟同步响应的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/event/property/post") != NULL)
// 	{
// 		printf("Handling property event post\n");
// 		// 在这里添加属性上报请求的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/event/property/post_reply") != NULL)
// 	{
// 		printf("Handling property event post reply\n");
// 		// 在这里添加属性上报响应的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/service/property/set") != NULL)
// 	{
// 		printf("Handling property set request\n");
// 		// 在这里添加属性设置请求的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/service/property/set_reply") != NULL)
// 	{
// 		printf("Handling property set reply\n");
// 		// 在这里添加属性设置响应的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/service/") != NULL)
// 	{
// 		printf("Handling service call request\n");
// 		// 在这里添加服务调用请求的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/service/_reply") != NULL)
// 	{
// 		printf("Handling service call reply\n");
// 		// 在这里添加服务调用响应的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/model/up_raw") != NULL)
// 	{
// 		printf("Handling model up raw request\n");
// 		// 在这里添加事件上送请求的处理逻辑
// 	}
// 	else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, productKey) != NULL && strstr(topicName, deviceName) != NULL && strstr(topicName, "/thing/model/up_raw_reply") != NULL)
// 	{
// 		printf("Handling model up raw reply\n");
// 		// 在这里添加事件上送响应的处理逻辑
// 	}

// 	MQTTClient_freeMessage(&message);
// 	MQTTClient_free(topicName);
// 	return 1;
// }

// void deliveryComplete(void *context, int deliveryToken)
// {
// 	printf("Delivery completed for token: %d\n", deliveryToken);
// }
// int main()
// {
// 	// 设置MQTT配置
// 	MqttConfig mqtt_config;

// 	mqtt_config.address = "116.62.24.127";
// 	mqtt_config.client_id = "cgqtest1";
// 	mqtt_config.username = "increase";
// 	mqtt_config.password = "123456";
// 	mqtt_config.port = 1883;
// 	mqtt_config.tls = 0;
// 	mqtt_config.keepAliveInterval = 60;
// 	mqtt_config.ca_path = "/app/core/ssl/ca/emqxsl-camqtt_config.crt";
// 	mqtt_config.private_key_path = NULL;
// 	mqtt_config.certificate_path = NULL;
// 	mqtt_config.onConnectionLost = connectionLost;
// 	mqtt_config.onMessageArrived = messageArrived;
// 	mqtt_config.onDeliveryComplete = deliveryComplete;

// 	// 初始化MQTT连接
// 	MQTTClient client;
// 	int rc = connect_mqtt(&client, &mqtt_config);
// 	if (rc != MQTTCLIENT_SUCCESS)
// 	{
// 		fprintf(stderr, "Failed to initialize MQTT connection\n");
// 		return -1;
// 	}

// 	int choice;
// 	char topic[256];
// 	char message[256];
// 	while (1)
// 	{
// 		menu();
// 		scanf("%d", &choice);
// 		memset(topic, 0, 256);
// 		memset(message, 0, 256);
// 		switch (choice)
// 		{
// 		case 1:
// 			printf("请输入要订阅的主题: ");
// 			scanf("%s", topic);
// 			while (getchar() != '\n');
// 			subscribe_topic(client, topic, 0);
// 			break;

// 		case 2:
// 			printf("请输入要取消订阅的主题: ");
// 			scanf("%s", topic);
// 			while (getchar() != '\n');
// 			unsubscribe_topic(client, topic);
// 			break;

// 		case 3:
// 			printf("请输入要发送信息的主题: ");
// 			scanf("%s", topic);
// 			while (getchar() != '\n');
// 			printf("请输入要发送信息: ");
// 			scanf("%s", message);
// 			while (getchar() != '\n');
// 			publish_message(client, topic, message, 0);
// 			break;

// 		case 4:
// 			printf("Exiting...\n");
// 			disconnect_mqtt(client);
// 			return 0;

// 		default:
// 			printf("Invalid choice. Please enter a valid option.\n");
// 		}
// 	}
// 	// 断开 MQTT 连接

// 	return 0;
// }
int parseVerInfoEvtSrv(const char *jsondata)
{
	if (jsondata == NULL)
	{
		return -1;
	}

	json_object *jsonObj = json_tokener_parse(jsondata);
	if (jsonObj == NULL)
	{
		return -1;
	}
	json_object *verInfoEvtObj;
	if (!json_object_object_get_ex(jsonObj, "verInfoEvt", &verInfoEvtObj))
	{
		json_object_put(jsonObj);
		return -1;
	}

	// 解析 verInfoEvt 字段内部字段
	json_object_object_foreach(verInfoEvtObj, key, val)
	{
		if (strcmp(key, "devRegMethod") == 0 && json_object_get_type(val) == json_type_int)
		{
			printf("devRegMethod = %d\n", json_object_get_int(val));
		}
		else if (strcmp(key, "pileHardwareVer") == 0 && json_object_get_type(val) == json_type_string)
		{
			printf("pileHardwareVer = %s\n", json_object_get_string(val));
		}
	}

	json_object_put(jsonObj);
	return 0;
}
int main()
{
	struct funConfUpdate_srv myStruct;

	// 使用 memset 将结构体初始化为全零
	memset(&myStruct, 0, sizeof(struct funConfUpdate_srv));

	strcpy(myStruct.confString,"cgq");

	printf("%s\n",createGetFunConfSrvSrvReply("666",&myStruct));
	return 0;
	return 0;
}