#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mqtt.h"
#include "message.h"
#include "list.h"

MQTTClient client = NULL;
static int connect_state = 0;
char productKey[64];
char deviceName[64];
int g_time_sync_success = 0; // 对时标志位

// 初始化MQTT连接
int connect_mqtt(MQTTClient *client, MqttConfig *config)
{
    int rc;
    char url[100] = {0};
    const char *protocol = (config->tls) ? "tls://" : "tcp://";

    snprintf(url, sizeof(url), "%s%s:%d", protocol, config->address, config->port);
    printf("url = %s ,client_id = %s\n", url, config->client_id);
    if ((rc = MQTTClient_create(client, url, config->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create mqtt client, return code %d\n", rc);
        return -1;
    }

    if ((rc = MQTTClient_setCallbacks(*client, NULL, config->onConnectionLost, config->onMessageArrived, config->onDeliveryComplete)) != MQTTCLIENT_SUCCESS)
    {
        printf("MQTT Failed to set callbacks, return code %d\n", rc);
        return -1;
    }

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    // 设置连接选项
    conn_opts.keepAliveInterval = config->keepAliveInterval;
    conn_opts.cleansession = 1;
    conn_opts.username = config->username;
    conn_opts.password = config->password;
    conn_opts.ssl = NULL;

    if (config->tls)
    {
        MQTTClient_SSLOptions ssl_opts = MQTTClient_SSLOptions_initializer;

        // 配置TLS选项
        ssl_opts.trustStore = (config->ca_path != NULL) ? config->ca_path : NULL;
        ssl_opts.privateKey = (config->private_key_path != NULL) ? config->private_key_path : NULL;
        ssl_opts.keyStore = (config->certificate_path != NULL) ? config->certificate_path : NULL;
        ssl_opts.sslVersion = MQTT_SSL_VERSION_DEFAULT;

        conn_opts.ssl = &ssl_opts;
    }

    // 连接到MQTT服务器
    if ((rc = MQTTClient_connect(*client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
    }

    return MQTTCLIENT_SUCCESS;
}

// 发布消息
int publish_message(MQTTClient client, const char *topic, const char *payload, int qos)
{
    MQTTClient_deliveryToken token;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;

    pubmsg.payload = (void *)payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = qos;
    pubmsg.retained = 0;

    if (MQTTClient_publishMessage(client, topic, &pubmsg, &token) != MQTTCLIENT_SUCCESS)
    {
        printf("MQTT Failed to publish message\n");
        return -1;
    }

    if (MQTTClient_waitForCompletion(client, token, 1000) != MQTTCLIENT_SUCCESS)
    {
        printf("MQTT Failed to receive ACK for published message\n");
        return -1;
    }

    return MQTTCLIENT_SUCCESS;
}

// 订阅主题
int subscribe_topic(MQTTClient client, char *topic, int qos)
{
    if (client == NULL)
    {
        printf("MQTT client is NULL\n");
        return -1;
    }
    // 订阅主题
    int rc = MQTTClient_subscribe(client, topic, qos);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("MQTT Failed to subscribe to topic %s, return code %d\n", topic, rc);
        return -1;
    }

    return 0;
}

// 取消订阅主题
int unsubscribe_topic(MQTTClient client, const char *topic)
{
    if (MQTTClient_unsubscribe(client, topic) != MQTTCLIENT_SUCCESS)
    {
        printf("MQTT Failed to unsubscribe from topic %s\n", topic);
        return -1;
    }

    return MQTTCLIENT_SUCCESS;
}

// 断开 MQTT 连接
void disconnect_mqtt(MQTTClient client)
{
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
}

int init_topic(MQTTClient client, const char *productKey, const char *deviceName)
{
    if (productKey == NULL || deviceName == NULL)
    {
        return -1;
    }
    const char *identifiers[] =
        {
            "confUpdateSrv",
            "getConfSrv",
            "funConfUpdateSrv",
            "getFunConfSrv",
            "queDataSrv",
            "devMaintainSrv",
            "devMaintainQuerySrv",
            "feeModelUpdateSrv",
            "feeModelQuerySrv",
            "startChargeSrv",
            "authResultSrv",
            "stopChargeSrv",
            "orderCheckSrv",
            "acOrderlyChargeSrv"};
    int result;
    char topic[256];

    snprintf(topic, sizeof(topic), "/ota/device/upgrade/%s/%s", productKey, deviceName);
    result = subscribe_topic(client, topic, 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/ota/firmware/get_reply", productKey, deviceName);
    result = subscribe_topic(client, topic, 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic), "/ext/ntp/%s/%s/response", productKey, deviceName);
    result = subscribe_topic(client, topic, 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/event/property/post_reply", productKey, deviceName);
    result = subscribe_topic(client, topic, 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/property/set", productKey, deviceName);
    result = subscribe_topic(client, topic, 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw_reply", productKey, deviceName);
    result = subscribe_topic(client, topic, 1);

    int i;
    for (i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); i++)
    {
        memset(topic, 0, sizeof(topic));
        snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/%s", productKey, deviceName, identifiers[i]);
        result = subscribe_topic(client, topic, 1);
    }

    return result; // 所有主题订阅成功，返回0
}

int parsefirmwareInfo(const char *payload, struct firmwareInfo *data)
{
    if (payload == NULL || data == NULL)
    {
        return -1;
    }

    json_object *jsonObj = json_tokener_parse(payload);
    if (jsonObj != NULL)
    {
        json_object_object_foreach(jsonObj, key, val)
        {
            if (strcmp(key, "id") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->id, json_object_get_string(val), sizeof(data->id) - 1);
            }
            if (strcmp(key, "message") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->message, json_object_get_string(val), sizeof(data->message) - 1);
            }
            if (strcmp(key, "code") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->code, json_object_get_string(val), sizeof(data->code) - 1);
            }
            if (strcmp(key, "version") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->version, json_object_get_string(val), sizeof(data->version) - 1);
            }
            else if (strcmp(key, "size") == 0 && json_object_get_type(val) == json_type_int)
            {
                data->size = json_object_get_int(val);
            }
            else if (strcmp(key, "url") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->url, json_object_get_string(val), sizeof(data->url) - 1);
            }
            else if (strcmp(key, "sign") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->sign, json_object_get_string(val), sizeof(data->sign) - 1);
            }
            else if (strcmp(key, "signMethod") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->signMethod, json_object_get_string(val), sizeof(data->signMethod) - 1);
            }
            else if (strcmp(key, "md5") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->md5, json_object_get_string(val), sizeof(data->md5) - 1);
            }
            else if (strcmp(key, "module") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->module, json_object_get_string(val), sizeof(data->module) - 1);
            }
        }

        json_object_put(jsonObj);

        return 0;
    }

    return -1;
}

