#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "ocpp_configurationKey.h"

static sqlite3 *ocpp_CK = NULL;


static int open_database(const char *db_name)
{
    if (ocpp_CK == NULL)
    {
        if (sqlite3_open(db_name, &ocpp_CK) != SQLITE_OK)
        {
            fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(ocpp_CK));
            return -1;
        }
    }
    return 0;
}

static void close_database()
{
    if (ocpp_CK != NULL)
    {
        sqlite3_close(ocpp_CK);
        ocpp_CK = NULL;
    }
}


static void ocpp_ConfiguartionKey_data_init(OCPP_ConfigurationKey_t *OCPP_ConfigurationKey)
{
	// 9.1.2. AuthorizationCacheEnable
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_AuthorizationCacheEnabled].key,
			ocpp_configurationKeyText[OCPP_CK_AuthorizationCacheEnabled], 64);

	// 9.1.3. AuthorizeRemoteTxRequests
	//
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_AuthorizeRemoteTxRequests].key,
			ocpp_configurationKeyText[OCPP_CK_AuthorizeRemoteTxRequests], 64);

	// 9.1.5. ClockAlignedDataInterval
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ClockAlignedDataInterval].key,
			ocpp_configurationKeyText[OCPP_CK_ClockAlignedDataInterval], 64);

	// 9.1.6. ConnectionTimeOut
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].type.intData = 0;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ConnectionTimeOut].key,
			ocpp_configurationKeyText[OCPP_CK_ConnectionTimeOut], 64);

	// 9.1.7. GetConfigurationMaxKeys
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_GetConfigurationMaxKeys].key,
			ocpp_configurationKeyText[OCPP_CK_GetConfigurationMaxKeys], 64);

	// 9.1.8. HeartbeatInterva
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].type.intData = 30;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_HeartbeatInterval].key,
			ocpp_configurationKeyText[OCPP_CK_HeartbeatInterval], 64);

	// 9.1.10. LocalAuthorizeOffline
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalAuthorizeOffline].key,
			ocpp_configurationKeyText[OCPP_CK_LocalAuthorizeOffline], 64);

	// 9.1.11. LocalPreAuthorize
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].type.boolData = false;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalPreAuthorize].key,
			ocpp_configurationKeyText[OCPP_CK_LocalPreAuthorize], 64);

	// 9.1.13. MeterValuesAlignedData
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].type.stringData,
			"Energy.Active.Import.Register,Current.Import,Power.Active.Import,Current.Offered,Power.Offered,Voltage,Soc", 512);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesAlignedData].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValuesAlignedData], 64);

	// 9.1.15. MeterValuesSampledData
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValuesSampledData], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValuesSampledData].type.stringData,
			"Energy.Active.Import.Register,Current.Import,Power.Active.Import,Current.Offered,Power.Offered,Voltage,Soc", 512);

	// 9.1.17. MeterValueSampleInterval
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].type.intData = 60;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MeterValueSampleInterval].key,
			ocpp_configurationKeyText[OCPP_CK_MeterValueSampleInterval], 64);

	// 9.1.19. NumberOfConnectors
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].type.intData = 2;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_NumberOfConnectors].key,
			ocpp_configurationKeyText[OCPP_CK_NumberOfConnectors], 64);
	// 9.1.20. ResetRetries
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ResetRetries].type.intData = 3;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ResetRetries].key,
			ocpp_configurationKeyText[OCPP_CK_ResetRetries], 64);

	// 9.1.23. StopTransactionOnEVSideDisconnect
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnEVSideDisconnect].key,
			ocpp_configurationKeyText[OCPP_CK_StopTransactionOnEVSideDisconnect], 64);

	// 9.1.24. StopTransactionOnInvalidId
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTransactionOnInvalidId].key,
			ocpp_configurationKeyText[OCPP_CK_StopTransactionOnInvalidId], 64);

	// 9.1.25. StopTxnAlignedData
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].key,
			ocpp_configurationKeyText[OCPP_CK_StopTxnAlignedData], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnAlignedData].type.stringData,
			"Energy.Active.Import.Register,Current.Import,Power.Active.Import,Current.Offered,Power.Offered,Voltage,Soc", 512);

	// 9.1.27. StopTxnSampledData
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].key,
			ocpp_configurationKeyText[OCPP_CK_StopTxnSampledData], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_StopTxnSampledData].type.stringData,
			"Energy.Active.Import.Register,Current.Import,Power.Active.Import,Current.Offered,Power.Offered,Voltage,Soc", 512);

	// 9.1.29. SupportedFeatureProfiles
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].key,
			ocpp_configurationKeyText[OCPP_CK_SupportedFeatureProfiles], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_SupportedFeatureProfiles].type.stringData,
			"Core,FirmwareManagement,LocalAuthListManagement,RemoteTrigger", 512);

	// 9.1.31. TransactionMessageAttempts
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempts].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempts].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempts].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempts].type.intData = 3;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_TransactionMessageAttempts].key,
			ocpp_configurationKeyText[OCPP_CK_TransactionMessageAttempts], 64);

	// 9.1.32. TransactionMessageRetryInterva
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].type.intData = 15;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_TransactionMessageRetryInterval].key,
			ocpp_configurationKeyText[OCPP_CK_TransactionMessageRetryInterval], 64);

	// 9.1.33. UnlockConnectorOnEVSideDisconnect
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_UnlockConnectorOnEVSideDisconnect].key,
			ocpp_configurationKeyText[OCPP_CK_UnlockConnectorOnEVSideDisconnect], 64);

	// 9.2.1. LocalAuthListEnabled
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].accessibility = OCPP_ACC_READWRITE;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].dataType = OCPP_CK_DT_BOOLEAN;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].type.boolData = true;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalAuthListEnabled].key,
			ocpp_configurationKeyText[OCPP_CK_LocalAuthListEnabled], 64);

	// 9.2.2. LocalAuthListMaxLength
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].type.intData = 50;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_LocalAuthListMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_LocalAuthListMaxLength], 64);

	// 9.2.3. SendLocalListMaxLength
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].type.intData = 20;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_SendLocalListMaxLength].key,
			ocpp_configurationKeyText[OCPP_CK_SendLocalListMaxLength], 64);

	// 9.4.1. ChargeProfileMaxStackLevel
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].type.intData = 3;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargeProfileMaxStackLevel].key,
			ocpp_configurationKeyText[OCPP_CK_ChargeProfileMaxStackLevel], 64);
	// 9.4.2. ChargingScheduleAllowedChargingRateUnit
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].dataType = OCPP_CK_DT_STRING;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].key,
			ocpp_configurationKeyText[OCPP_CK_ChargingScheduleAllowedChargingRateUnit], 64);
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleAllowedChargingRateUnit].type.stringData, "Current,Power", 512);

	// // 9.4.3. ChargingScheduleMaxPeriods
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].type.intData = 10;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_ChargingScheduleMaxPeriods].key,
			ocpp_configurationKeyText[OCPP_CK_ChargingScheduleMaxPeriods], 64);

	// // 9.4.5. MaxChargingProfilesInstalled
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].isUsed = true;
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].accessibility = OCPP_ACC_READONLY;
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].dataType = OCPP_CK_DT_INTEGER;
	OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].type.intData = 20;
	strncpy(OCPP_ConfigurationKey[OCPP_CK_MaxChargingProfilesInstalled].key, ocpp_configurationKeyText[OCPP_CK_MaxChargingProfilesInstalled], 64);
}

