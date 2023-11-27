#ifndef __PACKAGE__H__
#define __PACKAGE__H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <json-c/json.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>

#define FEE_MODE_TIME_NUM              14 //最大时段数
#define FAULT_WARN_VALUE_NUM           45 //最大故障点数据,告警点数据数目
#define CHARGE_PLOY_MAX_NUM           10 //最大策略数
#define MAX_DATA_SIZE 512
struct RecvData
{
    char id[40];
    char Version[40];
    char method[40];
    char params[512];
};
struct firmwareInfo_rec 
{
    char id[24];//消息 ID 号，String 类型的数字，取值范围 0~4294967295，且每个消息 ID在当前充电桩中具有唯一性。
    char message[256];//结果信息。
    char code[16];//状态码。
    char version[24];//充电桩固件的版本信息。
    int  size;//固件大小，单位：字节。
    char url[64];//固件下载地址。
    char sign[64];//固件签名。
    char signMethod[16];//签名方法， 目前支持 Md5，SHA256 两种签名方法。
    char md5[64];//当签名方法为 Md5 时，除了会给 sign 赋值外还会给 md5 赋值。
    char module[24];//固件所属的模块名。模块名为 default 时，云端不下发 module 参数。

};
struct devTime_rec 
{
    int deviceSendTime;
    int serverRecvTime;
    int serverSendTime;
};
struct firmwareEvt_req 
{
    char simNo[24];//SIM卡号
    char eleModelId[16];//充电桩当前使用的电费模型编号
    char serModelId[20];//充电桩当前使用的服务费模型编号
    char stakeModel[20];//充电桩型号，由厂商提供
    int vendorCode;//充电桩厂商编码
    char devSn[32];//即出厂编码，充电桩注册前作为充电桩唯一识别码使用。
    int devType;//终端类型10:单相交流 11:三相交流
    int portNum;//充电接口数
    char simMac[24];//充电桩的 MAC 地址
    int longitude;//经度,精度：0.000001，实际数据乘 1000000 再上传，单位：度
    int latitude;//纬度精度：0.01，实际数据乘 100 再上传，单位:m
    int height;//高度
    int gridType;//坐标类型,10：无经纬度坐标数值11：GPS 坐标12：北斗坐标13：GLONASS 坐标14：伽利略坐标
    char btMac[24];//充电桩蓝牙 MAC，
    int meaType;//用于计算充电费用的电表类型：11：交流电能表13：交流计量模块14：交流计量芯片
    int otRate;//额定功率,单位：kW，精确到小数点后一位，实际数据乘 10 再上传
    int otMinVol;//单位：V，精确到小数点后一位，实际数据乘 10 再上传
    int otMaxVol;//单位：V，精确到小数点后一位，实际数据乘 10 再上传
    int otCur;//单位：A，精确到小数点后一位，实际数据乘 10 再上传
    char inMeter[64];//交流 输入 电能 表地址
    char outMeter[64];//计量 用电 能表 地址
    int CT;//电流互感器变比
    int isGateLock;//10:无智能门锁11:有智能门锁
    int isGroundLock;//10:无地锁11:有地锁
};

struct verInfoEvt_req 
{
    int devRegMethod;// 充电桩注册方式,10:屏幕11:蓝牙12:串口13:NFC14:USB 外设15:固件16:其他
    char pileSoftwareVer[256];//充电桩软件版本号，最长 256 个字符。
    char pileHardwareVer[256];//充电桩硬件版本号
    char sdkVer[256];//使用的 SDK 版本号，最长 256 个字符。
};

struct devMduInfoEvt_req 
{
    char netMduInfo[256];//网络模块型号信息，
    char netMduSoftVer[256];//网络模块软件版本号信息
    char netMduImei[256];//网络模块 IMEI 信息，
    int  smartGun;//10：支持11：不支持
    int  mduInfoInt[2];
    char mduInfoString[2][256];
};