int parseServerRepResult(const char *payload, struct serverRepResult *data)
{
    if (payload == NULL || data == NULL)
    {
        return -1;
    }

    json_object *jsonObj = json_tokener_parse(payload);
    if (jsonObj != NULL)
    {
        json_object_object_foreach(jsonObj, key, val)
        {
            if (strcmp(key, "id") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->id, json_object_get_string(val), sizeof(data->id) - 1);
            }
            else if (strcmp(key, "Code") == 0 && json_object_get_type(val) == json_type_int)
            {
                data->Code = json_object_get_int(val);
            }
            else if (strcmp(key, "Data") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->Data, json_object_get_string(val), sizeof(data->Data) - 1);
            }
        }

        json_object_put(jsonObj);

        return 0;
    }

    return -1;
}

int parseRecvData(const char *payload, RecvMessage *data) // 解析服务器下放的事件，服务，属性数据的包
{
    if (payload == NULL || data == NULL)
    {
        return -1;
    }

    json_object *jsonObj = json_tokener_parse(payload);
    if (jsonObj != NULL)
    {
        json_object_object_foreach(jsonObj, key, val)
        {
            if (strcmp(key, "id") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->id, json_object_get_string(val), sizeof(data->id) - 1);
            }
            else if (strcmp(key, "version") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->version, json_object_get_string(val), sizeof(data->version) - 1);
            }
            else if (strcmp(key, "method") == 0 && json_object_get_type(val) == json_type_string)
            {
                strncpy(data->method, json_object_get_string(val), sizeof(data->method) - 1);
            }
            else if (strcmp(key, "params") == 0 && json_object_get_type(val) == json_type_object)
            {
                const char *params_str = json_object_to_json_string(val);
                strncpy(data->params, params_str, sizeof(data->params) - 1);
            }
        }

        json_object_put(jsonObj);

        return 0;
    }

    return -1;
}
void processServiceHandle(const char *productKey, const char *deviceName, const char *id, const char *method, const char *params) // 处理服务数据的函数
{
    if (productKey == NULL || deviceName == NULL || id == NULL || params == NULL || method == NULL)
    {
        return;
    }

    if (strstr("confUpdateSrv", method) == 0) // 配置更新服务
    {
        struct confUpdateSrv confUpdateSrv;
        if (parseConfUpdateSrv(params, &confUpdateSrv) == 0)
        {
            int code;
            confUpdateSrvReply(productKey, deviceName, id, code);
        }
    }

    else if (strstr("getConfSrv", method) == 0) // 查询充电桩配置服务
    {
        if (parseGetConfSrv(params) == 0)
        {
            struct funConfUpdate_srv data;
            getConfSrvReply(productKey, deviceName, id, &data);
        }
    }

    else if (strstr("funConfUpdateSrv", method) == 0) // 功能配置更新服务
    {
        struct funConfUpdate_srv funConfUpdate;
        if (parseFunConfUpdateSrv(params, &funConfUpdate) == 0)
        {
            int code;
            funConfUpdateSrvReply(productKey, deviceName, id, code);
        }
    }

    else if (strstr("getFunConfSrv", method) == 0) // 查询充电桩功能配置服务
    {
        if (parseGetFunConfSrv(params) == 0)
        {
            struct funConfUpdate_srv data;
            getFunConfSrvReply(productKey, deviceName, id, &data);
        }
    }

    else if (strstr("queDataSrv", method) == 0) // 充电桩日志查询
    {
        struct queDataSrv queDataSrv;
        if (parseQueDataSrv(params, &queDataSrv) == 0)
        {
            struct queDataSrv data;
            queDataSrvReply(productKey, deviceName, id, &data);
        }
    }

    else if (strstr("devMaintainSrv", method) == 0) // 充电桩维护服务
    {
        if (parseDevMaintainSrv(params) == 0)
        {
            int ctrlType;
            int reason;
            devMaintainSrvReply(productKey, deviceName, id, ctrlType, reason);
        }
    }

    else if (strstr("devMaintainQuerySrv", method) == 0) // 充电桩维护状态查询
    {
        if (parseDevMaintainQuerySrv(params) == 0)
        {
            int ctrlType;
            int reason;
            devMaintainQuerySrvReply(productKey, deviceName, id, ctrlType, reason);
        }
    }

    else if (strstr("feeModelUpdateSrv", method) == 0) // 计量计费模型更新
    {
        struct feeModelUpdateSrv feeModelUpdateSrv;
        if (parseFeeModelUpdateSrv(params, &feeModelUpdateSrv) == 0)
        {
            struct feeModelUpdateSrvRep data;
            feeModelUpdateSrvReply(productKey, deviceName, id, &data);
        }
    }
    else if (strstr("feeModelQuerySrv", method) == 0) // 计量计费模查询服务
    {
        if (parseFeeModelQuerySrv(params) == 0)
        {
            struct feeModelQuerySrv data;
            feeModelQuerySrvReply(productKey, deviceName, id, &data);
        }
    }

    else if (strstr("startChargeSrv", method) == 0) // 启动充电服务
    {
        struct startChargeSrv startChargeSrv;
        if (parseStartChargeSrv(params, &startChargeSrv) == 0)
        {
            struct startChargeSrvRep data;
            startChargeSrvReply(productKey, deviceName, id, &data);
        }
    }
    else if (strstr("authResultSrv", method) == 0) // 启动充电鉴权事件
    {
        struct authResultSrv authResultSrv;
        if (parseAuthResultSrv(params, &authResultSrv) == 0)
        {
            struct startChargeAuthEvt data;
            authResultSrvReply(productKey, deviceName, id, &data);
        }
    }
    else if (strstr("stopChargeSrv", method) == 0) // 上级系统停止充电
    {
        struct stopChargeSrv stopChargeSrv;
        if (parseStopChargeSrv(params, &stopChargeSrv) == 0)
        {
            struct startChargeSrvRep data;
            stopChargeSrvReply(productKey, deviceName, id, &data);
        }
    }
    else if (strstr("orderCheckSrv", method) == 0) // 有序充电策略下发
    {
        struct orderCheckSrv orderCheckSrv;
        if (parseOrderCheckSrv(params, &orderCheckSrv) == 0)
        {
            struct orderUpdateEvt data;
            orderCheckSrvReply(productKey, deviceName, id, &data);
        }
    }
    else if (strstr("acOrderlyChargeSrv", method) == 0) // 有序充电策略下发
    {
        struct acOrderlyChargeSrv acOrderlyChargeSrv;
        if (parseAcOrderlyChargeSrv(params, &acOrderlyChargeSrv) == 0)
        {
            struct acOrderlyChargeSrvRep data;
            acOrderlyChargeSrvReply(productKey, deviceName, id, &data);
        }
    }
}
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf(RED_COLOR "[topicName] = %s [payload] = %s\n\n" RESET_COLOR, topicName, (char *)message->payload);
    if (strstr(topicName, "/ota/device/upgrade/") != NULL)
    {
        printf("服务器推送固件信息\n");
        struct firmwareInfo data;
        memset(&data, 0, sizeof(data));
        parsefirmwareInfo((char *)message->payload, &data);
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
                g_time_sync_success = 1;
            }
        }
    }
    else if (strstr(topicName, "/thing/service/") != NULL)
    {
        RecvMessage data;
        memset(&data, 0, sizeof(data));
        if (parseRecvData((char *)message->payload, &data) == 0)
        {
            enqueueRecvMessage(data.id, data.version, data.method, data.params);
        }
    }
    else
    {
        struct serverRepResult data;
        parseServerRepResult(message->payload, &data);
        updateSendMessageStatus(data.id, 1);
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
        client == NULL;
    }

    connect_state = 0;
}