static int ocpp_ConfigurationKey_create_tables(OCPP_ConfigurationKey_t *OCPP_ConfigurationKey, int size)
{
	if (ocpp_CK == NULL || OCPP_ConfigurationKey == NULL || size <= 0)
	{
		return -1; // 输入参数错误
	}

	int rc;
	char *errMsg = NULL;
	char sql[512];

	// 创建表格
	snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS ConfigurationKeys("
							   "ID             INTEGER PRIMARY KEY AUTOINCREMENT,"
							   "Key            TEXT NOT NULL,"
							   "Value          TEXT DEFAULT NULL,"
							   "Type           INTEGER NOT NULL,"
							   "Accessibility  INTEGER NOT NULL,"
							   "IsUsed         INTEGER NOT NULL);");

	rc = sqlite3_exec(ocpp_CK, sql, NULL, NULL, &errMsg);
	if (rc != SQLITE_OK)
	{
		printf("创建 ConfigurationKeys 表失败: %s\n", errMsg);
		sqlite3_free(errMsg);
		return -1;
	}

	// 开始事务
	rc = sqlite3_exec(ocpp_CK, "BEGIN TRANSACTION;", 0, 0, 0);
	if (rc != SQLITE_OK)
	{
		printf("无法开始事务: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	sqlite3_stmt *stmt;
	const char *insert_sql = "INSERT INTO ConfigurationKeys (Key, Value, Type, Accessibility, IsUsed) VALUES (?, ?, ?, ?, ?)";
	char valueText[512];
	int i;
	for (i = 0; i < size; i++)
	{
		// 根据联合体类型将值转换为文本
		switch (OCPP_ConfigurationKey[i].dataType)
		{
		case OCPP_CK_DT_BOOLEAN:
			snprintf(valueText, sizeof(valueText), "%d", OCPP_ConfigurationKey[i].type.boolData);
			break;
		case OCPP_CK_DT_INTEGER:
			snprintf(valueText, sizeof(valueText), "%d", OCPP_ConfigurationKey[i].type.intData);
			break;
		case OCPP_CK_DT_STRING:
			snprintf(valueText, sizeof(valueText), "%s", OCPP_ConfigurationKey[i].type.stringData);
			break;
		default:
			snprintf(valueText, sizeof(valueText), "unknown");
			break;
		}

		// 准备 SQL 语句
		rc = sqlite3_prepare_v2(ocpp_CK, insert_sql, -1, &stmt, NULL);
		if (rc != SQLITE_OK)
		{
			// 回滚事务
			sqlite3_exec(ocpp_CK, "ROLLBACK;", 0, 0, 0);
			printf("无法准备 SQL 语句: %s\n", sqlite3_errmsg(ocpp_CK));
			return -1;
		}

		// 绑定参数
		sqlite3_bind_text(stmt, 1, OCPP_ConfigurationKey[i].key, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, valueText, -1, SQLITE_STATIC);
		sqlite3_bind_int(stmt, 3, OCPP_ConfigurationKey[i].dataType);
		sqlite3_bind_int(stmt, 4, OCPP_ConfigurationKey[i].accessibility);
		sqlite3_bind_int(stmt, 5, OCPP_ConfigurationKey[i].isUsed);

		// 执行 SQL 语句
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			// 回滚事务
			sqlite3_exec(ocpp_CK, "ROLLBACK;", 0, 0, 0);
			printf("插入数据失败: %s\n", sqlite3_errmsg(ocpp_CK));
			sqlite3_finalize(stmt);
			return -1; // 插入失败
		}

		// 释放资源
		sqlite3_finalize(stmt);
	}

	// 提交事务
	rc = sqlite3_exec(ocpp_CK, "COMMIT;", 0, 0, 0);
	if (rc != SQLITE_OK)
	{
		printf("提交事务失败: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	return 0; // 成功插入默认数据
}

/**
 * @description: 判断这个key值在数据库是否存在
 * @param {char *} key
 * @return {*} 1 = 存在,0 = 不存在
 */

bool ocpp_ConfigurationKey_isFound(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		// 处理错误情况，例如数据库未初始化或传入了无效的键
		return false;
	}

	// 使用参数化查询来避免 SQL 注入攻击
	sqlite3_stmt *stmt;
	const char *sql = "SELECT Accessibility FROM ConfigurationKeys WHERE Key=?";
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		// 处理数据库查询准备失败的情况
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		return false;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

	// 执行查询
	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		// 读取结果
		int count = sqlite3_column_int(stmt, 0);
		sqlite3_finalize(stmt);
		// 如果存在返回 true，否则返回 false
		return (count > 0) ? true : false;
	}
	else if (rc == SQLITE_ERROR)
	{
		// 处理 SQL 查询错误
		printf("Error executing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		sqlite3_finalize(stmt);
		return false;
	}
	else
	{
		// 处理其他情况，如未找到匹配的记录
		sqlite3_finalize(stmt);
		return false;
	}
}
/**
 * @description: 获取配置key的数据
 * @param {char *} key
 * @param {void *} value
 * @return {*} 返回value的数据类型,-1 = Error
 */

int ocpp_ConfigurationKey_getValue(const char *key, void *value)
{
	if (ocpp_CK == NULL || key == NULL || value == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化、传入了无效的键或无效的值指针
	}

	int type = -1;
	char dbValue[500];

	// 使用参数化查询来避免 SQL 注入攻击
	sqlite3_stmt *stmt;
	const char *sql = "SELECT Value, Type FROM ConfigurationKeys WHERE Key COLLATE NOCASE = ?";
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		// 处理数据库查询准备失败的情况
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

	// 执行查询
	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		// 读取结果
		snprintf(dbValue, sizeof(dbValue), "%s", (char *)sqlite3_column_text(stmt, 0));
		type = sqlite3_column_int(stmt, 1);
	}
	else if (rc == SQLITE_ERROR)
	{
		// 处理 SQL 查询错误
		printf("Error executing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		sqlite3_finalize(stmt);
		return -1;
	}

	// 释放资源
	sqlite3_finalize(stmt);

	if (type == -1 || dbValue[0] == '\0')
	{
		return -1; // 没有找到匹配的记录或值为空
	}

	// 根据Type将数据库中的值转换为对应的类型
	switch (type)
	{
	case 0: // 布尔类型
	case 1: // 整数类型
		*((int *)value) = atoi(dbValue);
		break;
	case 2: // 字符串类型
		strncpy((char *)value, dbValue, sizeof(dbValue));
		break;
	default:
		return -1; // 未知类型
	}

	return type; // 返回Type的值
}


int ocpp_ConfigurationKey_getIsUse(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化或传入了无效的键
	}

	int isUse = 0;

	// 使用参数化查询来避免 SQL 注入攻击
	sqlite3_stmt *stmt;
	const char *sql = "SELECT IsUse FROM ConfigurationKeys WHERE Key COLLATE NOCASE = ?";
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		// 处理数据库查询准备失败的情况
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

	// 执行查询
	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		isUse = sqlite3_column_int(stmt, 0);
	}
	else if (rc == SQLITE_ERROR)
	{
		// 处理 SQL 查询错误
		printf("Error executing SQL: %s\n", sqlite3_errmsg(ocpp_CK));
		sqlite3_finalize(stmt);
		return -1;
	}

	// 释放资源
	sqlite3_finalize(stmt);

	return isUse;
}