struct confUpdateSrv_rep
{
    int  equipParamFreq;//充电桩实时监测数据上报频率
    int  gunElecFreq;//充电枪充电中实时监测属性上报频率
    int  nonElecFreq;//充电枪非充电中实时监测数据上报频率
    int  faultWarnings;//故障告警全信息上传频率
    int  acMeterFreq;//充电桩交流电表底值监测数据上报频率
    int  offlinChaLen;//离线后可充电时长
    int  grndLock;//地锁监测上送频率
    int  doorLock;//门锁监测上送频率
    char qrCode[256];//二维码数据
};
struct funConfUpdate_srv
{
    int funCode;
    int confInt;
    char confString[32];
};
struct queDataSrv_rep
{
    int gunNo;//枪号
    char startDate[32];// 查询起始时间
    char stopDate[32];// 查询终止时间
    int askType;//查询类型
    char logQueryNo[38];//查询流水号
};

struct logQueryEvt_rep
{
    int gunNo;//枪号
    char startDate[32];// 查询起始时间
    char stopDate[32];// 查询终止时间
    int askType;//查询类型
    int result;//响应结果
    char logQueryNo[38];//查询流水号
    int retType;//响应类型
    int logQueryEvtSum;//日志结果上报事件总帧数
    int logQueryEvtNo;//日志结果上报帧序号
    char dataArea[256];//响应数据区
};

struct askFeeModelEvt_req
{
    int gunNo;//充电枪编号
    char eleModelId[16];//当前电费模型编号
    char serModelId[16];//当前服务费模型编号
};


struct feeModelUpdateSrv_rec
{
    char eleModelId[16];//当前电费模型编号
    char serModelId[16];//当前服务费模型编号
    int  timeNum;//时段数 N
    char timeSeg[FEE_MODE_TIME_NUM][24];//时段开始时间点
    int segFlag[FEE_MODE_TIME_NUM];//时段标志
    int chargeFee[FEE_MODE_TIME_NUM];//电费
    int serviceFee[FEE_MODE_TIME_NUM];//电费
};

struct feeModelUpdateSrv_rep
{
    char eleModelId[16];//当前电费模型编号
    char serModelId[16];//当前服务费模型编号
    int  result;//响应结果

};
struct feeModelQuerySrv_rep
{
    int gunNo;//枪号
    char eleModelId[16];//当前电费模型编号
    char serModelId[16];//当前服务费模型编号
    int  timeNum;//时段数 N
    char timeSeg[FEE_MODE_TIME_NUM][24];//时段开始时间点
    int segFlag[FEE_MODE_TIME_NUM];//时段标志
    int chargeFee[FEE_MODE_TIME_NUM];//电费
    int serviceFee[FEE_MODE_TIME_NUM];//电费
};

struct startChargeSrv_rec
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    int  startType;//启动方式
    int  chargeMode;//充电模式
    int  limitData;//限制值
    int  stopCode;//停机码
    int  startMode;//启动模式
    int  insertGunTime;//insertGunTime
};
struct startChargeSrv_rep
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
};

struct startChaResEvt_rep
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    int startResult;//启动结果
    int faultCode;//故障代码
    char vinCode[40];//vin 码

};

struct startChargeAuthEvt_req
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    int startType;//启动方式
    char authCode[40];//鉴权码
    int batterySoc;//电池 SOC
    int batteryCap;//电池容量
    int chargeTimes;//已充电次数
    int batteryVol;//当前电池电压
};

struct authResultSrv_rec
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    int  startType;//启动方式
    char authCode[40];//鉴权码
    int result;//鉴权结果
    int chargeMode;//充电模式
    int limitData;//限制值
    int stopCode;//停机码
    int startMode;//启动模式
    int insertGunTime;//插枪事件时间戳

};
struct stopChargeSrv_rec
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    int  stopReason;//停止原因
};

struct stopChargeSrv_rep
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
};

struct stopChaResEvt_rep
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    int stopResult;//停止结果
    int resultCode;//停止原因
    int stopFailReson;//停止失败原因
};

struct orderUpdateEvt_req
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    char vinCode[40];// 车辆唯一识别码
    int timeDivType;//计量计费类型
    char chargeStartTime[40];//开始充电时间
    char chargeEndTime[40];//结束充电时间
    int startSoc;//启动时 SOC
    int endSoc;//停止时 SOC
    int reason;//停止充电原因
    char eleModelId[40];//电费模型编号
    char serModelId[40];// 服务费模型编号
    char sumStart[40];//电表总起始值
    char sumEnd[40];//电表总止示值
    int totalElect;//总电量
    int sharpElect;//尖电量
    int peakElect;//峰电量
    int flatElect;//平电量
    int valleyElect;//谷电量
    int totalPowerCost;//总电费
    int totalServCost;//总服务费
    int sharpPowerCost;//尖电费
    int peakPowerCost;//峰电费
    int flatPowerCost;//平电费
    int valleyPowerCost;//谷点费
    int sharpServCost;//尖服务费
    int peakServCost;//峰服务费
    int flatServCost;//平电服务费
    int valleyServCost;//谷服务费
    
};

