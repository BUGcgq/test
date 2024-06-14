#ifndef __CHARGING_H_
#define __CHARGING_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ToolFunc.h"
#include <sqlite3.h>
#include<stdbool.h>

#define ORDER_MAX_CNT 5000                      // 订单最大数目
#define ORDER_MAX_OVERFLOW_CNT 1000             // 超5000后要删除的数量
#define ORDER_DATA_PATH        "/app/etc/order.db" 
#define ORDER_DATALOG_PATH "/app/etc/order.csv" // 订单记录
#define ORDER_DATALOG_NAME "order.csv"          // 订单记录文件名

typedef struct
{
    U32_T connector;//枪号
    U32_T transactionId;//事物ID
    U8_T  idTag[21]; // 根据实际最大长度来调整大小
    F32_T startMeterValue;//开始电表值
    U8_T  startTimestamp[32]; // 根据实际格式来调整大小
    U32_T startResponse;//事物回复
    F32_T stopMeterValue;//停止电表值
    U8_T  stopTimestamp[32]; // 根据实际格式来调整大小
    U32_T stopResponse;//事物回复
    U32_T reason;//停止原因
    U32_T type; //启动类型 1 - OCPP启动，2 - OCPP本地缓存启动，3 - 白名单启动
}CHARGE_ORDER_S;

int ocpp_charge_order_init();
int ocpp_chargeOrder_insert(const CHARGE_ORDER_S *order);
int ocpp_chargeOrder_update(int transactionId, const CHARGE_ORDER_S *order);
int ocpp_chargeOrder_StopResponse_update(int transactionId);
int ocpp_chargeOrder_transactionId_update(int oldTransactionId, int newTransactionId);
int ocpp_chargeOrder_startResponse_update(int transactionId);
bool ocpp_chargeOrder_read(CHARGE_ORDER_S *order);

void ocpp_tempOrder_save(int connector, CHARGE_ORDER_S *order);
int ocpp_tempOrder_read(int connector, CHARGE_ORDER_S *order);
void ocpp_tempOrder_clear(int connector);
#ifdef __cplusplus
}
#endif

#endif
