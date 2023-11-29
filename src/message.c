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


int FirmwareVersionPush(const char *productKey, const char *deviceName, const char *id, const char *version, const char *module)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/ota/device/inform/%s/%s", productKey, deviceName);

    char *send = createFirmwareVersionPush(id, version, module);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int FirmwareRatePush(const char *productKey, const char *deviceName, const char *id, const char *step, const char *desc, const char *module)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/ota/device/progress/%s/%s", productKey, deviceName);

    char *send = createFirmwareRatePush(id, step, desc, module);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int confUpdateSrvReply(const char *productKey, const char *deviceName, const char *id, int code)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/confUpdateSrv_reply", productKey, deviceName);
    char *send = createConfUpdateSrvReply(id, code);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int getConfSrvReply(const char *productKey, const char *deviceName, const char *id, const struct funConfUpdate_srv *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/getConfSrv_reply", productKey, deviceName);

    char *send = createGetFunConfSrvSrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int funConfUpdateSrvReply(const char *productKey, const char *deviceName, const char *id, int code)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/funConfUpdateSrv_reply", productKey, deviceName);

    char *send = createFunConfUpdateSrvReply(id, code);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int getFunConfSrvReply(const char *productKey, const char *deviceName, const char *id, const struct funConfUpdate_srv *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/getFunConfSrv_reply", productKey, deviceName);

    char *send = createGetFunConfSrvSrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int queDataSrvReply(const char *productKey, const char *deviceName, const char *id, const struct queDataSrv *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/queDataSrv_reply", productKey, deviceName);

    char *send = createQueDataSrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int devMaintainSrvReply(const char *productKey, const char *deviceName, const char *id, int ctrlType, int reason)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/devMaintainSrv_reply", productKey, deviceName);

    char *send = createDevMaintainSrvReply(id, ctrlType, reason);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int devMaintainQuerySrvReply(const char *productKey, const char *deviceName, const char *id, int ctrlType, int reason)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/devMaintainQuerySrv_reply", productKey, deviceName);

    char *send = createDevMaintainQuerySrvReply(id, ctrlType, reason);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int feeModelUpdateSrvReply(const char *productKey, const char *deviceName, const char *id, const struct feeModelUpdateSrvRep *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/feeModelUpdateSrv_reply", productKey, deviceName);

    char *send = createFeeModelUpdateSrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int feeModelQuerySrvReply(const char *productKey, const char *deviceName, const char *id, const struct feeModelQuerySrv *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/feeModelQuerySrv_reply", productKey, deviceName);

    char *send = createFeeModelQuerySrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int startChargeSrvReply(const char *productKey, const char *deviceName, const char *id, const struct startChargeSrvRep *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/startChargeSrv_reply", productKey, deviceName);

    char *send = createStartChargeSrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int authResultSrvReply(const char *productKey, const char *deviceName, const char *id, const struct startChargeAuthEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/authResultSrv_reply", productKey, deviceName);

    char *send = createStartChargeAuthEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int stopChargeSrvReply(const char *productKey, const char *deviceName, const char *id, const struct startChargeSrvRep *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/stopChargeSrv_reply", productKey, deviceName);

    char *send = createStopChargeSrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int orderCheckSrvReply(const char *productKey, const char *deviceName, const char *id, const struct orderUpdateEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/orderCheckSrv_reply", productKey, deviceName);

    char *send = createOrderUpdateEvtRequest(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int acOrderlyChargeSrvReply(const char *productKey, const char *deviceName, const char *id, const struct acOrderlyChargeSrvRep *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
    {
        return -1;
    }
    char topic[256]; // 适当选择足够容纳字符串的大小
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/acOrderlyChargeSrv_reply", productKey, deviceName);

    char *send = createAcOrderlyChargeSrvReply(id, data);
    if (send == NULL)
    {
        return -1;
    }

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AskClockSynRequest(const char *productKey, const char *deviceName, const char *id)
{
    if (productKey == NULL || deviceName == NULL)
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

    enqueueSendMessage(id, topic, send);

    free(send);

    return 0;
}

int FirmwareEvRequest(const char *productKey, const char *deviceName, const char *id, const struct firmwareEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int VerInfoEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct verInfoEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}
int DevMdunInfoEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct devMduInfoEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AskConfigEvtRequest(const char *productKey, const char *deviceName, const char *id)
{
    if (productKey == NULL || deviceName == NULL || id == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AskFeeModelEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct askFeeModelEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int StartChargeAuthEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct startChargeAuthEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int OrderUpdateEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct orderUpdateEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int TotalFaultEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct totalFaultEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AcStChEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct acStChEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AcCarInfoEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct acCarInfoEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AcCarConChEvtRequest(const char *productKey, const char *deviceName, const char *id, const struct acCarConChEvt *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AcDeRealItyProperty(const char *productKey, const char *deviceName, const char *id, const struct acDeRealIty *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AcGunRunItyProperty(const char *productKey, const char *deviceName, const char *id, const struct acGunRunIty *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AcGunIdleItyProperty(const char *productKey, const char *deviceName, const char *id, const struct acGunIdleIty *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}

int AcOutMeterItyProperty(const char *productKey, const char *deviceName, const char *id, const struct acOutMeterIty *data)
{
    if (productKey == NULL || deviceName == NULL || id == NULL || data == NULL)
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

    enqueueSendMessage(id, topic, send);
    free(send);

    return 0;
}