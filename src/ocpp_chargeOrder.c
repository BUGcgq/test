#include "ocpp_chargeOrder.h"
// #include "../drive/Fram.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

static sqlite3 *orderDB;

static int open_database(const char *db_name)
{
	if (orderDB == NULL)
	{
		if (sqlite3_open(db_name, &orderDB) != SQLITE_OK)
		{
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(orderDB));
			return -1;
		}
	}
	return 0;
}

static void close_database()
{
	if (orderDB != NULL)
	{
		sqlite3_close(orderDB);
		orderDB = NULL;
	}
}

static int ocpp_ChargingOrders_create_tables()
{
	if (orderDB == NULL)
	{
		return -1; // 处理数据库连接获取失败的情况
	}

	char *zErrMsg = 0;
	sqlite3_stmt *stmt = NULL;
	const char *sql = "CREATE TABLE IF NOT EXISTS OCPP_Orders  ("
					  "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
					  "Connector INTEGER,"
					  "TransactionId INTEGER UNIQUE,"
					  "IdTag TEXT,"
					  "StartMeterValue REAL,"
					  "StartTimestamp TEXT,"
					  "StartResponse INTEGER,"
					  "StopMeterValue REAL,"
					  "StopTimestamp TEXT,"
					  "StopResponse INTEGER,"
					  "Reason INTEGER,"
					  "Type INTEGER"
					  ");";

	int rc = sqlite3_exec(orderDB, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("创建 chargingOrders 表失败: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

}
/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-03-05
 * 描述 : 获取订单数量
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
static int ocpp_ChargingOrders_getRecordCount()
{
	if (orderDB == NULL)
	{
		return -1; // 数据库连接无效
	}

	const char *countSQL = "SELECT COUNT(*) FROM OCPP_Orders;";
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, countSQL, -1, &stmt, NULL);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Query preparation error: %s\n", sqlite3_errmsg(orderDB));
		return -1; // 查询准备失败
	}

	int recordCount = 0;

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		recordCount = sqlite3_column_int(stmt, 0);
	}
	else
	{
		fprintf(stderr, "Query execution error: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt); // 关闭语句对象
		return -1;				// 查询执行失败
	}

	sqlite3_finalize(stmt); // 关闭语句对象
	return recordCount;
}

