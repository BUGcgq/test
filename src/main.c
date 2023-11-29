#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "package.h"
#include "mqtt.h"
#include "message.h"

MQTTClient client;
static int connect_state = 0;
char *productKey;
char *deviceName;

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	printf(RED_COLOR "[topicName] = %s [payload] = %s\n\n" RESET_COLOR, topicName, (char *)message->payload);
	if (strstr(topicName, "/ota/device/upgrade/") != NULL)
	{
		printf("服务器推送固件信息\n");
	}
	else if (strstr(topicName, "/ext/ntp/") != NULL && strstr(topicName, "/response") != NULL)
	{
		struct devTime data;
		memset(&data, 0, sizeof(data));
		if (parseDevTimeInfo((char *)message->payload, &data) == 0)
		{
			if (setSystemTime(&data) == 0)
			{
				printf("设置时间成功\n");
			}
		}
	}
	else if (strstr(topicName, "/thing/model/up_raw_reply") != NULL)
	{
		printf("事件相应\n");
	}
	else if (strstr(topicName, "/thing/event/property/post_reply") != NULL)
	{
		printf("属性上报响应\n");
	}
	else if (strstr(topicName, "/thing/event/property/set") != NULL)
	{
		printf("属性设置请求\n");
	}
	else if (strstr(topicName, "/thing/service/") != NULL)
	{
		RecvMessage data;
		memset(&data, 0, sizeof(data));
		if (parseRecvData((char *)message->payload, &data) == 0)
		{
			processServiceHandle(client, productKey, deviceName, data.id, data.method, data.params);
		}
	}

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	return 1;
}

// 回调函数：连接丢失
void connectionLost(void *context, char *cause)
{
	printf("Connection lost\n");
	if (client != NULL)
	{
		disconnect_mqtt(&client);
	}

	connect_state = 0;
}

void *pv_mqtt_comm_thread(void *arg)
{
	if (arg == NULL)
	{
		printf("Error: MqttConfig pointer is NULL\n");
		return NULL;
	}

	MqttConfig *mqtt_config = (MqttConfig *)arg;
	int i;
	int gun_state_bak;
	int feel_flag;
	int sync_flag;
	while (1)
	{
		int rc = connect_mqtt(&client, mqtt_config);
		if (rc == MQTTCLIENT_SUCCESS)
		{
			connect_state = 1;
		}
		else
		{
			printf("Failed to connect to MQTT server. Retrying...\n");
		}

		while (connect_state == 1)
		{

			if (feel_flag)
			{
				if (600)
				{
					printf("请求计费模型\n");
				}
				sleep(1);
				continue;
			}
			if (1)
			{
				printf("对时\n");
			}

			if (600)
			{
				printf("交流桩实时监测数据\n");
			}

			if (90)
			{
				printf("交流桩充电枪充电中监测数据\n");
			}
			if (180)
			{
				printf("交流桩充电枪非充电中监测数据\n");
			}
			if (60)
			{
				printf("交流输出电表底值监测数据\n");
			}
			if (gun_state_bak)
			{
				printf("事件：交流充电枪状态变更事件\n");
			}
			if (1)
			{
				printf("有未上传订单\n");
			}

			sleep(1);
		}

		sleep(30);
	}

	return NULL;
}

int main()
{

	struct devMduInfoEvt devMduInfo = {
		.netMduInfo = "NetMdu123",
		.netMduSoftVer = "1.0.0",
		.netMduImei = "123456789",
		.smartGun = 10,
		.mduInfoInt = {20, 30},
		.mduInfoString = {"CPUNum123", "Reserved456"}};

	char *str2 = createDevMdunInfoEvtRequest("2", &devMduInfo);
	printf(YELLOW_COLOR "[topicName]\n%s\n[payload]\n%s\n\n" RESET_COLOR, "cgq", str2);
	RecvMessage data;
	memset(&data, 0, sizeof(data));
	parseRecvData(str2, &data);
	// 打印结构体的值
	printf("id: %s\n", data.id);
	printf("version: %s\n", data.version);
	printf("method: %s\n", data.method);
	printf("params: %s\n", data.params);
	char *str = generateUniqueIdStr();

	printf("id = %s\n", str);

	free(str);

	return 0;
}
