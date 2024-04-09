#ifndef __TCU_MQTT__H__
#define __TCU_MQTT__H__

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


// 定义回调函数类型
typedef void (*mqtt_connection_lost_cb)(void* context, char* cause);
typedef int (*mqtt_message_arrived_cb)(void* context, char* topicName, int topicLen, MQTTClient_message* message);
typedef void (*mqtt_delivery_complete_cb)(void* context, MQTTClient_deliveryToken dt);

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
} MqttConfig;


int connect_mqtt(MQTTClient *client, MqttConfig *config,MQTTClient_connectionLost *onConnectionLost,
                 MQTTClient_messageArrived *onMessageArrived,
                 MQTTClient_deliveryComplete *onDeliveryComplet);
int subscribe_topic(MQTTClient client, char *topic, int qos);
int unsubscribe_topic(MQTTClient client, const char *topic);
int publish_message(MQTTClient client, const char *topic, const char *payload, int qos);
void disconnect_mqtt(MQTTClient client);

#ifdef __cplusplus
}
#endif

#endif