void recvServiceHandle()
{
    if (IsRecvMessage())
    {
        char id[40];
        char version[40];
        char method[40];
        char params[MAX_DATA_SIZE];
        if (dequeueRecvMessage(id, version, method, params))
        {
            processServiceHandle(productKey, deviceName, id, method, params);
            memset(id, 0, sizeof(id));
            memset(version, 0, sizeof(version));
            memset(method, 0, sizeof(method));
            memset(params, 0, sizeof(params));
        }
    }
}
/**
 * @description: 判断是否超时
 * @param startTime判断的时间
 */
static bool isTimeout(time_t startTime)
{
    time_t currentTime = time(NULL);
    return (currentTime - startTime >= MAX_TIMEOUT_SECONDS);
}
/**
 * @description: 信息发送控制线程
 */
void *sendMessageToServer(void *arg)
{

    char id[40];
    char topic[128];
    char payload[MAX_DATA_SIZE];
    int status;
    time_t sendTime;
    bool messageSent = false; // 标志变量，追踪消息是否已发送
    int timeoutCount = 0;     // 超时次数标志位
    while (1)
    {
        if (dequeueSendMessage(id, topic, payload, &status))
        {
            if (!messageSent && payload != NULL)
            {
                printf(YELLOW_COLOR "[SEND] = %s\n\n" RESET_COLOR, (char *)payload);
                publish_message(client,topic,payload,1);
                messageSent = true;
                sendTime = time(NULL);
            }

            if (isTimeout(sendTime) && messageSent)
            {
                printf(RED_COLOR "Timeout for id: %s\n\n" RESET_COLOR, id);
                if (status != 1)
                {
                    status = 2;     // 2 表示超时
                    timeoutCount++; // 增加超时次数
                }
            }

            if (status == 1 || status == 2)
            {
                if (status == 1)
                {
                    timeoutCount = 0; // 重置超时次数
                }

                deleteSendMessage(id);
                messageSent = false;
                status = 0;
            }
            memset(id, 0, 40);
            memset(topic, 0, 128);
            memset(payload, 0, MAX_DATA_SIZE);
        }
        sleep(1); // 等待一段时间后重试
    }

    return NULL;
}

