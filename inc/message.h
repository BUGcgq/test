#ifndef ___MESSAGE__H__
#define ___MESSAGE__H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <json-c/json.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include <openssl/rand.h>
#include "mqtt.h"
#include "package.h"

#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR "\033[36m"
#define WHITE_COLOR "\033[37m"

#define RESET_COLOR "\033[0m"

#define MAX_DATA_SIZE                 512//缓冲区大小  

typedef struct RecvMessage
{
    char id[40];
    char version[40];
    char method[40];
    char params[MAX_DATA_SIZE];
}RecvMessage;

int setSystemTime(const struct devTime *data);//对时
char *generateUniqueIdStr();//生成唯一id
int parseRecvData(const char *payload, RecvMessage *data); // 解析服务器服务数据的包
void processServiceHandle(MQTTClient client, const char *productKey, const char *deviceName,const char *id, const char *method, const char *params);//处理服务器服务的包
//回复服务
int confUpdateSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复配置更新服务
int getConfSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复查询充电桩配置服务
int funConfUpdateSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复功能配置更新
int getFunConfSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复查询充电桩功能配置服务
int queDataSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复充电桩日志查询
int devMaintainSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复充电桩维护服务
int devMaintainQuerySrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复充电桩维护状态查询
int feeModelUpdateSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复计量计费模型更新
int feeModelQuerySrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复计量计费模查询服务
int startChargeSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复启动充电服务
int authResultSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复启动充电鉴权事件
int stopChargeSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复上级系统停止充电
int orderCheckSrvReply(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//回复有序充电策略下发
//事件请求
int AskClockSynRequest(MQTTClient client, const char *productKey, const char *deviceName);//时钟同步请求
int FirmwareEvRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct firmwareEvt*data);//固件信息上报
int VerInfoEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct verInfoEvt*data);//事件：版本信息上报
int DevMdunInfoEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct devMduInfoEvt*data);//事件：充电桩组件信息
int AskConfigEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id);//事件：配置更新请求
int AskFeeModelEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct askFeeModelEvt*data);//事件：计费模型请求
int StartChargeAuthEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct startChargeAuthEvt*data);//事件：启动充电鉴权
int OrderUpdateEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct orderUpdateEvt*data);//事件：交易记录
int TotalFaultEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct totalFaultEvt*data);//事件 ：故障告警事件
int AcStChEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct acStChEvt*data);//事件：交流充电枪状态变更事件
int AcCarInfoEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct acCarInfoEvt*data);//事件：充电前车辆信息上报事件
int AcCarConChEvtRequest(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct acCarConChEvt*data);//事件：交流车辆连接状态变化事件
//属性上报
int AcDeRealItyProperty(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct acDeRealIty*data);//交流充电桩实时监测数据
int AcGunRunItyProperty(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct acGunRunIty*data);//交流充电枪充电中实时监测数据
int AcGunIdleItyProperty(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct acGunIdleIty*data);//交流充电枪非充电中实时监测数据
int AcOutMeterItyProperty(MQTTClient client, const char *productKey, const char *deviceName,const char *id,const struct acOutMeterIty*data);//交流输出电表底值监测数据


#ifdef __cplusplus
}
#endif

#endif