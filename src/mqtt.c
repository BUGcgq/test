#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mqtt.h"

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：connect_mqtt
 * 描    述: 初始化MQTT连接
 *
 * 参    数: client - [MQTTClient]
 * 参    数: config - [配置结构体]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */

int connect_mqtt(MQTTClient *client, MqttConfig *config, MQTTClient_connectionLost *onConnectionLost,
                 MQTTClient_messageArrived *onMessageArrived,
                 MQTTClient_deliveryComplete *onDeliveryComplete)
{
    if (client == NULL || config == NULL || onConnectionLost == NULL || onMessageArrived == NULL)
    {
        printf("Invalid input parameters.\n");
        return -1;
    }

    int rc;
    char url[100] = {0};
    const char *protocol = (config->tls) ? "ssl://" : "tcp://";

    snprintf(url, sizeof(url), "%s%s:%d", protocol, config->address, config->port);
    printf("Connecting to URL: %s , Client ID: %s\n", url, config->client_id);

    rc = MQTTClient_create(client, url, config->client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create mqtt client, return code %d\n", rc);
        return -1;
    }

    rc = MQTTClient_setCallbacks(*client, NULL, onConnectionLost, onMessageArrived, onDeliveryComplete);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
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
        ssl_opts.enableServerCertAuth = (config->ca_path != NULL) ? 1 : 0; // 如果trustStore不为NULL，则启用服务器证书认证，否则禁用
        ssl_opts.trustStore = (config->ca_path != NULL) ? config->ca_path : NULL;
        ssl_opts.privateKey = (config->private_key_path != NULL) ? config->private_key_path : NULL;
        ssl_opts.keyStore = (config->certificate_path != NULL) ? config->certificate_path : NULL;
        ssl_opts.sslVersion = MQTT_SSL_VERSION_DEFAULT;

        conn_opts.ssl = &ssl_opts;
        conn_opts.ssl = &ssl_opts;
    }

    // 连接到MQTT服务器
    rc = MQTTClient_connect(*client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
    }

    printf("Connected successfully!\n");

    return MQTTCLIENT_SUCCESS;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：publish_message
 * 描    述: 发布消息
 *
 * 参    数: client - [MQTTClient]
 * 参    数: topic - [主题]
 * 参    数: payload - [信息]
 * 参    数: qos - [策略]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */

int publish_message(MQTTClient client, const char *topic, const char *payload, int qos)
{
    if (client == NULL)
    {
        return -1;
    }

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
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：subscribe_topic
 * 描    述: 订阅主题
 *
 * 参    数: client - [MQTTClient]
 * 参    数: topic - [主题]
 * 参    数: qos - [策略]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
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
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：unsubscribe_topic
 * 描    述: 取消订阅主题
 *
 * 参    数: client - [MQTTClient]
 * 参    数: topic - [主题]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int unsubscribe_topic(MQTTClient client, const char *topic)
{
    if (client == NULL)
    {
        return -1;
    }

    if (MQTTClient_unsubscribe(client, topic) != MQTTCLIENT_SUCCESS)
    {
        printf("MQTT Failed to unsubscribe from topic %s\n", topic);
        return -1;
    }

    return MQTTCLIENT_SUCCESS;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：disconnect_mqtt
 * 描    述: 断开连接
 *
 * 参    数: client - [MQTTClient]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void disconnect_mqtt(MQTTClient client)
{
    if (client == NULL)
    {
        return;
    }

    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
}


int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf("[RECV] = topic : %s\npayload : %s\n\n", topicName, (char *)message->payload);
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-26
 * 函 数 名：connectionLost
 * 描    述: 连接丢失回调
 *
 * 参    数: context - [参数说明]
 * 参    数: cause - [原因]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void connectionLost(void *context, char *cause)
{
}

// int main()
// {
//     MqttConfig config;
//     config.address = strdup("i80f1fe0.ala.cn-hangzhou.emqxsl.cn");
//     config.client_id = strdup("increase");
//     config.username = strdup("chenguiquan");
//     config.password = strdup("123456");
//     config.port = 8883;
//     config.tls = 1;
//     config.keepAliveInterval = 60;
//     config.ca_path = NULL;

//     MQTTClient client = NULL;

//     int ret = -1;
//     if (connect_mqtt(&client, &config, connectionLost, messageArrived, NULL) == MQTTCLIENT_SUCCESS)
//     {
//         printf("连接服务器成功\n");
//     }
//     else
//     {
//         return;
//     }

//     char cmd[10];
//     while (1)
//     {
//         printf("请输入命令(1 - 订阅主题,2 - 发布消息,3 - 退出）: ");
//         scanf("%s", cmd);

//         if (strcmp(cmd, "1") == 0)
//         {
//             char topic[100];
//             int qos = 1; 
//             printf("请输入要订阅的主题: ");
//             scanf("%s", topic);
//             subscribe_topic(client, topic, qos);
//         }
//         else if (strcmp(cmd, "2") == 0)
//         {
//             char topic[100];
//             char payload[100];
//             int qos = 1; 
//             printf("请输入要发布的主题: ");
//             scanf("%s", topic);
//             printf("请输入要发布的消息: ");
//             scanf("%s", payload);
//             publish_message(client, topic, payload, qos);
//         }
//         else if (strcmp(cmd, "3") == 0)
//         {
//             break;
//         }
//         else
//         {
//             printf("无效的命令，请重新输入.\n");
//         }

//         // 清空输入缓冲
//         while (getchar() != '\n');
//     }

//     return 0;
// }