/**
 * @description: 获取配置key是否使用
 * @param {char *} key = "key名"
 * @return {*}0 = bool
 *            1 = int
 *            2  = string
 *            -1 = Error
 */

int ocpp_ConfigurationKey_getAcc(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化或传入了无效的键
	}

	int access = -1;

	// 使用参数化查询来避免 SQL 注入攻击
	sqlite3_stmt *stmt;
	const char *sql = "SELECT Accessibility FROM ConfigurationKeys WHERE Key COLLATE NOCASE = ?";
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		// 处理数据库查询准备失败的情况
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

	// 执行查询
	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		access = sqlite3_column_int(stmt, 0);
	}
	else if (rc == SQLITE_ERROR)
	{
		// 处理 SQL 查询错误
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		sqlite3_finalize(stmt);
		return -1;
	}

	// 释放资源
	sqlite3_finalize(stmt);

	return access;
}

/**
 * @description: 获取配置key的数据类型
 * @param {char *} key = "key名"
 * @return {*}0 = bool
 *            1 = int
 *            2  = string
 *            -1 = Error
 */
int ocpp_ConfigurationKey_getType(const char *key)
{
	if (ocpp_CK == NULL || key == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化或传入了无效的键
	}

	int dataType = -1;

	// 使用参数化查询来避免 SQL 注入攻击
	sqlite3_stmt *stmt;
	const char *sql = "SELECT Type FROM ConfigurationKeys WHERE Key COLLATE NOCASE = ?";
	int rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		// 处理数据库查询准备失败的情况
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

	// 执行查询
	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		dataType = sqlite3_column_int(stmt, 0);
	}
	else if (rc != SQLITE_DONE)
	{
		// 处理执行 SQL 失败的情况
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
	}

	// 释放资源
	sqlite3_finalize(stmt);

	return dataType;
}