struct orderCheckSrv_rec
{
    int gunNo;//枪号
    char preTradeNo[40];//上级系统交易流水号
    char tradeNo[38];//充电桩交易流水号
    int errCode;//交易记录上传结果

};

struct totalFaultEvt_req
{
    int gunNo;//枪号
    int faultSum;//故障总数
    int warnSum;//告警总数
    int faultValue[FAULT_WARN_VALUE_NUM];//故障点数据
    int warnValue[FAULT_WARN_VALUE_NUM];//告警点数据

};

struct acDeRealIty_ity
{
    int netType;//网络类型
    int sigVal;//网络信号等级
    int netId;//网络运营商
    int acVolA;//A 相采集电压
    int acCurA;//A 相采集电流
    int acVolB;//B 相采集电压
    int acCurB;//B 相采集电流
    int acVolC;//C 相采集电压
    int acCurC;//C 相采集电流
    int caseTemp;//桩内温度
    char eleModelId[40];//电费模型编号
    char serModelId[40];// 服务费模型编号
};

struct acGunRunIty_ity
{
    int gunNo;//充电枪编号
    int workStatus;//工作状态
    int conStatus;//连接确认开关状态
    int outRelayStatus;//输出继电器状态
    int eLockStatus;//充电接口电子锁状态
    int gunTemp;// 充电枪头温度
    int acVolA;//A 相采集电压
    int acCurA;//A 相采集电流
    int acVolB;//B 相采集电压
    int acCurB;//B 相采集电流
    int acVolC;//C 相采集电压
    int acCurC;//C 相采集电流
    char preTradeNo[40];//上级系统交易流水号 
    char tradeNo[40];// 充电桩交易流水号
    int realPower;//充电实际功率
    int chgTime;//累计充电时间
    int totalElect;//总电量
    int sharpElect;//尖电量
    int peakElect;//峰电量
    int flatElect;//平电量
    int valleyElect;//谷电量
    int totalCost;//总金额
    int totalPowerCost;//总电费
    int totalServCost;//总服务费
    int PwmDutyRadio;//PWM 占空比
};

struct acGunIdleIty_ity
{
    int gunNo;//充电枪编号
    int workStatus;//工作状态
    int conStatus;//连接确认开关状态
    int outRelayStatus;//输出继电器状态
    int eLockStatus;//充电接口电子锁状态
    int gunTemp;// 充电枪头温度
    int acVolA;//A 相采集电压
    int acCurA;//A 相采集电流
    int acVolB;//B 相采集电压
    int acCurB;//B 相采集电流
    int acVolC;//C 相采集电压
    int acCurC;//C 相采集电流
};

struct acOutMeterIty_ity
{
    int gunNo;//充电枪编号
    char acqTime[40];//采集时间
    char mailAddr[40];//通信地址
    char meterNo[40];//表号
    char assetId[40];//电表资产编码
    char sumMeter[40];//电表底值
    char lastTrade[40];//最后交易流水
    int power;//充电中订单的已充电量
};

struct acOrderlyChargeSrv_rec
{
    char preTradeNo[40];//订单流水号
    char validTime[CHARGE_PLOY_MAX_NUM][40];//策略生效时间
    int  kw[CHARGE_PLOY_MAX_NUM];//策略配置功率
    int num;//策略段数
};

struct acOrderlyChargeSrv_rep
{
    char preTradeNo[40];//订单流水号
    int  result;//返回结果
    int reason;//失败原因
};

struct acStChEvt_req
{
    int  gunNo;//充电枪编号
    char yxOccurTime[40];//发生时刻
    int connCheckStatus;//充电枪连接状态
};

struct acCarInfoEvt_req
{
    int  gunNo;//返回结果
    int  batterySOC;//电池 SOC
    int  batteryCap;//电池容量
    char vinCode[40];//VIM
    int state;//获取车辆信息状态
};