void *pv_mqtt_comm_thread(void *arg)
{
    if (arg == NULL)
    {
        printf("Error: MqttConfig pointer is NULL\n");
        return NULL;
    }

    MqttConfig *mqtt_config = (MqttConfig *)arg;
    initSendMessageQueue();
    initRecvMessageQueue(); // 初始化消息队列
    pthread_t SendMessageThread;
    if (pthread_create(&SendMessageThread, NULL, sendMessageToServer, client) != 0)
    {
        fprintf(stderr, "Failed to create server thread\n");
        return;
    }
    pthread_detach(SendMessageThread);
    int i;
    int gun_state_bak;
    int feel_flag;
    unsigned int u32_sync_time = 0;
    unsigned int u32_failed_sync_count = 0;
    unsigned int u32_last_online_time = 0;
    while (1)
    {
        int rc = connect_mqtt(&client, mqtt_config);
        if (rc == MQTTCLIENT_SUCCESS)
        {
            if (init_topic(client, productKey, deviceName) == -1)
            {
                sleep(30);
                continue;
            }
            connect_state = 1;
            u32_last_online_time = 0;
        }
        else
        {
            printf("Failed to connect to MQTT server. Retrying...\n");
        }

        while (connect_state == 1)
        {

            // if (feel_flag)
            // {
            // 	if (600)
            // 	{
            // 		printf("请求计费模型\n");
            // 	}
            // 	sleep(1);
            // 	continue;
            // }
            // if (u32_last_online_time == 0) // 充电桩每次上线
            // {
            // 	u32_sync_time = u32_inc_get_system_time_ms();
            // }
            // if (u32_inc_get_diff_time_ms(&u32_sync_time) > 24 * 60 * 60 * 1000) // 每24小时对时一次
            // {
            // 	u32_sync_time = u32_inc_get_system_time_ms();
            // }
            // if (!g_time_sync_success) // 对时失败后每10秒对时1次，对时三次未成功上报对时故障
            // {
            // 	if (u32_inc_get_diff_time_ms(&u32_sync_time) > 10 * 1000)
            // 	{
            // 		u32_failed_sync_count++;
            // 		if (u32_failed_sync_count >= 3)
            // 		{
            // 		}
            // 		u32_sync_time = u32_inc_get_system_time_ms();
            // 	}
            // }
            // else
            // {
            // 	u32_failed_sync_count = 0;
            // }
            // if (u32_failed_sync_count >= 3 && u32_inc_get_diff_time_ms(&u32_sync_time) > 2 * 60 * 60 * 1000) // 对时故障后每两小时对时一次，对时成功故障消除
            // {
            // 	u32_sync_time = u32_inc_get_system_time_ms();
            // }
            // if (1)
            // {
            // 	printf("对时\n");
            // }

            // if (600)
            // {
            // 	printf("交流桩实时监测数据\n");
            // }

            // if (90)
            // {
            // 	printf("交流桩充电枪充电中监测数据\n");
            // }
            // if (180)
            // {
            // 	printf("交流桩充电枪非充电中监测数据\n");
            // }
            // if (60)
            // {
            // 	printf("交流输出电表底值监测数据\n");
            // }
            // if (gun_state_bak)
            // {
            // 	printf("事件：交流充电枪状态变更事件\n");
            // }
            // if (1)
            // {
            // 	printf("有未上传订单\n");
            // }

            sleep(1);
        }

        sleep(30);
    }

    return NULL;
}

int mqtt_comm_init()
{
    MqttConfig mqttConfig;
    mqttConfig.address = strdup("mqtt.example.com");
    mqttConfig.client_id = strdup("client123");
    mqttConfig.username = strdup("user");
    mqttConfig.password = strdup("pass");
    mqttConfig.port = 80;
    mqttConfig.tls = 0;
    mqttConfig.keepAliveInterval = 60;
    mqttConfig.ca_path = strdup("/path/to/ca_cert.pem");
    mqttConfig.private_key_path = strdup("/path/to/private_key.pem");
    mqttConfig.certificate_path = strdup("/path/to/certificate.pem");
    mqttConfig.onMessageArrived = messageArrived;
    mqttConfig.onConnectionLost = connectionLost;
    mqttConfig.onDeliveryComplete = NULL;

    pthread_t mqttCommThread;
    if (pthread_create(&mqttCommThread, NULL, sendMessageToServer, &mqttConfig) != 0)
    {
        fprintf(stderr, "Failed to create server thread\n");
        return -1;
    }

    pthread_detach(mqttCommThread);

    return 0;
}