/**
 * @description: 将Configurationkey插入数据库
 * @param:
 * @return: 0 = success, -1 = fail
 */
int ocpp_ConfigurationKey_InsertSQLite(const char *key, const char *value, char type, char accessibility, int isUse)
{
	if (ocpp_CK == NULL || key == NULL || value == NULL)
		return -1;

	int rc;
	sqlite3_stmt *stmt;
	const char *sql = "INSERT INTO ConfigurationKeys (Key, Value, Type, Accessibility, IsUse) VALUES (?, ?, ?, ?, ?)";
	rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, value, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, type);
	sqlite3_bind_int(stmt, 4, accessibility);
	sqlite3_bind_int(stmt, 5, isUse);

	// 执行 SQL 语句
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		sqlite3_finalize(stmt);
		return -1;
	}

	// 释放资源
	sqlite3_finalize(stmt);

	return 0;
}

int ocpp_ConfigurationKey_Modify(const char *key, const char *value, int isUse)
{
	if (ocpp_CK == NULL || key == NULL || value == NULL)
	{
		return -1; // 输入参数错误
	}

	int rc;
	sqlite3_stmt *stmt;
	const char *sql = "UPDATE ConfigurationKeys SET Value = ?, IsUse = ? WHERE Key = ?";
	rc = sqlite3_prepare_v2(ocpp_CK, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, value, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, isUse);
	sqlite3_bind_text(stmt, 3, key, -1, SQLITE_STATIC);

	// 执行 SQL 语句
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		printf("SQL error: %s\n", sqlite3_errmsg(ocpp_CK));
		sqlite3_finalize(stmt);
		return -1;
	}

	// 释放资源
	sqlite3_finalize(stmt);

	return 0; // 成功更新值和IsUse
}

