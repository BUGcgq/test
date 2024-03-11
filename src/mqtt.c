#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mqtt.h"
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


int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    printf(RED_COLOR "[topicName] = %s [payload] = %s\n\n" RESET_COLOR, topicName, (char *)message->payload);

    return 1;
}

// 回调函数：连接丢失
void connectionLost(void *context, char *cause)
{

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


    return 0;
}