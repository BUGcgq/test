
#ifndef __OCPP_CONFIGURATION_KEY__H__
#define __OCPP_CONFIGURATION_KEY__H__

#ifdef __cplusplus
extern "C" {
#endif

#include<stdbool.h>
#include <sqlite3.h>

#define CONFIGKEY_DB_PATH "/app/etc/configurationKey.db"

enum OCPP_ACCESSIBILITY_TYPE_E{
    OCPP_ACC_READONLY = 0,
    OCPP_ACC_WRITEONLY,
    OCPP_ACC_READWRITE,
    OCPP_ACC_NONE,
    OCPP_ACC_MAX
};

static const char * ocpp_AccessibilityText[OCPP_ACC_MAX] = {
    "READONLY",
    "WRITEONLY",
    "READWRITE",
    "NONE"
};

enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E{
    OCPP_CK_DT_BOOLEAN,
    OCPP_CK_DT_INTEGER,
    OCPP_CK_DT_STRING,
    OCPP_CK_DT_NONE
};

enum OCPP_CONFIGURATION_KEY_E {
    // Core Profile
    OCPP_CK_AuthorizationCacheEnabled,           // 是否启用授权缓存
    OCPP_CK_AuthorizeRemoteTxRequests,           // 是否需要事先授权远程交易请求
    OCPP_CK_ClockAlignedDataInterval,            // 时钟对齐数据间隔（秒）
    OCPP_CK_ConnectionTimeOut,                   // 超时取消交易时间（秒）
    OCPP_CK_GetConfigurationMaxKeys,             // 配置键中的最大项目数
    OCPP_CK_HeartbeatInterval,                   // 心跳间隔（秒）
    OCPP_CK_LocalAuthorizeOffline,               // 离线时是否启动本地授权交易
    OCPP_CK_LocalPreAuthorize,                   // 联机时是否启动本地授权交易

    OCPP_CK_MeterValuesAlignedData,              // 对齐数据的电表值
    OCPP_CK_MeterValuesSampledData,              // 采样数据的电表值
    OCPP_CK_MeterValueSampleInterval,            // 电表值采样间隔（秒）
    OCPP_CK_NumberOfConnectors,                  // 连接器数量
    OCPP_CK_ResetRetries,                        // 重置重试次数

    OCPP_CK_StopTransactionOnEVSideDisconnect,   // EV断开时是否停止交易
    OCPP_CK_StopTransactionOnInvalidId,          // ID无效时是否停止交易
    OCPP_CK_StopTxnAlignedData,                  // 停止交易时的对齐数据
    OCPP_CK_StopTxnSampledData,                  // 停止交易时的采样数据
    OCPP_CK_SupportedFeatureProfiles,            // 支持的功能配置文件
    OCPP_CK_TransactionMessageAttempts,          // 交易消息尝试次数
    OCPP_CK_TransactionMessageRetryInterval,     // 交易消息重试间隔（秒）
    OCPP_CK_UnlockConnectorOnEVSideDisconnect,   // EV断开时是否解锁连接器

    // Local Auth List Management Profile
    OCPP_CK_LocalAuthListEnabled,                // 是否启用本地授权列表
    OCPP_CK_LocalAuthListMaxLength,              // 本地授权列表的最大长度
    OCPP_CK_SendLocalListMaxLength,              // 发送本地列表的最大长度

    // Smart Charging Profile
    OCPP_CK_ChargeProfileMaxStackLevel,          // 最大充电配置文件堆栈级别
    OCPP_CK_ChargingScheduleAllowedChargingRateUnit,  // 允许的充电速率单位
    OCPP_CK_ChargingScheduleMaxPeriods,          // 最大充电时间段
    OCPP_CK_MaxChargingProfilesInstalled,        // 已安装的最大充电配置文件
    OCPP_CK_MAXLEN
};




static const char *ocpp_configurationKeyText[OCPP_CK_MAXLEN] = {
    // 9.1 Core Profile
    "AuthorizationCacheEnabled",
    "AuthorizeRemoteTxRequests",
    "ClockAlignedDataInterval",
    "ConnectionTimeOut",
    "GetConfigurationMaxKeys",
    "HeartbeatInterval",
    "LocalAuthorizeOffline",
    "LocalPreAuthorize",
    // 电表值相关
    "MeterValuesAlignedData",
    "MeterValuesSampledData",
    "MeterValueSampleInterval",
    "NumberOfConnectors",
    "ResetRetries",

    "StopTransactionOnEVSideDisconnect",
    "StopTransactionOnInvalidId",
    "StopTxnAlignedData",
    "StopTxnSampledData",
    "SupportedFeatureProfiles",
    "TransactionMessageAttempts",
    "TransactionMessageRetryInterval",
    "UnlockConnectorOnEVSideDisconnect",
    // 9.2 Local Auth List Management Profile
    "LocalAuthListEnabled",
    "LocalAuthListMaxLength",
    "SendLocalListMaxLength",
    // 9.4 Smart Charging Profile
    "ChargeProfileMaxStackLevel",
    "ChargingScheduleAllowedChargingRateUnit",
    "ChargingScheduleMaxPeriods",
    "MaxChargingProfilesInstalled"
};

typedef struct{
    char key[64];
    bool isUsed;                                                
    enum OCPP_ACCESSIBILITY_TYPE_E accessibility; 
    enum OCPP_CONFIGURATION_KEY_DATA_TYPE_E dataType;
    union{
        bool boolData;
        int  intData;
        char stringData[512];;
    }type;
    
}OCPP_ConfigurationKey_t;

bool ocpp_ConfigurationKey_isFound(const char *key);
int ocpp_ConfigurationKey_getValue(const char *key, void *value);
int ocpp_ConfigurationKey_getIsUse(const char *key);
int ocpp_ConfigurationKey_getAcc(const char *key);
int ocpp_ConfigurationKey_getType(const char *key);
int ocpp_ConfigurationKey_Modify(const char *key, const char *value, int isUse);
int ocpp_ConfigurationKey_init();


#ifdef __cplusplus
}
#endif

#endif