int ocpp_ConfigurationKey_init()
{

	if (open_database(CHARGINGPROFILE_DB_PATH) != 0)
	{
		return -1;
	}

	OCPP_ConfigurationKey_t OCPP_ConfigurationKey[OCPP_CK_MAXLEN];
	ocpp_ConfiguartionKey_data_init(OCPP_ConfigurationKey);
	//数据库不存在或打开失败，尝试创建数据库和表
	if (ocpp_ConfigurationKey_create_tables(OCPP_ConfigurationKey,OCPP_CK_MAXLEN) != 0)
	{
		close_database();
		return -1;
	}

	return 0; // 初始化成功
}

// int main()
// {
// 	if (ocpp_ConfigurationKey_init() != 0)
// 	{
// 		return -1;
// 	}


// 	char MeterValuesSampledData[512];

// 	int numberOfConnector = 0;

// 	ocpp_ConfigurationKey_getValue(ocpp_configurationKeyText[OCPP_CK_NumberOfConnectors], (void *)&numberOfConnector);
// 	ocpp_ConfigurationKey_getValue("MeterValuesSampledData", (void *)MeterValuesSampledData);
// 	printf("MeterValuesSampledData = %s,numberOfConnector = %d\n",MeterValuesSampledData,numberOfConnector);

// 	return 0;
// }