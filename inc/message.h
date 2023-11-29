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
#include "list.h"

#define MAX_TIMEOUT_SECONDS                                        4                              //判断回复信息超时的时间

int setSystemTime(const struct devTime *data);//对时
char *generateUniqueIdStr();//生成唯一id
//远程升级
int FirmwareVersionPush(const char *productKey, const char *deviceName, const char *id, const char *version, const char *module);//上报固件版本
int FirmwareRatePush(const char *productKey, const char *deviceName, const char *id, const char *step, const char *desc, const char *module);////上报升级进度
//回复服务
int confUpdateSrvReply(const char *productKey, const char *deviceName, const char *id, int code);//回复配置更新服务
int getConfSrvReply(const char *productKey, const char *deviceName, const char *id, const struct funConfUpdate_srv *data);//回复查询充电桩配置服务
int funConfUpdateSrvReply(const char *productKey, const char *deviceName, const char *id, int code);//回复功能配置更新
int getFunConfSrvReply(const char *productKey, const char *deviceName, const char *id, const struct funConfUpdate_srv *data);//回复查询充电桩功能配置服务
int queDataSrvReply(const char *productKey, const char *deviceName, const char *id, const struct queDataSrv *data);//回复充电桩日志查询
int devMaintainSrvReply(const char *productKey, const char *deviceName, const char *id, int ctrlType, int reason);//回复充电桩维护服务
int devMaintainQuerySrvReply(const char *productKey, const char *deviceName, const char *id, int ctrlType, int reason);//回复充电桩维护状态查询
int feeModelUpdateSrvReply(const char *productKey, const char *deviceName, const char *id, const struct feeModelUpdateSrvRep *data);//回复计量计费模型更新
int feeModelQuerySrvReply(const char *productKey, const char *deviceName, const char *id,const struct feeModelQuerySrv *data);//回复计量计费模查询服务
int startChargeSrvReply(const char *productKey, const char *deviceName, const char *id, const struct startChargeSrvRep *data);//回复启动充电服务
int authResultSrvReply(const char *productKey, const char *deviceName, const char *id, const struct startChargeAuthEvt *data);//回复启动充电鉴权事件
int stopChargeSrvReply(const char *productKey, const char *deviceName, const char *id, const struct startChargeSrvRep *data);//回复上级系统停止充电
int orderCheckSrvReply(const char *productKey, const char *deviceName, const char *id,const struct orderUpdateEvt *data);//回复有序充电策略下发
int acOrderlyChargeSrvReply(const char *productKey, const char *deviceName, const char *id, const struct acOrderlyChargeSrvRep *data);//回复：结果
//事件请求
int AskClockSynRequest(const char *productKey, const char *deviceName, const char *id);//时钟同步请求
int FirmwareEvRequest(const char *productKey, const char *deviceName,const char *id,const struct firmwareEvt*data);//固件信息上报
int VerInfoEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct verInfoEvt*data);//事件：版本信息上报
int DevMdunInfoEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct devMduInfoEvt*data);//事件：充电桩组件信息
int AskConfigEvtRequest(const char *productKey, const char *deviceName,const char *id);//事件：配置更新请求
int AskFeeModelEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct askFeeModelEvt*data);//事件：计费模型请求
int StartChargeAuthEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct startChargeAuthEvt*data);//事件：启动充电鉴权
int OrderUpdateEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct orderUpdateEvt*data);//事件：交易记录
int TotalFaultEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct totalFaultEvt*data);//事件 ：故障告警事件
int AcStChEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct acStChEvt*data);//事件：交流充电枪状态变更事件
int AcCarInfoEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct acCarInfoEvt*data);//事件：充电前车辆信息上报事件
int AcCarConChEvtRequest(const char *productKey, const char *deviceName,const char *id,const struct acCarConChEvt*data);//事件：交流车辆连接状态变化事件
//属性上报
int AcDeRealItyProperty(const char *productKey, const char *deviceName,const char *id,const struct acDeRealIty*data);//交流充电桩实时监测数据
int AcGunRunItyProperty(const char *productKey, const char *deviceName,const char *id,const struct acGunRunIty*data);//交流充电枪充电中实时监测数据
int AcGunIdleItyProperty(const char *productKey, const char *deviceName,const char *id,const struct acGunIdleIty*data);//交流充电枪非充电中实时监测数据
int AcOutMeterItyProperty(const char *productKey, const char *deviceName,const char *id,const struct acOutMeterIty*data);//交流输出电表底值监测数据


#ifdef __cplusplus
}
#endif

#endif