/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-03-05
 * 描述 : 删除最旧订单个数
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
static int ocpp_ChargingOrders_delOldestRecords(int n)
{
	if (orderDB == NULL)
	{
		return -1; // 数据库连接无效
	}

	// 删除最旧的 n 条记录
	const char *deleteSQL = "DELETE FROM OCPP_Orders WHERE ID IN (SELECT ID FROM OCPP_Orders ORDER BY StartTimestamp ASC LIMIT ?);";
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, deleteSQL, -1, &stmt, NULL);

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Delete statement preparation error: %s\n", sqlite3_errmsg(orderDB));
		return -1; // 删除准备失败
	}

	sqlite3_bind_int(stmt, 1, n);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "Delete error: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt);
		return -1; // 删除失败
	}

	sqlite3_finalize(stmt);
	return 0; // 删除成功
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：插入一条订单到数据库
 * 描    述:
 *
 * 参    数: order - [订单结构体]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int ocpp_chargeOrder_insert(const CHARGE_ORDER_S *order)
{
	if (orderDB == NULL || order == NULL || order->transactionId <= 0)
	{
		return -1; // 无效的参数或数据库连接
	}

	if (ocpp_ChargingOrders_getRecordCount() > ORDER_MAX_CNT)
	{
		ocpp_ChargingOrders_delOldestRecords(ORDER_MAX_OVERFLOW_CNT);
	}

	const char *insertSQL = "INSERT INTO OCPP_Orders (Connector, TransactionId, IdTag, StartMeterValue, "
							"StartTimestamp, StartResponse, StopMeterValue, StopTimestamp, StopResponse, "
							"Reason, Type) "
							"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, insertSQL, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "准备插入语句时出错: %s\n", sqlite3_errmsg(orderDB));
		return -1; // 准备语句失败
	}

	// 注意绑定参数的顺序和数据类型
	sqlite3_bind_int(stmt, 1, order->connector);
	sqlite3_bind_int(stmt, 2, order->transactionId);
	sqlite3_bind_text(stmt, 3, order->idTag, -1, SQLITE_STATIC);
	sqlite3_bind_double(stmt, 4, order->startMeterValue);
	sqlite3_bind_text(stmt, 5, order->startTimestamp, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, order->startResponse);
	sqlite3_bind_double(stmt, 7, order->stopMeterValue);
	sqlite3_bind_text(stmt, 8, order->stopTimestamp, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 9, order->stopResponse);
	sqlite3_bind_int(stmt, 10, order->reason);
	sqlite3_bind_int(stmt, 11, order->type);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "插入数据时出错: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt);
		return -1; // 插入数据失败
	}

	sqlite3_finalize(stmt);

	return 0; // 插入成功
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：更新数据库订单数据
 * 描    述: 根据事物ID更新订单
 *
 * 参    数: transactionId - [事物ID]
 * 参    数: order - [订单结构体]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int ocpp_chargeOrder_update(int transactionId, const CHARGE_ORDER_S *order)
{
	if (orderDB == NULL || order == NULL)
	{
		return -1; // 无效的参数或数据库连接
	}
	const char *updateSQL = "UPDATE OCPP_Orders SET Connector=?, TransactionId=?, IdTag=?, StartMeterValue=?, "
							"StartTimestamp=?, StartResponse=?, StopMeterValue=?, StopTimestamp=?, "
							"StopResponse=?, Reason=?, Type=? "
							"WHERE TransactionId=?";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, updateSQL, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "准备更新语句时出错: %s\n", sqlite3_errmsg(orderDB));
		return -1; // 准备语句失败
	}

	// 注意绑定参数的顺序和数据类型
	sqlite3_bind_int(stmt, 1, order->connector);
	sqlite3_bind_int(stmt, 2, order->transactionId);
	sqlite3_bind_text(stmt, 3, order->idTag, -1, SQLITE_STATIC);
	sqlite3_bind_double(stmt, 4, order->startMeterValue);
	sqlite3_bind_text(stmt, 5, order->startTimestamp, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 6, order->startResponse);
	sqlite3_bind_double(stmt, 7, order->stopMeterValue);
	sqlite3_bind_text(stmt, 8, order->stopTimestamp, -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 9, order->stopResponse);
	sqlite3_bind_int(stmt, 10, order->reason);
	sqlite3_bind_int(stmt, 11, order->type);
	sqlite3_bind_int(stmt, 12, transactionId); // Bind TransactionId for WHERE clause

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "更新数据时出错: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt);
		return -1; // 更新数据失败
	}

	sqlite3_finalize(stmt);

	return 0; // 更新成功
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：更新服务器回复
 * 描    述: 根据事物ID更新服务器回复停止事物
 *
 * 参    数: transactionId - [事物ID]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int ocpp_chargeOrder_StopResponse_update(int transactionId)
{
	if (orderDB == NULL)
	{
		return -1;
	}

	const char *updateSQL = "UPDATE OCPP_Orders SET StopResponse=1 WHERE TransactionId=?";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, updateSQL, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "准备更新语句时出错: %s\n", sqlite3_errmsg(orderDB));
		return -1; // 准备语句失败
	}

	sqlite3_bind_int(stmt, 1, transactionId);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "更新数据时出错: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt);
		return -1; // 更新数据失败
	}

	sqlite3_finalize(stmt);

	return 0; // 更新成功
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：更新事物ID
 * 描    述:
 *
 * 参    数: oldTransactionId - [旧事物ID]
 * 参    数: newTransactionId - [新事物ID]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int ocpp_chargeOrder_transactionId_update(int oldTransactionId, int newTransactionId)
{
	if (orderDB == NULL)
	{
		return -1;
	}
	const char *updateSQL = "UPDATE OCPP_Orders SET TransactionId=? WHERE TransactionId=?";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, updateSQL, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "准备更新语句时出错: %s\n", sqlite3_errmsg(orderDB));
		return -1; // 准备语句失败
	}

	sqlite3_bind_int(stmt, 1, newTransactionId);
	sqlite3_bind_int(stmt, 2, oldTransactionId);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "更新数据时出错: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt);
		return -1; // 更新数据失败
	}

	sqlite3_finalize(stmt);

	return 0; // 更新成功
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：更新服务器回复
 * 描    述: 根据事物ID更新服务器回复开始事物
 *
 * 参    数: transactionId - [事物ID]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int ocpp_chargeOrder_startResponse_update(int transactionId)
{
	if (orderDB == NULL)
	{
		return -1;
	}

	const char *updateSQL = "UPDATE OCPP_Orders SET StartResponse=1 WHERE TransactionId=?";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, updateSQL, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "准备更新语句时出错: %s\n", sqlite3_errmsg(orderDB));
		return -1; // 准备语句失败
	}

	sqlite3_bind_int(stmt, 1, transactionId);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "更新数据时出错: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt);
		return -1; // 更新数据失败
	}

	sqlite3_finalize(stmt);

	return 0; // 更新成功
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：读取未完成订单
 * 描    述: 按顺序读取数据库里面TYPE为0和1，且服务器未回复订单
 *
 * 参    数: order - [接收的结构体]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
