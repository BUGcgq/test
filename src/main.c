#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "package.h"
#include "mqtt.h"
#include "list.h"

MQTTClient client;
static int connect_state = 0;
// 充电桩响应的主题是/sys/{productKey}/{deviceName}/thing/service/{identifier}_reply
void processService(const char *id, const char *method, const char *params) // 处理服务数据的函数
{
	if (id == NULL || params == NULL || method == NULL)
	{
		return;
	}

	if (strstr("confUpdateSrv", method) == 0)
	{
		struct confUpdateSrv confUpdateSrv;
		if (parseConfUpdateSrv(params, &confUpdateSrv) == 0)
		{
		}
	}

	else if (strstr("getConfSrv", method) == 0)
	{
		parseGetConfSrv(params);
	}

	else if (strstr("funConfUpdateSrv", method) == 0)
	{
		struct funConfUpdate_srv funConfUpdate;
		if (parseFunConfUpdateSrv(params, &funConfUpdate) == 0)
		{
		}
	}

	else if (strstr("getFunConfSrv", method) == 0)
	{
		if (parseGetFunConfSrv(params) == 0)
		{
		}
	}

	else if (strstr("queDataSrv", method) == 0)
	{
		struct queDataSrv queDataSrv;
		if (parseQueDataSrv(params, &queDataSrv) == 0)
		{
		}
	}

	else if (strstr("devMaintainSrv", method) == 0)
	{
		if (parseDevMaintainSrv(params) == 0)
		{
		}
	}

	else if (strstr("devMaintainQuerySrv", method) == 0)
	{
		if (parseDevMaintainQuerySrv(params) == 0)
		{
		}
	}

	else if (strstr("feeModelUpdateSrv", method) == 0)
	{
		struct feeModelUpdateSrv feeModelUpdateSrv;
		if (parseFeeModelUpdateSrv(params, &feeModelUpdateSrv) == 0)
		{
		}
	}
	else if (strstr("feeModelQuerySrv", method) == 0)
	{
		if (parseFeeModelQuerySrv(params) == 0)
		{
		}
	}
	else if (strstr("feeModelUpdateSrv", method) == 0)
	{
		struct feeModelUpdateSrv feeModelUpdateSrv;
		if (parseFeeModelUpdateSrv(params, &feeModelUpdateSrv) == 0)
		{
		}
	}
	else if (strstr("startChargeSrv", method) == 0)
	{
		struct startChargeSrv startChargeSrv;
		if (parseStartChargeSrv(params, &startChargeSrv) == 0)
		{
		}
	}
	else if (strstr("authResultSrv", method) == 0)
	{
		struct authResultSrv authResultSrv;
		if (parseAuthResultSrv(params, &authResultSrv) == 0)
		{
		}
	}
	else if (strstr("stopChargeSrv", method) == 0)
	{
		struct stopChargeSrv stopChargeSrv;
		if (parseStopChargeSrv(params, &stopChargeSrv) == 0)
		{
		}
	}
	else if (strstr("orderCheckSrv", method) == 0)
	{
		struct orderCheckSrv orderCheckSrv;
		if (parseOrderCheckSrv(params, &orderCheckSrv) == 0)
		{
		}
	}
	else if (strstr("acOrderlyChargeSrv", method) == 0)
	{
		struct acOrderlyChargeSrv acOrderlyChargeSrv;
		if (parseAcOrderlyChargeSrv(params, &acOrderlyChargeSrv) == 0)
		{
		}
	}
}

int parseRecvData(char *topicName, const char *payload) // 解析服务器下放的事件，服务，属性数据的包
{
	if (payload == NULL || topicName == NULL)
	{
		return -1;
	}
	char id[40];
	char version[40];
	char method[40];
	char params[MAX_DATA_SIZE];
	json_object *jsonObj = json_tokener_parse(payload);
	if (jsonObj != NULL)
	{
		json_object_object_foreach(jsonObj, key, val)
		{
			if (strcmp(key, "id") == 0 && json_object_get_type(val) == json_type_string)
			{
				strncpy(id, json_object_get_string(val), sizeof(id) - 1);
			}
			else if (strcmp(key, "version") == 0 && json_object_get_type(val) == json_type_string)
			{
				strncpy(version, json_object_get_string(val), sizeof(version) - 1);
			}
			else if (strcmp(key, "method") == 0 && json_object_get_type(val) == json_type_string)
			{
				strncpy(method, json_object_get_string(val), sizeof(method) - 1);
			}
			else if (strcmp(key, "params") == 0)
			{
				const char *paramsStr = json_object_to_json_string(val);
				strncpy(params, paramsStr, sizeof(params) - 1);
			}
		}

		enqueueRecvMessage(topicName, id, version, method, params);

		json_object_put(jsonObj);
		return 0;
	}

	return -1;
}

