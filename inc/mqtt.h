#ifndef __MQTT__H__
#define __MQTT__H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <MQTTClient.h>
#include <MQTTAsync.h>
#include <MQTTProperties.h>
#include <MQTTReasonCodes.h>


// 定义函数指针类型
typedef void (*ConnectionLostCallback)(void *context, char *cause);
typedef int (*MessageArrivedCallback)(void *context, char *topicName, int topicLen, MQTTClient_message *message);
typedef void (*DeliveryCompleteCallback)(void *context, MQTTClient_deliveryToken dt);

// 定义结构体
typedef struct 
{
    const char* address;
    const char* client_id;
    const char* username;
    const char* password;
    int         port;
    bool        tls;  // 是否使用TLS
    int         keepAliveInterval;
    const char* ca_path;  // CA证书路径
    const char* private_key_path;  // 私钥路径
    const char* certificate_path;  // 证书路径
    // 函数指针
    ConnectionLostCallback onConnectionLost;
    MessageArrivedCallback onMessageArrived;
    DeliveryCompleteCallback onDeliveryComplete;
} MqttConfig;


int connect_mqtt(MQTTClient *client, MqttConfig *config);
int subscribe_topic(MQTTClient client, char *topic, int qos);
int unsubscribe_topic(MQTTClient client, const char *topic);
int publish_message(MQTTClient client, const char *topic, const char *payload, int qos);
void disconnect_mqtt(MQTTClient client);
int mqtt_comm_init();

#ifdef __cplusplus
}
#endif

#endif