struct acCarConChEvt_req
{
    int  gunNo;//返回结果
    int  cpStatus;//CP 状态
    int  cpVolt;//CP 电压采集值
    int s3Status;//S3 开关状态
    char yxOccurTime[40];//发生时刻
};
//上报固件版本Topic：/ota/device/inform/{YourProductKey}/{YourDeviceName}
//推送固件信息Topic：/ota/device/upgrade/{YourProductKey}/{YourDeviceName},
//上报升级进度Topic：/ota/device/progress/{YourProductKey}/{YourDeviceName}
//请求Topic：/sys/{productKey}/{deviceName}/thing/ota/firmware/get
//响应Topic：/sys/{productKey}/{deviceName}/thing/ota/firmware/get_reply
//时钟同步请求Topic： /ext/ntp/${YourProductKey}/${YourDeviceName}/request
//时钟同步响应Topic： /ext/ntp/${YourProductKey}/${YourDeviceName}/response
//属性上报请求Topic： /sys/{productKey}/{deviceName}/thing/event/property/post
//属性上报响应Topic： /sys/{productKey}/{deviceName}/thing/event/property/post_reply
//属性设置请求Topic：/sys/{productKey}/{deviceName}/thing/service/property/set
//属性设置响应Topic：/sys/{productKey}/{deviceName}/thing/service/property/set_reply
//服务调用请求Topic： /sys/{productKey}/{deviceName}/thing/service/{identifier}
//服务调用响应Topic： /sys/{productKey}/{deviceName}/thing/service/{identifier}_reply
//事件上送请求Topic： /sys/{productKey}/{deviceName}/thing/model/up_raw
//事件上送响应Topic： /sys/{productKey}/{deviceName}/thing/model/up_raw_reply
//远程升级
//组包函数返回的是字符串内存，需要调用free释放，不然会内存泄露
int parseRecvData(const char *jsondata, struct RecvData *recvData);//解析服务器下放的事件，服务，属性数据的包
const char *createFirmwareVersionPush(const char *id, const char *version, const char *module);//上报固件版本
int parseFirmwareInfo(const char *jsondata, struct firmwareInfo_rec *data);//推送固件信息
const char *createFirmwareRatePush(const char *id, const char *step, const char *desc,const char *module);//上报升级进度
const char *createAskFirmwareInfoRequest(const char *id, const char *module);//设备请求固件信息
//时钟同步
const char *createAskClockSynRequest(long long milliseconds);//时钟同步请求
int  parseDevTimeInfo(const char *jsondata, struct devTime_rec *data);//时钟同步响应
//固件信息上报
const char *createFirmwareEvtRequest(const char *id,const struct firmwareEvt_req *data);//事件：固件信息上报
//版本信息上报
const char *createVerInfoEvtRequest(const char *id,const struct verInfoEvt_req *data);//事件：版本信息上报
//充电桩组件信息上报
const char *createDevMdunInfoEvtRequest(const char *id,const struct devMduInfoEvt_req *data);//事件：充电桩组件信息
//充电桩配置信息管理
const char *createAskConfigEvtRequest(const char *id);//事件：配置更新请求
int  parseConfUpdateSrv(const char *jsondata, struct confUpdateSrv_rep *data);//服务：配置更新
const char *createConfUpdateSrvReply(const char *id,int code);//回复：更新结果
int  parseGetConfSrv(const char *jsondata);//服务：配置更新
const char *createGetConfigSrvReply(const char *id,const struct confUpdateSrv_rep *data);//回复：充电桩配置信息
//充电桩功能配置管理
int  parseFunConfUpdateSrv(const char *jsondata, struct funConfUpdate_srv *data);//服务：功能配置
const char *createFunConfUpdateSrvReply(const char *id,int code);//回复：配置结果
int  parseGetFunConfSrv(const char *jsondata);//服务：功能配置查询
const char *createGetFunConfSrvSrvReply(const char *id, const struct funConfUpdate_srv *data);//回复：查询结果
//充电桩日志查询
int  parseQueDataSrv(const char *jsondata, struct queDataSrv_rep * data);//服务：查询设备日志信息
const char *createQueDataSrvReply(const char *id, const struct queDataSrv_rep *data);//回复：日志信息
//充电桩日志查询结果上报事件
const char *createLogQueryEvtReply(const char *id,const struct logQueryEvt_rep *data);//事件：日志查询结果
//充电桩维护管理
int  parseDevMaintainSrv(const char *jsondata);//服务：充电桩维护服务
const char *createDevMaintainSrvReply(const char *id,int ctrlType,int reason);//回复：设置结果
int  parseDevMaintainQuerySrv(const char *jsondata);//服务：维护状态查询
const char *createDevMaintainQuerySrvReply(const char *id,int ctrlType,int reason);//回复：当前状态
//计量计费模型管理
const char *createAskFeeModelEvtRequest(const char *id,const struct askFeeModelEvt_req *data);//事件：计费模型请求
int  parseFeeModelUpdateSrv(const char *jsondata, struct feeModelUpdateSrv_rec *data);//服务：计费模型更新
const char *createFeeModelUpdateSrvReply(const char *id,const struct feeModelUpdateSrv_rep *data);//回复：更新结果
//计量计费模查询服务
int  parseFeeModelQuerySrv(const char *jsondata);//服务：计费模型查询
const char *createFeeModelQuerySrvReply(const char *id,const struct feeModelQuerySrv_rep *data);//回复：查询结果
//上级系统远程发起充电
int  parseStartChargeSrv(const char *jsondata, struct startChargeSrv_rec *data);//服务：启动充电
const char *createStartChargeSrvReply(const char *id,const struct startChargeSrv_rep *data);//回复：交易信息
const char *createStartChaResEvtReply(const char *id, const struct startChaResEvt_rep *data);//事件：充电启动结果
//充电桩发起充电
const char *createStartChargeAuthEvtRequest(const char *id,const struct startChargeAuthEvt_req *data);//事件：启动充电鉴权
int  parseAuthResultSrv(const char *jsondata, struct authResultSrv_rec *data);//服务：鉴权结果
//上级系统停止充电
int  parseStopChargeSrv(const char *jsondata, struct stopChargeSrv_rec *data);//服务：停止充电
const char *createStopChargeSrvReply(const char *id,const struct stopChargeSrv_rep *data);//回复：交易信息
//停止结果事件
const char *createStopChaResEvtReply(const char *id, const struct stopChaResEvt_rep *data);//事件：停止充电结果
//交易记录上传事件
const char *createOrderUpdateEvtRequest(const char *id,const struct orderUpdateEvt_req *data);//事件：交易记录
//交易记录确认服务
int  parseOrderCheckSrv(const char *jsondata, struct orderCheckSrv_rec *data);//服务：交易记录结果
//充电桩故障告警
const char *createTotalFaultEvtRequest(const char *id,const struct totalFaultEvt_req *data);//事件 ：故障告警事件
//数据上报
const char *createAcDeRealItyProperty(const char *id,const struct acDeRealIty_ity *data);//交流充电桩实时监测数据
const char *createAcGunRunItyProperty(const char *id,const struct acGunRunIty_ity *data);//交流充电枪充电中实时监测数据
const char *createAcGunIdleItyProperty(const char *id,const struct acGunIdleIty_ity *data);//交流充电枪非充电中实时监测数据
const char *createAcOutMeterItyProperty(const char *id,const struct acOutMeterIty_ity *data);//交流输出电表底值监测数据
//有序充电策略
int  parseAcOrderlyChargeSrv(const char *jsondata, struct acOrderlyChargeSrv_rec *data);//服务：有序充电
const char *createAcOrderlyChargeSrvReply(const char *id,const struct acOrderlyChargeSrv_rep *data);//回复：结果
//交流充电枪状态变更事件
const char *createAcStChEvtRequest(const char *id,const struct acStChEvt_req *data);//事件：交流充电枪状态变更事件
//充电前车辆信息上报事件
const char *createAcCarInfoEvtRequest(const char *id,const struct acCarInfoEvt_req *data);//事件：充电前车辆信息上报事件
//交流车辆连接状态变化事件
const char *createAcCarConChEvtRequest(const char *id,const struct acCarConChEvt_req *data);//事件：交流车辆连接状态变化事件
#ifdef __cplusplus
}
#endif

#endif