void packageRecvHandler()
{

	if (IsRecvMessage())
	{
		char topicName[40];
		char id[40];
		char version[40];
		char method[40];
		char params[MAX_DATA_SIZE];
		if (dequeueRecvMessage(topicName, id, version, method, params))
		{

			printf("Topic Name: %s\n", topicName);
			printf("ID: %s\n", id);
			printf("Version: %s\n", version);
			printf("Method: %s\n", method);
			printf("Params: %s\n", params);

			if (strstr(topicName, "/sys/") != NULL && strstr(topicName, "/thing/event/property/post_reply") != NULL)
			{
				printf("Handling property event post reply\n");
				// 在这里添加属性上报响应的处理逻辑
			}
			else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, "/thing/service/property/set_reply") != NULL)
			{
				printf("Handling property set reply\n");
				// 在这里添加属性设置响应的处理逻辑
			}
			else if (strstr(topicName, "/sys/") != NULL && strstr(topicName, "/thing/service/") != NULL)
			{
				printf("Handling service call request\n");
				// 在这里添加服务调用请求的处理逻辑
			}
		}
	}
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	printf(RED_COLOR "[topicName] = %s [payload] = %s\n\n" RESET_COLOR, topicName, (char *)message->payload);
	if (strstr(topicName, "/ota/device/upgrade/") != NULL)
	{
		printf("Handling firmware upgrade\n");
		// 在这里添加推送固件信息的处理逻辑
	}
	else if (strstr(topicName, "/ext/ntp/") != NULL && strstr(topicName, "/response") != NULL)
	{
		printf("Handling NTP response\n");
		struct devTime data;
		if (parseDevTimeInfo((char *)message->payload, &data) == 0)
		{
			
		}
	}
	else
	{
		parseRecvData(topicName, (char *)message->payload);
	}

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	return 1;
}

void deliveryComplete(void *context, int deliveryToken)
{
	printf("Delivery completed for token: %d\n", deliveryToken);
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
	char *topicName = "test";
	struct verInfoEvt verInfo = {
		.devRegMethod = 10,
		.pileSoftwareVer = "1.0.0",
		.pileHardwareVer = "2.0.0",
		.sdkVer = "3.0.0"};
	char *str1 = createVerInfoEvtRequest("1", &verInfo);
	printf(YELLOW_COLOR "[topicName]\n%s\n[payload]\n%s\n\n" RESET_COLOR, topicName, str1);
	parseRecvData(topicName, str1);
	struct devMduInfoEvt devMduInfo = {
		.netMduInfo = "NetMdu123",
		.netMduSoftVer = "1.0.0",
		.netMduImei = "123456789",
		.smartGun = 10,
		.mduInfoInt = {20, 30},
		.mduInfoString = {"CPUNum123", "Reserved456"}};

	char *str2 = createDevMdunInfoEvtRequest("2", &devMduInfo);
	printf(YELLOW_COLOR "[topicName]\n%s\n[payload]\n%s\n\n" RESET_COLOR, topicName, str2);
	parseRecvData(topicName, str2);
	char *str3 = createAskConfigEvtRequest("3");
	printf(YELLOW_COLOR "[topicName]\n%s\n[payload]\n%s\n\n" RESET_COLOR, topicName, str3);
	parseRecvData(topicName, str3);
	while (1)
	{
		packageRecvHandler();
		sleep(5);
	}

	// MqttConfig mqttConfig;
	// mqttConfig.address = "mqtt.eclipse.org";
	// mqttConfig.client_id = "my_client_id";
	// mqttConfig.username = "my_username";
	// mqttConfig.password = "my_password";
	// mqttConfig.port = 1883;
	// mqttConfig.tls = false;
	// mqttConfig.keepAliveInterval = 60;
	// mqttConfig.ca_path = "/path/to/ca_certificate.pem";
	// mqttConfig.private_key_path = "/path/to/private_key.pem";
	// mqttConfig.certificate_path = "/path/to/certificate.pem";
	// mqttConfig.onConnectionLost = messageArrived;
	// mqttConfig.onMessageArrived = connectionLost;
	// mqttConfig.onDeliveryComplete = NULL;

	return 0;
}