bool ocpp_chargeOrder_read(CHARGE_ORDER_S *order)
{
	if (orderDB == NULL || order == NULL)
	{
		return false;
	}

	const char *selectSQL = "SELECT * FROM OCPP_Orders WHERE StopResponse = 0 AND (Type = 1 OR Type = 2) ORDER BY ID DESC LIMIT 1";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(orderDB, selectSQL, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "准备查询语句时出错: %s\n", sqlite3_errmsg(orderDB));
		return false; // 准备语句失败
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		order->connector = sqlite3_column_int(stmt, 1);
		order->transactionId = sqlite3_column_int(stmt, 2);
		const char *idTag = (const char *)sqlite3_column_text(stmt, 3);
		if (idTag)
			strcpy(order->idTag, idTag);
		order->startMeterValue = sqlite3_column_double(stmt, 4);
		const char *startTimestamp = (const char *)sqlite3_column_text(stmt, 5);
		if (startTimestamp)
			strcpy(order->startTimestamp, startTimestamp);
		order->startResponse = sqlite3_column_int(stmt, 6);
		order->stopMeterValue = sqlite3_column_double(stmt, 7);
		const char *stopTimestamp = (const char *)sqlite3_column_text(stmt, 8);
		if (stopTimestamp)
			strcpy(order->stopTimestamp, stopTimestamp);
		order->stopResponse = sqlite3_column_int(stmt, 9);
		order->reason = sqlite3_column_int(stmt, 10);
		order->type = sqlite3_column_int(stmt, 11);

		sqlite3_finalize(stmt);
		return true; // 成功读取到记录
	}
	else if (rc == SQLITE_DONE)
	{
		// 没有符合条件的记录
		sqlite3_finalize(stmt);
		return false;
	}
	else
	{
		// 查询出错
		fprintf(stderr, "查询数据时出错: %s\n", sqlite3_errmsg(orderDB));
		sqlite3_finalize(stmt);
		return false;
	}
}

#define TMP_LOG_FLAG 0X12345678
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名： 临时订单保存
 * 描    述: 根据connector号保存它的订单信息
 *
 * 参    数: connector - [枪号]
 * 参    数: order - [订单数据]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void ocpp_tempOrder_save(int connector, CHARGE_ORDER_S *order)
{
	int start_addr;
	int temp_flag;
	if (connector == 0)
		start_addr = 0;
	else
		start_addr = 4096;

	if (s32_fram_data_write(start_addr + 4, (U8_T *)order, sizeof(CHARGE_ORDER_S)) == 0)
	{
		temp_flag = TMP_LOG_FLAG;
		s32_fram_data_write(start_addr, (U8_T *)(&temp_flag), sizeof(temp_flag));
	}
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名： 读取临时订单信息
 * 描    述: 根据connector号读取它的订单信息
 *
 * 参    数: connector - [枪号]
 * 参    数: order - [订单数据]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int ocpp_tempOrder_read(int connector, CHARGE_ORDER_S *order)
{
	int start_addr;
	int temp_flag;

	if (connector == 0)
		start_addr = 0;
	else
		start_addr = 4096;

	temp_flag = 0;
	if ((s32_fram_data_read(start_addr, (U8_T *)(&temp_flag), sizeof(temp_flag)) == 0) && (temp_flag == TMP_LOG_FLAG))
	{
		s32_fram_data_read(start_addr + 4, (U8_T *)order, sizeof(CHARGE_ORDER_S));
		return 0;
	}

	return -1;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名： 清除临时订单信息
 * 描    述: 根据connector号清除它的订单信息
 *
 * 参    数: connector - [枪号]
 * 参    数: order - [订单数据]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void ocpp_tempOrder_clear(int connector)
{
	int temp_flag;
	if (connector == 0)
	{
		s32_fram_data_write(0, (U8_T *)(&temp_flag), sizeof(temp_flag));
	}
	else
	{
		s32_fram_data_write(4096, (U8_T *)(&temp_flag), sizeof(temp_flag));
	}
}

int ocpp_charge_order_init()
{
	if (open_database(ORDER_DATA_PATH) != 0)
	{
		return -1;
	}
	// 检查配置表是否存在
	if (ocpp_ChargingOrders_create_tables() == -1)
	{
		printf("ocpp_ChargingOrders_create_tables  fail\n");
		close_database();
		return -1;
	}

	return 0;
}