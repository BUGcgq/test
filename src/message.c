#include "package.h"
#include "message.h"

int setSystemTime(const struct devTime *data)
{
    // 获取当前设备时间的毫秒数
    struct timeval currentTime;
    if (gettimeofday(&currentTime, NULL) != 0)
    {
        printf("Failed to get device current time\n");
        return -1;
    }

    long long deviceRecvTime = (long long)currentTime.tv_sec * 1000 + currentTime.tv_usec / 1000;

    // 计算真实时间
    long long realTime = (data->serverRecvTime + data->serverSendTime +
                          deviceRecvTime - data->deviceSendTime) /
                         2;

    // 设置系统时间
    struct timeval newTime;
    newTime.tv_sec = realTime / 1000;
    newTime.tv_usec = (realTime % 1000) * 1000;

    if (settimeofday(&newTime, NULL) != 0)
    {
        printf("Failed to set system time\n");
        return -1;
    }

    return 0;
}

char *generateUniqueIdStr()
{
    unsigned int num = 0;

    // 使用 OpenSSL 生成随机数
    if (RAND_bytes((unsigned char *)&num, sizeof(num)) != 1)
    {
        printf("生成随机数出错\n");
        // 如果生成随机数失败，退回到使用时间生成伪随机数
        num = (unsigned int)time(NULL);
    }

    // 取模以确保在指定范围内
    num %= 10000000000; // 10位数字

    // 使用snprintf将整数转换为字符串
    char *result = (char *)malloc(11);  // 10位数字 + 结尾的null字符
    snprintf(result, 11, "%010u", num); // %010u 保证输出的数字是10位，不足前面用0填充

    return result;
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
void processServiceHandle(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const char *method, const char *params) // 处理服务数据的函数
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || params == NULL || method == NULL)
    {
        return;
    }

    if (strstr("confUpdateSrv", method) == 0)
    {
        struct confUpdateSrv confUpdateSrv;
        if (parseConfUpdateSrv(params, &confUpdateSrv) == 0)
        {
            confUpdateSrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("getConfSrv", method) == 0)
    {
        if (parseGetConfSrv(params) == 0)
        {
            getConfSrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("funConfUpdateSrv", method) == 0)
    {
        struct funConfUpdate_srv funConfUpdate;
        if (parseFunConfUpdateSrv(params, &funConfUpdate) == 0)
        {
            funConfUpdateSrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("getFunConfSrv", method) == 0)
    {
        if (parseGetFunConfSrv(params) == 0)
        {
            getFunConfSrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("queDataSrv", method) == 0)
    {
        struct queDataSrv queDataSrv;
        if (parseQueDataSrv(params, &queDataSrv) == 0)
        {
            queDataSrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("devMaintainSrv", method) == 0)
    {
        if (parseDevMaintainSrv(params) == 0)
        {
            devMaintainSrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("devMaintainQuerySrv", method) == 0)
    {
        if (parseDevMaintainQuerySrv(params) == 0)
        {
            devMaintainQuerySrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("feeModelUpdateSrv", method) == 0)
    {
        struct feeModelUpdateSrv feeModelUpdateSrv;
        if (parseFeeModelUpdateSrv(params, &feeModelUpdateSrv) == 0)
        {
            feeModelUpdateSrvReply(client, productKey, deviceName, id);
        }
    }
    else if (strstr("feeModelQuerySrv", method) == 0)
    {
        if (parseFeeModelQuerySrv(params) == 0)
        {
            feeModelQuerySrvReply(client, productKey, deviceName, id);
        }
    }

    else if (strstr("startChargeSrv", method) == 0)
    {
        struct startChargeSrv startChargeSrv;
        if (parseStartChargeSrv(params, &startChargeSrv) == 0)
        {
            startChargeSrvReply(client, productKey, deviceName, id);
        }
    }
    else if (strstr("authResultSrv", method) == 0)
    {
        struct authResultSrv authResultSrv;
        if (parseAuthResultSrv(params, &authResultSrv) == 0)
        {
            authResultSrvReply(client, productKey, deviceName, id);
        }
    }
    else if (strstr("stopChargeSrv", method) == 0)
    {
        struct stopChargeSrv stopChargeSrv;
        if (parseStopChargeSrv(params, &stopChargeSrv) == 0)
        {
            stopChargeSrvReply(client, productKey, deviceName, id);
        }
    }
    else if (strstr("orderCheckSrv", method) == 0)
    {
        struct orderCheckSrv orderCheckSrv;
        if (parseOrderCheckSrv(params, &orderCheckSrv) == 0)
        {
            orderCheckSrvReply(client, productKey, deviceName, id);
        }
    }
    else if (strstr("acOrderlyChargeSrv", method) == 0)
    {
        struct acOrderlyChargeSrv acOrderlyChargeSrv;
        if (parseAcOrderlyChargeSrv(params, &acOrderlyChargeSrv) == 0)
        {
            orderCheckSrvReply(client, productKey, deviceName, id);
        }
    }
}

int confUpdateSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/confUpdateSrv_reply", productKey, deviceName);
    int code = 1;
    char *send = createConfUpdateSrvReply(id, code);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int getConfSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/getConfSrv_reply", productKey, deviceName);

    struct funConfUpdate_srv data;

    char *send = createGetFunConfSrvSrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int funConfUpdateSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/funConfUpdateSrv_reply", productKey, deviceName);

    int code;

    char *send = createFunConfUpdateSrvReply(id, code);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int getFunConfSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/getFunConfSrv_reply", productKey, deviceName);

    struct funConfUpdate_srv data;

    char *send = createGetFunConfSrvSrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int queDataSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/queDataSrv_reply", productKey, deviceName);

    struct queDataSrv data;

    char *send = createQueDataSrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int devMaintainSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/devMaintainSrv_reply", productKey, deviceName);

    int ctrlType;
    int reason;

    char *send = createDevMaintainSrvReply(id, ctrlType, reason);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int devMaintainQuerySrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/devMaintainQuerySrv_reply", productKey, deviceName);

    int ctrlType;
    int reason;

    char *send = createDevMaintainQuerySrvReply(id, ctrlType, reason);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int feeModelUpdateSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/feeModelUpdateSrv_reply", productKey, deviceName);

    struct feeModelUpdateSrvRep data;

    char *send = createFeeModelUpdateSrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int feeModelQuerySrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/feeModelQuerySrv_reply", productKey, deviceName);

    struct feeModelQuerySrv data;

    char *send = createFeeModelQuerySrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int startChargeSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/startChargeSrv_reply", productKey, deviceName);

    struct startChargeSrvRep data;

    char *send = createStartChargeSrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int authResultSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/authResultSrv_reply", productKey, deviceName);

    struct startChargeAuthEvt data;

    char *send = createStartChargeAuthEvtRequest(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int stopChargeSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/stopChargeSrv_reply", productKey, deviceName);

    struct startChargeSrvRep data;

    char *send = createStopChargeSrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int orderCheckSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/orderCheckSrv_reply", productKey, deviceName);

    struct orderUpdateEvt data;

    char *send = createOrderUpdateEvtRequest(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int acOrderlyChargeSrvReply(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/acOrderlyChargeSrv_reply", productKey, deviceName);

    struct acOrderlyChargeSrvRep data;

    char *send = createAcOrderlyChargeSrvReply(id, &data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AskClockSynRequest(MQTTClient client, const char *productKey, const char *deviceName)
{
    if (client == NULL || productKey == NULL || deviceName == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/ext/ntp/%s/%s/request", productKey, deviceName);

    struct timeval tv;
    gettimeofday(&tv, NULL);

    // 将秒和纳秒部分转换为毫秒
    long long currentTimeMillis = (long long)(tv.tv_sec) * 1000 + (long long)(tv.tv_usec) / 1000;
    char *send = createAskClockSynRequest(currentTimeMillis);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int FirmwareEvRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct firmwareEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createFirmwareEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int VerInfoEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct verInfoEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createVerInfoEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}
int DevMdunInfoEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct devMduInfoEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createDevMdunInfoEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AskConfigEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createAskConfigEvtRequest(id);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AskFeeModelEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct askFeeModelEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createAskFeeModelEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int StartChargeAuthEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct startChargeAuthEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createStartChargeAuthEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int OrderUpdateEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct orderUpdateEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createOrderUpdateEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int TotalFaultEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct totalFaultEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createTotalFaultEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AcStChEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct acStChEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createAcStChEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AcCarInfoEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct acCarInfoEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createAcCarInfoEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AcCarConChEvtRequest(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct acCarConChEvt *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/model/up_raw", productKey, deviceName);

    char *send = createAcCarConChEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AcDeRealItyProperty(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct acDeRealIty *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/event/property/post", productKey, deviceName);

    char *send = createAcDeRealItyProperty(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AcGunRunItyProperty(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct acGunRunIty *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/event/property/post", productKey, deviceName);

    char *send = createAcGunRunItyProperty(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AcGunIdleItyProperty(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct acGunIdleIty *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/event/property/post", productKey, deviceName);

    char *send = createAcGunIdleItyProperty(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}

int AcOutMeterItyProperty(MQTTClient client, const char *productKey, const char *deviceName, const char *id, const struct acOutMeterIty *data)
{
    if (client == NULL || productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/event/property/post", productKey, deviceName);

    char *send = createAcOutMeterItyProperty(id, data);
    if (send == NULL)
    {
        return -1;
    }

    int ret = publish_message(client, topic, send, 1);
    free(send);

    return ret;
}