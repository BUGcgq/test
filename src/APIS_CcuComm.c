#include "APIS_CcuComm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_mutex_t g_ccuLock = PTHREAD_MUTEX_INITIALIZER; // 初始化互斥锁
static U32_T g_ccu_com_fd = -1;
static U16_T g_rectNum = 0;
static DATA_RECT_INFO_T g_rectData[RECT_MODULE_MAX_NUM];
static DATA_CHARGER_INFO_T g_gunData[2];
static DATA_SYS_INFO_T g_sysData;
static CFG_CONTROLLER_INFO_T g_conTroCfg;
static U32_T lastSendAddr;
/* 串口发送和接收缓冲区定义 */
U8_T g_rxBuf[CCU_COM_RX_BUF_SIZE];
U16_T g_rxLen = 0;

typedef struct
{
    CCU_SERVICE_TYPE_E service;
    void *callback;
} CCU_SERVICE_MAP_T;

static CCU_SERVICE_MAP_T g_ccuServiceMap[] =
    {
        {CCU_RECV_ERROR, NULL},
        {CCU_RECV_CONFIG_FINISH, NULL},
        {CCU_RECV_MESSAGE, NULL},
        {CCU_RECV_CONFIG_REQ, NULL},
        {CCU_RECV_DI_STATE, NULL},
        {CCU_RECV_CHARGE_STATE, NULL},
        {CCU_RECV_GUN_STATE, NULL},
        {CCU_RECV_RECT_FAULT, NULL},
        {CCU_RECV_CCU_STATE1_FAULT, NULL},
        {CCU_RECV_CCU_STATE2_FAULT, NULL}};

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-05-13
 * 函 数 名：CCU_GetServiceCallback
 * 描    述: 获取回调
 *
 * 参    数: service - [回调类型]
 * 返回类型：void*
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static void *CCU_GetServiceCallback(CCU_SERVICE_TYPE_E service)
{
    if (service < 0 || service >= sizeof(g_ccuServiceMap) / sizeof(CCU_SERVICE_MAP_T))
    {
        return NULL;
    }
    return g_ccuServiceMap[service].callback;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-05-13
 * 函 数 名：ET_CCU_RegisterCallback
 * 描    述: 回调注册
 *
 * 参    数: service - [回调类型]
 * 参    数: callback - [回调函数]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void ET_CCU_RegisterCallback(CCU_SERVICE_TYPE_E service, void *callback)
{
    if (service < 0 || service >= sizeof(g_ccuServiceMap) / sizeof(CCU_SERVICE_MAP_T))
    {
        return;
    }

    g_ccuServiceMap[service].callback = callback;
}

/*************************************************************
函数名称: CCU_CalculateCrc
函数功能: crc校验计算函数
输入参数: pu8_buf  -- 指向校验的数据区
          u32_len  -- 数据长度
输出参数: 无
返回值  ：无
**************************************************************/
static U16_T CCU_CalculateCrc(U8_T *pu8_buf, U32_T u32_len)
{
    U16_T crc, q, r;

    crc = 0xFFFF;
    for (q = 0; q < u32_len; q++)
    {
        crc ^= pu8_buf[q];
        for (r = 0; r < 8; r++)
        {
            if (crc & 1)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
                crc >>= 1;
        }
    }
    return crc;
}

static U32_T CCU_PrintHexData(const char *label, const unsigned char *data, size_t length)
{
#if CCU_DEBUG_MSG == 1
    if (label == NULL || data == NULL)
    {
        return 1; // Return error code if input is invalid
    }
    int i;
    printf("%s: ", label);
    for (i = 0; i < length; i++)
    {
        printf("0x%02X ", data[i]);
    }
    printf("\n");
#endif
    return 0;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-22
 * 函 数 名：ET_CCU_TelemetryDataCmd
 * 描    述: 发送遥测数据命令
 *
 * 参    数: fd - [文件描述符]
 * 参    数: startAddr - [起始地址]
 * 参    数: dataLen - [数据长度]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static U32_T ET_CCU_TelemetryDataCmd(U32_T fd, U32_T startAddr, U16_T dataLen)
{
    pthread_mutex_lock(&g_ccuLock);
    if (fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;        // 从机地址
    txBuf[1] = READ_INPUT_REGISTERS; // 操作方式
    txBuf[2] = startAddr >> 8;
    txBuf[3] = startAddr;
    txBuf[4] = dataLen >> 8;
    txBuf[5] = dataLen;
    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 6);
    txBuf[6] = (U8_T)u16_crc;
    txBuf[7] = (U8_T)(u16_crc >> 8);

    U32_T ret = com_send_data(fd, txBuf, 8);

    lastSendAddr = startAddr;
    pthread_mutex_unlock(&g_ccuLock); // 解锁

    if (ret > 0)
    {
        CCU_PrintHexData("发送遥测数据命令", txBuf, 8);
    }

    return ret;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-22
 * 函 数 名：ET_CCU_ReadDataCmd
 * 描    述: 读取寄存器命令
 *
 * 参    数: fd - [文件描述符]
 * 参    数: startAddr - [起始地址]
 * 参    数: dataLen - [数据长度]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */

static U32_T ET_CCU_ReadDataCmd(U32_T fd, U32_T startAddr, U16_T dataLen)
{
    pthread_mutex_lock(&g_ccuLock);
    if (fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;          // 从机地址
    txBuf[1] = READ_HOLDING_REGISTERS; // 操作方式
    txBuf[2] = startAddr >> 8;
    txBuf[3] = startAddr;
    txBuf[4] = dataLen >> 8;
    txBuf[5] = dataLen;
    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 6);
    txBuf[6] = (U8_T)u16_crc;
    txBuf[7] = (U8_T)(u16_crc >> 8);

    U32_T ret = com_send_data(fd, txBuf, 8);
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    return ret;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-26
 * 函 数 名：ET_CCU_TelemetrySysInfo
 * 描    述: 充电桩系统遥测数据
 *
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_TelemetrySysInfo()
{
    return ET_CCU_TelemetryDataCmd(g_ccu_com_fd, TELEMETRY_SYS_DATA_START_ADDR, TELEMETRY_SYS_DATA_LEN);
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-22
 * 函 数 名：ET_CCU_GetFreeGunData
 * 描    述: 遥测枪空闲数据
 *
 * 参    数: gunID - [枪id]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_TelemetryFreeGunData(U8_T gunID)
{

    if (gunID == 0)
    {
        return ET_CCU_TelemetryDataCmd(g_ccu_com_fd, TELEMETRY_GUN1_DATA_START_ADDR, TELEMETRY_IDLE_DATA_LEN);
    }
    else
    {
        return ET_CCU_TelemetryDataCmd(g_ccu_com_fd, TELEMETRY_GUN2_DATA_START_ADDR, TELEMETRY_IDLE_DATA_LEN);
    }
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-22
 * 函 数 名：ET_CCU_GetChargeGunData
 * 描    述: 遥测枪充电数据
 *
 * 参    数: gunID - [枪id]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_TelemetryChargeGunData(U8_T gunID)
{
    if (gunID == 0)
    {
        return ET_CCU_TelemetryDataCmd(g_ccu_com_fd, TELEMETRY_GUN1_DATA_START_ADDR, TELEMETRY_CHARGE_DATA_LEN);
    }
    else
    {
        return ET_CCU_TelemetryDataCmd(g_ccu_com_fd, TELEMETRY_GUN2_DATA_START_ADDR, TELEMETRY_CHARGE_DATA_LEN);
    }
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-22
 * 函 数 名：ET_CCU_GetRectModuleData
 * 描    述: 遥测模块数据
 *
 * 参    数: num - [模块数量]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_TelemetryRectModuleData()
{
    U8_T len = g_rectNum * TELEMETRY_RECT_DATA_LEN;
    return ET_CCU_TelemetryDataCmd(g_ccu_com_fd, TELEMETRY_RECT_DATA_START_ADDR, len);
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-05-14
 * 函 数 名：ET_CCU_ParseSysData
 * 描    述: 解析CCU系统基础数据
 *
 * 参    数: n - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static U32_T ET_CCU_ParseSysData(U32_T n)
{
    U16_T u16_temp_data = 0;
    static U16_T u16_diState = 0;
    U32_T(*fn_RecvDiState)
    (U16_T) = CCU_GetServiceCallback(CCU_RECV_DI_STATE);
    U32_T offset = n + 3;
    DATA_SYS_INFO_T data;
    memset(&data, 0, sizeof(DATA_SYS_INFO_T));

    // 充电桩内部通信协议版本
    u16_temp_data = (g_rxBuf[offset] << 8) | (g_rxBuf[offset + 1]);
    data.version = u16_temp_data;

    // 整流模块输出电压
    u16_temp_data = (g_rxBuf[offset + 2] << 8) | (g_rxBuf[offset + 3]);
    data.ccuSwVer = u16_temp_data;

    // 整流模块输出电流
    u16_temp_data = (g_rxBuf[offset + 6] << 8) | (g_rxBuf[offset + 7]);
    data.tcuHwVer = u16_temp_data;

    // 充电桩环境温度
    u16_temp_data = (g_rxBuf[offset + 10] << 8) | (g_rxBuf[offset + 11]);
    data.caseTemp = (F32_T)u16_temp_data / 10;

    // CCU控制器DI状态
    u16_temp_data = (g_rxBuf[offset + 12] << 8) | (g_rxBuf[offset + 13]);
    data.diStatus = u16_temp_data;

    pthread_mutex_lock(&g_ccuLock);
    memcpy(&g_sysData, &data, sizeof(DATA_SYS_INFO_T));
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    if (u16_diState != data.diStatus)
    {
        u16_diState = data.diStatus;

        if (fn_RecvDiState != NULL)
        {
            fn_RecvDiState(u16_diState);
        }
    }
    return 0;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-23
 * 函 数 名：ET_CCU_TelemetrychargeData
 * 描    述: 解析充电枪数据
 *
 * 参    数: n - [参数说明]
 * 参    数: gunID - [枪号]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static U32_T ET_CCU_ParseChargeData(U32_T n, U32_T gunID)
{
    U16_T u16_temp_data = 0;
    U32_T u32_temp_data = 0;
    U64_T u64_temp_data = 0;
    static U16_T u16_gunState = 0;
    static U16_T u16_chargeState = 0;

    U32_T offset = n + 3;

    U32_T(*fn_RecvChargeState)
    (U16_T, U16_T) = CCU_GetServiceCallback(CCU_RECV_CHARGE_STATE);
    U32_T(*fn_RecvGunState)
    (U16_T, U16_T) = CCU_GetServiceCallback(CCU_RECV_GUN_STATE);
    U32_T(*fn_RecvState1Fault)
    (U16_T, U16_T) = CCU_GetServiceCallback(CCU_RECV_CCU_STATE1_FAULT);
    U32_T(*fn_RecvState2Fault)
    (U16_T, U16_T) = CCU_GetServiceCallback(CCU_RECV_CCU_STATE2_FAULT);

    DATA_CHARGER_INFO_T data;
    memset(&data, 0, sizeof(DATA_CHARGER_INFO_T));

    U32_T i;

    // 充电机状态
    u16_temp_data = (g_rxBuf[offset] << 8) | (g_rxBuf[offset + 1]);
    data.gunData.chgState = u16_temp_data;

    // 充电枪连接状态
    u16_temp_data = (g_rxBuf[offset + 2] << 8) | (g_rxBuf[offset + 3]);
    data.gunData.gunConnState = u16_temp_data;

    // 充电枪锁状态
    u16_temp_data = (g_rxBuf[offset + 4] << 8) | (g_rxBuf[offset + 5]);
    data.gunData.gunLockState = u16_temp_data;

    // 充电枪归位状态
    u16_temp_data = (g_rxBuf[offset + 6] << 8) | (g_rxBuf[offset + 7]);
    data.gunData.gunResState = u16_temp_data;

    // CCS1电压
    u16_temp_data = (g_rxBuf[offset + 8] << 8) | g_rxBuf[offset + 9];
    data.gunData.ccs1Volt = (F32_T)u16_temp_data / 100;

    // 枪温度1
    u16_temp_data = (g_rxBuf[offset + 10] << 8) | (g_rxBuf[offset + 11]);
    data.gunData.gunTemp1 = (F32_T)(u16_temp_data) / 10 - 50;

    // 枪温度2
    u16_temp_data = (g_rxBuf[offset + 12] << 8) | (g_rxBuf[offset + 13]);
    data.gunData.gunTemp2 = (F32_T)(u16_temp_data) / 10 - 50;

    // 枪状态STATE1
    u16_temp_data = (g_rxBuf[offset + 14] << 8) | (g_rxBuf[offset + 15]);
    data.gunData.gunState1 = u16_temp_data;

    // 枪状态STATE2
    u16_temp_data = (g_rxBuf[offset + 16] << 8) | (g_rxBuf[offset + 17]);
    data.gunData.gunState2 = u16_temp_data;

    // 枪状态STATE3
    u16_temp_data = (g_rxBuf[offset + 18] << 8) | (g_rxBuf[offset + 19]);
    data.gunData.gunState3 = u16_temp_data;

    // 枪最大功率
    u16_temp_data = (g_rxBuf[offset + 20] << 8) | (g_rxBuf[offset + 21]);
    data.gunData.gunMaxRate = (F32_T)(u16_temp_data) / 10;

    // 枪最大输出电流
    u16_temp_data = (g_rxBuf[offset + 22] << 8) | (g_rxBuf[offset + 23]);
    data.gunData.gunMaxCurr = (F32_T)(u16_temp_data) / 10;

    // 枪启动失败原因
    u32_temp_data = (g_rxBuf[offset + 30] << 24) | (g_rxBuf[offset + 31] << 16) |
                    (g_rxBuf[offset + 32] << 8) | (g_rxBuf[offset + 33]);
    data.gunData.gunStarFailReson = (U32_T)u32_temp_data;

    // 枪停机原因
    u64_temp_data = (g_rxBuf[offset + 34] << 56) | (g_rxBuf[offset + 35] << 48) |
                    (g_rxBuf[offset + 36] << 40) | (g_rxBuf[offset + 37] << 32) |
                    (g_rxBuf[offset + 38] << 24) | (g_rxBuf[offset + 39] << 16) |
                    (g_rxBuf[offset + 40] << 8) | (g_rxBuf[offset + 41]);
    data.gunData.gunstopReson = (U64_T)u64_temp_data;

    // 电表测量电压值
    u16_temp_data = (g_rxBuf[offset + 42] << 8) | (g_rxBuf[offset + 43]);
    data.gunData.meterVolt = (F32_T)(u16_temp_data) / 10;

    // 电表测量电流值
    u16_temp_data = (g_rxBuf[offset + 44] << 8) | (g_rxBuf[offset + 45]);
    data.gunData.meterCurr = (F32_T)(u16_temp_data) / 10;

    // 电表当前读数
    u64_temp_data = (g_rxBuf[offset + 46] << 56) | (g_rxBuf[offset + 47] << 48) |
                    (g_rxBuf[offset + 48] << 40) | (g_rxBuf[offset + 49] << 32) |
                    (g_rxBuf[offset + 50] << 24) | (g_rxBuf[offset + 51] << 16) |
                    (g_rxBuf[offset + 52] << 8) | (g_rxBuf[offset + 53]);
    data.gunData.meterRead = (F64_T)(u64_temp_data) / 1000;

    // 母线正极到地电压
    u16_temp_data = (g_rxBuf[offset + 70] << 8) | (g_rxBuf[offset + 71]);
    data.gunData.busPosToGroundVolt = (F32_T)u16_temp_data / 10;

    // 母线负极到地电压
    u16_temp_data = (g_rxBuf[offset + 72] << 8) | (g_rxBuf[offset + 73]);
    data.gunData.busNegToGroundVolt = (F32_T)u16_temp_data / 10;

    // 母线正极到地电阻
    u16_temp_data = (g_rxBuf[offset + 74] << 8) | (g_rxBuf[offset + 75]);
    data.gunData.busPosToGroundRes = (F32_T)u16_temp_data / 10;

    // 母线负极到地电阻
    u16_temp_data = (g_rxBuf[offset + 76] << 8) | (g_rxBuf[offset + 77]);
    data.gunData.busNegToGroundRes = (F32_T)u16_temp_data / 10;

    // 母线A相输出电压
    u16_temp_data = (g_rxBuf[offset + 78] << 8) | (g_rxBuf[offset + 79]);
    data.gunData.busOutVoltSideA = (F32_T)u16_temp_data / 10;

    // 母线A相输出电流
    u16_temp_data = (g_rxBuf[offset + 80] << 8) | (g_rxBuf[offset + 81]);
    data.gunData.busOutCuSideA = (F32_T)u16_temp_data / 10;

    // 母线B相输出电压
    u16_temp_data = (g_rxBuf[offset + 82] << 8) | (g_rxBuf[offset + 83]);
    data.gunData.busOutVoltSideB = (F32_T)u16_temp_data / 10;

    // 母线B相输出电流
    u16_temp_data = (g_rxBuf[offset + 84] << 8) | (g_rxBuf[offset + 85]);
    data.gunData.busOutCuSideB = (F32_T)u16_temp_data / 10;

    // 单体动力蓄电池最高允许充电电压
    u16_temp_data = (g_rxBuf[offset + 94] << 8) | (g_rxBuf[offset + 95]);
    data.bmsData.cellmaxlAlwChgVolt = (F32_T)(u16_temp_data) / 100;

    // 最高允许充电电流
    u16_temp_data = (g_rxBuf[offset + 96] << 8) | (g_rxBuf[offset + 97]);
    data.bmsData.maxAlwChgCurr = (F32_T)(u16_temp_data) / 10;

    // 动力蓄电池标称总能量
    u16_temp_data = (g_rxBuf[offset + 98] << 8) | (g_rxBuf[offset + 99]);
    data.bmsData.nominalTotalElect = (F32_T)(u16_temp_data) / 10;

    // 最高允许充电总电压
    u16_temp_data = (g_rxBuf[offset + 100] << 8) | (g_rxBuf[offset + 101]);
    data.bmsData.maxAlwChgVolt = (F32_T)(u16_temp_data) / 10;

    // 最高允许动力蓄电池温度
    u16_temp_data = (g_rxBuf[offset + 102] << 8) | (g_rxBuf[offset + 103]);
    data.bmsData.maxAlwTemp = u16_temp_data - 5;

    // 整车动力蓄电池荷电状态
    u16_temp_data = (g_rxBuf[offset + 104] << 8) | (g_rxBuf[offset + 105]);
    data.bmsData.batEleSOC = (F32_T)(u16_temp_data) / 10;

    // 接收bms报文标志
    u16_temp_data = (g_rxBuf[offset + 106] << 8) | (g_rxBuf[offset + 107]);
    data.bmsData.bmsFlag = u16_temp_data;

    // 电压需求
    u16_temp_data = (g_rxBuf[offset + 108] << 8) | (g_rxBuf[offset + 109]);
    data.bmsData.batNeedVolt = (F32_T)(u16_temp_data) / 10;

    // 电流需求
    u16_temp_data = (g_rxBuf[offset + 110] << 8) | (g_rxBuf[offset + 111]);
    data.bmsData.battNeedCurr = (F32_T)(u16_temp_data) / 10;

    // 充电模式
    u16_temp_data = (g_rxBuf[offset + 112] << 8) | (g_rxBuf[offset + 113]);
    data.bmsData.chgMode = u16_temp_data;

    // 充电电压测量值
    u16_temp_data = (g_rxBuf[offset + 114] << 8) | (g_rxBuf[offset + 115]);
    data.bmsData.chgVolt = (F32_T)(u16_temp_data) / 10;

    // 充电电流测量值
    u16_temp_data = (g_rxBuf[offset + 116] << 8) | (g_rxBuf[offset + 117]);
    data.bmsData.chgCurr = (F32_T)(u16_temp_data) / 10;

    // 最高单体动力蓄电池电压
    u16_temp_data = (g_rxBuf[offset + 118] << 8) | (g_rxBuf[offset + 119]);
    data.bmsData.cellMaxChgVolt = (F32_T)(u16_temp_data) / 10;

    // 最高电压电池所在电池包序号
    u16_temp_data = (g_rxBuf[offset + 120] << 8) | (g_rxBuf[offset + 121]);
    data.bmsData.cellMaxVoltGroupNum = u16_temp_data;

    // 电池荷电状态
    u16_temp_data = (g_rxBuf[offset + 122] << 8) | (g_rxBuf[offset + 123]);
    data.bmsData.batSoc = u16_temp_data;

    // 剩余充电时间
    u16_temp_data = (g_rxBuf[offset + 124] << 8) | (g_rxBuf[offset + 125]);
    data.bmsData.remainChgTime = u16_temp_data;

    // 最高单体动力蓄电池电压电池号
    u16_temp_data = (g_rxBuf[offset + 126] << 8) | (g_rxBuf[offset + 127]);
    data.bmsData.cellMaxVoltID = u16_temp_data;

    // 整车动力蓄电池最高温度
    u16_temp_data = (g_rxBuf[offset + 128] << 8) | (g_rxBuf[offset + 129]);
    data.bmsData.batMaxTemp = (F32_T)(u16_temp_data)-50;

    // 最高温度探针序号
    u16_temp_data = (g_rxBuf[offset + 130] << 8) | (g_rxBuf[offset + 131]);
    data.bmsData.maxTempID = u16_temp_data;

    // 整车动力蓄电池最低温度
    u16_temp_data = (g_rxBuf[offset + 132] << 8) | (g_rxBuf[offset + 133]);
    data.bmsData.batMinTemp = (F32_T)(u16_temp_data)-50;

    // 最低温度探针序号
    u16_temp_data = (g_rxBuf[offset + 134] << 8) | (g_rxBuf[offset + 135]);
    data.bmsData.minTempID = u16_temp_data;

    // 电池状态
    u16_temp_data = (g_rxBuf[offset + 136] << 8) | (g_rxBuf[offset + 137]);
    data.bmsData.batState = u16_temp_data;

    // 电池终止荷电状态
    u16_temp_data = (g_rxBuf[offset + 138] << 8) | (g_rxBuf[offset + 139]);
    data.bmsData.batEndSoc = u16_temp_data;

    // 动力蓄电池单体电压终止允许充电最低值
    u16_temp_data = (g_rxBuf[offset + 140] << 8) | (g_rxBuf[offset + 141]);
    data.bmsData.cellEndMinVolt = (F32_T)(u16_temp_data) / 100;

    // 动力蓄电池单体电压终止允许充电最高值
    u16_temp_data = (g_rxBuf[offset + 142] << 8) | (g_rxBuf[offset + 143]);
    data.bmsData.cellEndMaxVolt = (F32_T)(u16_temp_data) / 100;

    // 电池终止最低温度
    u16_temp_data = (g_rxBuf[offset + 144] << 8) | (g_rxBuf[offset + 145]);
    data.bmsData.batEndMinTemp = (F32_T)(u16_temp_data)-50;

    // 电池终止最高温度
    u16_temp_data = (g_rxBuf[offset + 146] << 8) | (g_rxBuf[offset + 147]);
    data.bmsData.batEndMaxTemp = (F32_T)(u16_temp_data)-50;

    // 车辆识别码
    for (i = 0; i < 17; i++)
    {
        data.bmsData.vin[i] = g_rxBuf[offset + i + 148];
    }
    data.bmsData.vin[17] = '\0';
    pthread_mutex_lock(&g_ccuLock);
    memcpy(&g_gunData[gunID], &data, sizeof(DATA_CHARGER_INFO_T));
    pthread_mutex_unlock(&g_ccuLock); // 解锁

    if (u16_chargeState != data.gunData.chgState)
    {
        u16_chargeState = data.gunData.chgState;

        if (fn_RecvChargeState != NULL)
        {
            fn_RecvChargeState(gunID, u16_chargeState);
        }
    }

    if (u16_gunState != data.gunData.gunConnState)
    {
        u16_gunState = data.gunData.gunConnState;

        if (fn_RecvGunState != NULL)
        {
            fn_RecvGunState(gunID, u16_gunState);
        }
    }

    if (data.gunData.gunState1 != 0)
    {
        if (fn_RecvState1Fault != NULL)
        {
            fn_RecvState1Fault(gunID, data.gunData.gunState1);
        }
    }

    if (data.gunData.gunState2 != 0)
    {
        if (fn_RecvState2Fault != NULL)
        {
            fn_RecvState2Fault(gunID, data.gunData.gunState2);
        }
    }
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-23
 * 函 数 名：ET_CCU_ParseRectData
 * 描    述: 解析模块数据
 *
 * 参    数: n - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static U32_T ET_CCU_ParseRectData(U32_T n)
{
    U16_T u16_temp_data = 0;
    U32_T u32_temp_data = 0;
    U32_T i, offset;
    U32_T(*fn_RecvRectFault)
    (U16_T, U16_T) = CCU_GetServiceCallback(CCU_RECV_RECT_FAULT);
    pthread_mutex_lock(&g_ccuLock);
    for (i = 0; i < g_rectNum; i++)
    {

        offset = i * TELEMETRY_RECT_DATA_LEN * 2 + 3;
        // 整流模块通信状态定义
        u16_temp_data = (g_rxBuf[offset] << 8) | (g_rxBuf[offset + 1]);
        g_rectData[i].commState = u16_temp_data;
        // 整流模块状态定义
        u32_temp_data = (g_rxBuf[offset + 2] << 24) | (g_rxBuf[offset + 3] << 16) |
                        (g_rxBuf[offset + 4] << 8) | (g_rxBuf[offset + 5]);
        g_rectData[i].rectState = u32_temp_data;
        // 整流模块输出电压
        u16_temp_data = (g_rxBuf[offset + 6] << 8) | (g_rxBuf[offset + 7]);
        g_rectData[i].rectOutVolt = (F32_T)u16_temp_data / 10;
        // 整流模块输出电流
        u16_temp_data = (g_rxBuf[offset + 8] << 8) | (g_rxBuf[offset + 9]);
        g_rectData[i].rectOutCurr = (F32_T)u16_temp_data / 10;
        // 整流模块限流点
        u16_temp_data = (g_rxBuf[offset + 10] << 8) | (g_rxBuf[offset + 11]);
        g_rectData[i].rectLimitPoint = (F32_T)u16_temp_data / 10;
        // 整流模块温度
        u16_temp_data = (g_rxBuf[offset + 12] << 8) | (g_rxBuf[offset + 13]);
        g_rectData[i].rectTemp = (F32_T)u16_temp_data / 10;
        // 整流模块输出功率
        u16_temp_data = (g_rxBuf[offset + 14] << 8) | (g_rxBuf[offset + 15]);
        g_rectData[i].rectOutPower = (F32_T)u16_temp_data / 10;

        if (g_rectData[i].rectState != 0)
        {
            if (fn_RecvRectFault != NULL)
            {
                fn_RecvRectFault(i, g_rectData[i].rectState);
            }
        }
    }
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    return 0;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-26
 * 函 数 名：ET_CCU_ParseTelemetryData
 * 描    述: 解析遥测数据
 *
 * 参    数: n - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static U32_T ET_CCU_ParseTelemetryData(U32_T n)
{
    U32_T(*fn_RecvMsg)
    () = CCU_GetServiceCallback(CCU_RECV_MESSAGE);
    switch (lastSendAddr)
    {
    case TELEMETRY_SYS_DATA_START_ADDR:
        ET_CCU_ParseSysData(n);
        break;
    case TELEMETRY_GUN1_DATA_START_ADDR:
        ET_CCU_ParseChargeData(n, 0);
        break;
    case TELEMETRY_GUN2_DATA_START_ADDR:
        ET_CCU_ParseChargeData(n, 1);
        break;
    case TELEMETRY_RECT_DATA_START_ADDR:
        ET_CCU_ParseRectData(n);
        break;
    default:
        return;
    }

    if (fn_RecvMsg != NULL)
    {
        fn_RecvMsg();
    }
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-26
 * 函 数 名：ET_CCU_RecvErrorData
 * 描    述: 解析错误帧
 *
 * 参    数: n - [参数说明]
 * 参    数: index - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static U32_T ET_CCU_ParseErrorData(U32_T n, U32_T index)
{
    U16_T u16_error_code = 0;
    U16_T u8_original_index = index & 0x7F; // 将错误码的最高位清零，得到原始的功能码

    u16_error_code = (g_rxBuf[n + 2] << 8) | g_rxBuf[n + 3];

    U32_T(*fn_RecvError)
    (U16_T, U16_T) = CCU_GetServiceCallback(CCU_RECV_ERROR);
    if (fn_RecvError != NULL)
    {
        fn_RecvError(u8_original_index, u16_error_code);
    }
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-26
 * 函 数 名：CCU_RecvCmdHandle
 * 描    述: 分类解析接收的数据
 *
 * 参    数: n - [参数说明]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */

static U32_T CCU_RecvCmdHandle(U32_T n)
{
    U8_T index = g_rxBuf[n + 1];
    U32_T(*fn_RecvConfigReq)
    () = CCU_GetServiceCallback(CCU_RECV_CONFIG_REQ);
    U32_T(*fn_RecvConfFinish)
    () = CCU_GetServiceCallback(CCU_RECV_CONFIG_FINISH);
    switch (index)
    {
    case READ_HOLDING_REGISTERS:
        break;
    case READ_INPUT_REGISTERS:
        ET_CCU_ParseTelemetryData(n);
        break;
    case WRITE_HOLDING_REGISTER:

        break;
    case DIAGNOSTICS:
        if (fn_RecvConfigReq != NULL)
        {
            fn_RecvConfigReq();
        }
        break;
    case MASK_WRITE_REGISTER:
        if (fn_RecvConfFinish != NULL)
        {
            fn_RecvConfFinish();
        }
        break;
    default:
        if ((index & 0x80) == 0x80)
        {
            ET_CCU_ParseErrorData(n, index);
        }
        else
        {
            printf("Unknown message\n");
        }
        break;
    }

    return 0;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-22
 * 函 数 名：
 * 描    述:
 *
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_UsartRecvData(void)
{
    if (g_ccu_com_fd == -1)
    {
        printf("接收数据失败\n");
        return -1;
    }
    U8_T j;
    U32_T i, h;
    U16_T u16_byte_len = 0;
    g_rxLen = com_recv_data(g_ccu_com_fd, g_rxBuf, CCU_COM_RX_BUF_SIZE);
    if (g_rxLen > 0)
    {
        CCU_PrintHexData("接收到数据", g_rxBuf, g_rxLen);
    }
    h = 0;
    /*查找完整的数据包*/
    while (g_rxLen >= 6)
    {
        // 判断地址位
        if (g_rxBuf[h] != SLAVE_ADDRESS)
        {
            printf("地址接收错误 %X----%X\n", g_rxBuf[0], g_rxBuf[h]);
            h++;
            g_rxLen--;
            continue;
        }
        // 遥测数据才判断长度
        if (g_rxBuf[h + 1] == READ_HOLDING_REGISTERS || g_rxBuf[h + 1] == READ_INPUT_REGISTERS)
        {
            // 获取数据长度
            u16_byte_len = (g_rxBuf[h + 2]) + 5;
            // 接收长度过短
            if ((g_rxLen) < u16_byte_len) // 接收出错
            {
                printf("接收长度过短\n");
                h++;
                g_rxLen--;
                continue;
            }
            if (CCU_CalculateCrc(&g_rxBuf[h], u16_byte_len) != 0)
            {
                printf("CRC错误\n");
                h++;
                g_rxLen--;
                continue;
            }
        }
        else if (g_rxBuf[h + 1] == WRITE_HOLDING_REGISTER || g_rxBuf[h + 1] == MASK_WRITE_REGISTER)
        {
            // 获取数据长度
            u16_byte_len = 8;
            // 接收长度过短
            if ((g_rxLen) < u16_byte_len) // 接收出错
            {
                printf("接收长度过短\n");
                continue;
            }
            if (CCU_CalculateCrc(&g_rxBuf[h], u16_byte_len) != 0)
            {
                printf("CRC错误\n");
                h++;
                g_rxLen--;
                continue;
            }
        }
        else if (g_rxBuf[h + 1] == DIAGNOSTICS)
        {
            // 获取数据长度
            u16_byte_len = 8;
            // 接收长度过短
            if ((g_rxLen) < u16_byte_len) // 接收出错
            {
                printf("接收长度过短\n");
                continue;
            }
            if (CCU_CalculateCrc(&g_rxBuf[h], u16_byte_len) != 0)
            {
                printf("CRC错误\n");
                h++;
                g_rxLen--;
                continue;
            }
        }
        if (h > 0)
        {
            for (i = 0; i < g_rxLen; i++)
                g_rxBuf[i] = g_rxBuf[h + i];

            h = 0;
        }

        CCU_RecvCmdHandle(h);

        g_rxLen = 0;
    }

    if (h > 0)
    {
        for (i = 0; i < g_rxLen; i++)
            g_rxBuf[i] = g_rxBuf[h + i];
    }
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-22
 * 函 数 名：ET_CCU_CommInit
 * 描    述: 串口初始化
 *
 * 参    数: config - [窗口配置]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_CommInit(CCU_CFG_T *config)
{
    if (config == NULL)
    {
        printf("传入的config为NULL\n");
        return -1;
    }

    g_ccu_com_fd = com_open_port(config->portPath, config->comBaud, config->parity);
    if (g_ccu_com_fd == -1)
    {
        printf("串口初始化失败\n");
        return -1;
    }

    return 0;
}

// 设置模块数量的接口
void ET_CCU_SetRectNum(U16_T num)
{
    if (num < 0)
    {
        return;
    }

    pthread_mutex_lock(&g_ccuLock);
    g_rectNum = num;
    pthread_mutex_unlock(&g_ccuLock); // 解锁
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-26
 * 函 数 名：ET_CCU_GetSysInfoData
 * 描    述: 获取充电桩系统遥测数据
 *
 * 参    数: sysData - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */

U32_T ET_CCU_GetSysInfoData(pstDATA_SYS_INFO_T pSysData)
{
    if (pSysData == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    memcpy(pSysData, &g_sysData, sizeof(DATA_CHARGER_INFO_T));
    pthread_mutex_unlock(&g_ccuLock); // 解锁
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-26
 * 函 数 名：ET_CCU_GetChargeData
 * 描    述: 获取充电机数据
 *
 * 参    数: gunData - [数据结构]
 * 参    数: gunID - [枪号]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */

U32_T ET_CCU_GetChargeData(U8_T gunID, pstDATA_CHARGER_INFO_T pGunData)
{
    if (pGunData == NULL || (gunID < 0 || gunID > 1))
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    memcpy(pGunData, &g_gunData[gunID], sizeof(DATA_CHARGER_INFO_T));
    pthread_mutex_unlock(&g_ccuLock); // 解锁
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-26
 * 函 数 名：ET_CCU_GetRectModuleData
 * 描    述: 获取模块数据
 *
 * 参    数: rectData - [模块结构体指针]
 * 参    数: num - [模块数量]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_GetRectModuleData(U8_T rectId, pstDATA_RECT_INFO_T pRectData)
{
    if (pRectData == NULL || rectId < 0 || rectId > g_rectNum)
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    memcpy(pRectData, &g_rectData[rectId], sizeof(DATA_RECT_INFO_T));
    pthread_mutex_unlock(&g_ccuLock); // 解锁

    return 0;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-28
 * 函 数 名：ET_CCU_SetConTroConfig
 * 描    述: 设置控制器配置
 *
 * 参    数: data - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_SetConTroConfig(pstCFG_CONTROLLER_INFO_T pCtrConfig)
{
    if (pCtrConfig == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    if (g_ccu_com_fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;       // 从机地址
    txBuf[1] = MASK_WRITE_REGISTER; // 操作方式
    txBuf[2] = CONTROLLER_CONFIG_START_ADDR >> 8;
    txBuf[3] = CONTROLLER_CONFIG_START_ADDR;
    txBuf[4] = CONTROLLER_CONFIG_DATA_LEN >> 8;
    txBuf[5] = CONTROLLER_CONFIG_DATA_LEN;
    txBuf[6] = CONTROLLER_CONFIG_DATA_LEN * 2;

    txBuf[7] = pCtrConfig->proVer >> 8;
    txBuf[8] = pCtrConfig->proVer;
    txBuf[9] = pCtrConfig->ctrlDebug >> 8;
    txBuf[10] = pCtrConfig->ctrlDebug;
    txBuf[11] = pCtrConfig->chrBusNum >> 8;
    txBuf[12] = pCtrConfig->chrBusNum;
    txBuf[13] = pCtrConfig->haveParCont >> 8;
    txBuf[14] = pCtrConfig->haveParCont;
    txBuf[15] = pCtrConfig->haveAcCont >> 8;
    txBuf[16] = pCtrConfig->haveAcCont;
    txBuf[17] = pCtrConfig->acContStat >> 8;
    txBuf[18] = pCtrConfig->acContStat;
    txBuf[19] = pCtrConfig->dcContStat >> 8;
    txBuf[20] = pCtrConfig->dcContStat;
    txBuf[21] = pCtrConfig->parContStat >> 8;
    txBuf[22] = pCtrConfig->parContStat;
    txBuf[23] = pCtrConfig->standbyLockGun >> 8;
    txBuf[24] = pCtrConfig->standbyLockGun;
    txBuf[25] = pCtrConfig->gunResDetect >> 8;
    txBuf[26] = pCtrConfig->gunResDetect;

    txBuf[27] = pCtrConfig->disChrFault >> 8;
    txBuf[28] = pCtrConfig->disChrFault;
    txBuf[29] = pCtrConfig->outPutUnctrl >> 8;
    txBuf[30] = pCtrConfig->outPutUnctrl;
    txBuf[31] = pCtrConfig->coolSysType >> 8;
    txBuf[32] = pCtrConfig->coolSysType;
    txBuf[33] = pCtrConfig->coolSysActivate >> 8;
    txBuf[34] = pCtrConfig->coolSysActivate;
    txBuf[35] = pCtrConfig->gunLockStatType >> 8;
    txBuf[36] = pCtrConfig->gunLockStatType;
    txBuf[37] = pCtrConfig->gunElecLockCtrl >> 8;
    txBuf[38] = pCtrConfig->gunElecLockCtrl;

    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 39);
    txBuf[39] = (U8_T)u16_crc;
    txBuf[40] = (U8_T)(u16_crc >> 8);

    U32_T ret = com_send_data(g_ccu_com_fd, txBuf, 41);
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    if (ret > 0)
    {
        CCU_PrintHexData("发送控制器配置", txBuf, 41);
    }
    return ret;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-28
 * 函 数 名：ET_CCU_SetChargerConfig
 * 描    述: 设置充电桩配置
 *
 * 参    数: data - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_SetChargerConfig(pstCFG_CHARGER_INFO_T pChrConfig)
{
    if (pChrConfig == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    if (g_ccu_com_fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U16_T tempData;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;       // 从机地址
    txBuf[1] = MASK_WRITE_REGISTER; // 操作方式
    txBuf[2] = CHARGER_CONFIG_START_ADDR >> 8;
    txBuf[3] = CHARGER_CONFIG_START_ADDR;
    txBuf[4] = CHARGER_CONFIG_DATA_LEN >> 8;
    txBuf[5] = CHARGER_CONFIG_DATA_LEN;
    txBuf[6] = CHARGER_CONFIG_DATA_LEN * 2;

    txBuf[7] = pChrConfig->insBridDelay >> 8;
    txBuf[8] = pChrConfig->insBridDelay;
    txBuf[9] = pChrConfig->fanStartTemp >> 8;
    txBuf[10] = pChrConfig->fanStartTemp;
    txBuf[11] = pChrConfig->fanStopTemp >> 8;
    txBuf[12] = pChrConfig->fanStopTemp;
    txBuf[13] = pChrConfig->chrOverTempThr >> 8;
    txBuf[14] = pChrConfig->chrOverTempThr;
    txBuf[15] = pChrConfig->gunCurrDownThr >> 8;
    txBuf[16] = pChrConfig->gunCurrDownThr;
    txBuf[17] = pChrConfig->gunShutDownThr >> 8;
    txBuf[18] = pChrConfig->gunShutDownThr;
    tempData = pChrConfig->bmsTimeoutCof * 10;
    txBuf[19] = tempData >> 8;
    txBuf[20] = tempData;
    txBuf[21] = pChrConfig->contAdhesionThr >> 8;
    txBuf[22] = pChrConfig->contAdhesionThr;
    tempData = pChrConfig->gunMaxCurr;
    txBuf[23] = tempData >> 8;
    txBuf[24] = tempData;
    tempData = pChrConfig->dcBusVoltA;
    txBuf[25] = tempData >> 8;
    txBuf[26] = tempData;
    tempData = pChrConfig->dcBusVoltB;
    txBuf[27] = tempData >> 8;
    txBuf[28] = tempData;

    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 29);
    txBuf[29] = (U8_T)u16_crc;
    txBuf[30] = (U8_T)(u16_crc >> 8);

    U32_T ret = com_send_data(g_ccu_com_fd, txBuf, 31);
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    if (ret > 0)
    {
        CCU_PrintHexData("发送充电桩配置", txBuf, 31);
    }
    return -1;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-28
 * 函 数 名：ET_CCU_SetRectModConfig
 * 描    述: 设置整流模块设置
 *
 * 参    数: data - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_SetRectModConfig(pstCFG_RECT_MODULE_INFO_T pRectModConfig)
{
    if (pRectModConfig == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    if (g_ccu_com_fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U16_T tempData;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;       // 从机地址
    txBuf[1] = MASK_WRITE_REGISTER; // 操作方式
    txBuf[2] = RECT_MODULE_CONFIG_START_ADDR >> 8;
    txBuf[3] = RECT_MODULE_CONFIG_START_ADDR;
    txBuf[4] = RECT_MODULE_CONFIG_DATA_LEN >> 8;
    txBuf[5] = RECT_MODULE_CONFIG_DATA_LEN;
    txBuf[6] = RECT_MODULE_CONFIG_DATA_LEN * 2;

    txBuf[7] = pRectModConfig->rectNum >> 8;
    txBuf[8] = pRectModConfig->rectNum;
    txBuf[9] = pRectModConfig->rectPro >> 8;
    txBuf[10] = pRectModConfig->rectPro;
    tempData = pRectModConfig->rectRatedCurr;
    txBuf[11] = tempData >> 8;
    txBuf[12] = tempData;
    txBuf[13] = pRectModConfig->rectFailDelayTime >> 8;
    txBuf[14] = pRectModConfig->rectFailDelayTime;
    tempData = pRectModConfig->rectRatedPower * 10;
    txBuf[15] = tempData >> 8;
    txBuf[16] = tempData;
    txBuf[17] = pRectModConfig->rectCommBreakTime >> 8;
    txBuf[18] = pRectModConfig->rectCommBreakTime;
    tempData = pRectModConfig->rectOutputOverVolt;
    txBuf[19] = tempData >> 8;
    txBuf[20] = tempData;
    tempData = pRectModConfig->rectOutputMaxVolt;
    txBuf[21] = tempData >> 8;
    txBuf[22] = tempData;
    tempData = pRectModConfig->rectOutputMinVolt;
    txBuf[23] = tempData >> 8;
    txBuf[24] = tempData;
    txBuf[25] = pRectModConfig->rectMaxLimitCurr >> 8;
    txBuf[26] = pRectModConfig->rectMaxLimitCurr;
    txBuf[27] = pRectModConfig->rectMinLimitCurr >> 8;
    txBuf[28] = pRectModConfig->rectMinLimitCurr;

    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 29);
    txBuf[29] = (U8_T)u16_crc;
    txBuf[30] = (U8_T)(u16_crc >> 8);

    U32_T ret = com_send_data(g_ccu_com_fd, txBuf, 31);
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    if (ret > 0)
    {
        CCU_PrintHexData("发送整流模块设置", txBuf, 31);
    }
    return ret;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-28
 * 函 数 名：ET_CCU_SetRectModConfig
 * 描    述: 电表设置
 *
 * 参    数: data - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_SetMeterConfig(pstCFG_METER_INFO_T pMeterConfig)
{
    if (pMeterConfig == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    if (g_ccu_com_fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U16_T tempData;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;       // 从机地址
    txBuf[1] = MASK_WRITE_REGISTER; // 操作方式
    txBuf[2] = METER_CONFIG_START_ADDR >> 8;
    txBuf[3] = METER_CONFIG_START_ADDR;
    txBuf[4] = METER_CONFIG_DATA_LEN >> 8;
    txBuf[5] = METER_CONFIG_DATA_LEN;
    txBuf[6] = METER_CONFIG_DATA_LEN * 2;

    txBuf[7] = pMeterConfig->acMeterBaud >> 8;
    txBuf[8] = pMeterConfig->acMeterBaud;
    txBuf[9] = pMeterConfig->acMeter1Addr >> 56;
    txBuf[10] = pMeterConfig->acMeter1Addr >> 48;
    txBuf[11] = pMeterConfig->acMeter1Addr >> 40;
    txBuf[12] = pMeterConfig->acMeter1Addr >> 32;
    txBuf[13] = pMeterConfig->acMeter1Addr >> 24;
    txBuf[14] = pMeterConfig->acMeter1Addr >> 16;
    txBuf[15] = pMeterConfig->acMeter1Addr >> 8;
    txBuf[16] = pMeterConfig->acMeter1Addr;

    txBuf[17] = pMeterConfig->acMeter2Addr >> 56;
    txBuf[18] = pMeterConfig->acMeter2Addr >> 48;
    txBuf[19] = pMeterConfig->acMeter2Addr >> 40;
    txBuf[20] = pMeterConfig->acMeter2Addr >> 32;
    txBuf[21] = pMeterConfig->acMeter2Addr >> 24;
    txBuf[22] = pMeterConfig->acMeter2Addr >> 16;
    txBuf[23] = pMeterConfig->acMeter2Addr >> 8;
    txBuf[24] = pMeterConfig->acMeter2Addr;

    txBuf[25] = pMeterConfig->acMeterCoe >> 8;
    txBuf[26] = pMeterConfig->acMeterCoe;

    txBuf[27] = pMeterConfig->acMeterBaud >> 8;
    txBuf[28] = pMeterConfig->acMeterBaud;
    txBuf[29] = pMeterConfig->acMeter1Addr >> 56;
    txBuf[30] = pMeterConfig->acMeter1Addr >> 48;
    txBuf[31] = pMeterConfig->acMeter1Addr >> 40;
    txBuf[32] = pMeterConfig->acMeter1Addr >> 32;
    txBuf[33] = pMeterConfig->acMeter1Addr >> 24;
    txBuf[34] = pMeterConfig->acMeter1Addr >> 16;
    txBuf[35] = pMeterConfig->acMeter1Addr >> 8;
    txBuf[36] = pMeterConfig->acMeter1Addr;

    txBuf[37] = pMeterConfig->acMeter2Addr >> 56;
    txBuf[38] = pMeterConfig->acMeter2Addr >> 48;
    txBuf[39] = pMeterConfig->acMeter2Addr >> 40;
    txBuf[40] = pMeterConfig->acMeter2Addr >> 32;
    txBuf[41] = pMeterConfig->acMeter2Addr >> 24;
    txBuf[42] = pMeterConfig->acMeter2Addr >> 16;
    txBuf[43] = pMeterConfig->acMeter2Addr >> 8;
    txBuf[44] = pMeterConfig->acMeter2Addr;

    txBuf[45] = pMeterConfig->acMeterCoe >> 8;
    txBuf[46] = pMeterConfig->acMeterCoe;

    txBuf[45] = pMeterConfig->commInterrNum >> 8;
    txBuf[46] = pMeterConfig->commInterrNum;

    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 47);
    txBuf[47] = (U8_T)u16_crc;
    txBuf[48] = (U8_T)(u16_crc >> 8);

    U32_T ret = com_send_data(g_ccu_com_fd, txBuf, 49);
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    if (ret > 0)
    {
        CCU_PrintHexData("发送电表设置", txBuf, 49);
    }
    return ret;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-04-28
 * 函 数 名：CCU_ChargeTypeConfig
 * 描    述: 设置充电类型配置
 *
 * 参    数: data - [参数说明]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static U32_T CCU_SendChargeCmd(pstCMD_CHARGE_INFO_T pChrCmd, U32_T startAddr, U16_T dataLen)
{
    if (pChrCmd == NULL)
    {
        return -1;
    }

    pthread_mutex_lock(&g_ccuLock);
    if (g_ccu_com_fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U16_T tempData;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;       // 从机地址
    txBuf[1] = MASK_WRITE_REGISTER; // 操作方式
    txBuf[2] = startAddr >> 8;
    txBuf[3] = startAddr;
    txBuf[4] = dataLen >> 8;
    txBuf[5] = dataLen;
    txBuf[6] = dataLen * 2;
    txBuf[7] = pChrCmd->assignPower >> 8;
    txBuf[8] = pChrCmd->assignPower;
    txBuf[9] = pChrCmd->dualGunCharge >> 8;
    txBuf[10] = pChrCmd->dualGunCharge;
    txBuf[11] = pChrCmd->ctrCmd >> 8;
    txBuf[12] = pChrCmd->ctrCmd;
    txBuf[13] = pChrCmd->chargeTypeCtr >> 8;
    txBuf[14] = pChrCmd->chargeTypeCtr;
    tempData = pChrCmd->powerCtr * 10;
    txBuf[15] = tempData >> 8;
    txBuf[16] = tempData;

    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 17);
    txBuf[17] = (U8_T)u16_crc;
    txBuf[18] = (U8_T)(u16_crc >> 8);

    U32_T ret = com_send_data(g_ccu_com_fd, txBuf, 19);
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    if (ret > 0)
    {
        CCU_PrintHexData("发送充电命令", txBuf, 19);
    }
    return ret;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-05-21
 * 函 数 名：ET_CCU_SendChargeCmd
 * 描    述: 发送充电命令
 *
 * 参    数: pChrCmd - [参数说明]
 * 参    数: gunID - 0 -- 1枪，1 -- 2枪
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_SendChargeCmd(pstCMD_CHARGE_INFO_T pChrCmd, U32_T gunID)
{
    if (gunID == 0)
    {
        return CCU_SendChargeCmd(pChrCmd, GUN1_CHARGE_CMD_START_ADDR, GUN_CHARGE_CMD_DATA_LEN);
    }
    else
    {
        return CCU_SendChargeCmd(pChrCmd, GUN2_CHARGE_CMD_START_ADDR, GUN_CHARGE_CMD_DATA_LEN);
    }
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-05-06
 * 函 数 名：ET_CCU_SetCcuConfig
 * 描    述: 发送CCU配置信息
 *
 * 参    数: data - [ccu配置结构体]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
U32_T ET_CCU_SetCcuConfig(pstCFG_CCU_INFO_T pConfig)
{
    pthread_mutex_lock(&g_ccuLock);
    if (g_ccu_com_fd == -1)
    {
        pthread_mutex_unlock(&g_ccuLock); // 解锁
        return -1;
    }

    U16_T u16_crc;
    U16_T tempData;
    U8_T txBuf[CCU_COM_TX_BUF_SIZE];
    memset(txBuf, 0, CCU_COM_TX_BUF_SIZE);

    txBuf[0] = SLAVE_ADDRESS;       // 从机地址
    txBuf[1] = MASK_WRITE_REGISTER; // 操作方式
    txBuf[2] = CCU_CONFIG_START_ADDR >> 8;
    txBuf[3] = CCU_CONFIG_START_ADDR;
    txBuf[4] = CCU_CONFIG_DATA_LEN >> 8;
    txBuf[5] = CCU_CONFIG_DATA_LEN;
    txBuf[6] = CCU_CONFIG_DATA_LEN * 2;

    txBuf[7] = pConfig->ctrlConfig.proVer >> 8;
    txBuf[8] = pConfig->ctrlConfig.proVer;
    txBuf[9] = pConfig->ctrlConfig.ctrlDebug >> 8;
    txBuf[10] = pConfig->ctrlConfig.ctrlDebug;
    txBuf[11] = pConfig->ctrlConfig.chrBusNum >> 8;
    txBuf[12] = pConfig->ctrlConfig.chrBusNum;
    txBuf[13] = pConfig->ctrlConfig.haveParCont >> 8;
    txBuf[14] = pConfig->ctrlConfig.haveParCont;
    txBuf[15] = pConfig->ctrlConfig.haveAcCont >> 8;
    txBuf[16] = pConfig->ctrlConfig.haveAcCont;
    txBuf[17] = pConfig->ctrlConfig.acContStat >> 8;
    txBuf[18] = pConfig->ctrlConfig.acContStat;
    txBuf[19] = pConfig->ctrlConfig.dcContStat >> 8;
    txBuf[20] = pConfig->ctrlConfig.dcContStat;
    txBuf[21] = pConfig->ctrlConfig.parContStat >> 8;
    txBuf[22] = pConfig->ctrlConfig.parContStat;
    txBuf[23] = pConfig->ctrlConfig.standbyLockGun >> 8;
    txBuf[24] = pConfig->ctrlConfig.standbyLockGun;
    txBuf[25] = pConfig->ctrlConfig.gunResDetect >> 8;
    txBuf[26] = pConfig->ctrlConfig.gunResDetect;
    txBuf[27] = pConfig->ctrlConfig.disChrFault >> 8;
    txBuf[28] = pConfig->ctrlConfig.disChrFault;
    txBuf[29] = pConfig->ctrlConfig.outPutUnctrl >> 8;
    txBuf[30] = pConfig->ctrlConfig.outPutUnctrl;
    txBuf[31] = pConfig->ctrlConfig.coolSysType >> 8;
    txBuf[32] = pConfig->ctrlConfig.coolSysType;
    txBuf[33] = pConfig->ctrlConfig.coolSysActivate >> 8;
    txBuf[34] = pConfig->ctrlConfig.coolSysActivate;
    txBuf[35] = pConfig->ctrlConfig.gunLockStatType >> 8;
    txBuf[36] = pConfig->ctrlConfig.gunLockStatType;
    txBuf[37] = pConfig->ctrlConfig.gunElecLockCtrl >> 8;
    txBuf[38] = pConfig->ctrlConfig.gunElecLockCtrl;

    txBuf[49] = pConfig->chrConfig.insBridDelay >> 8;
    txBuf[50] = pConfig->chrConfig.insBridDelay;
    txBuf[51] = pConfig->chrConfig.fanStartTemp >> 8;
    txBuf[52] = pConfig->chrConfig.fanStartTemp;
    txBuf[53] = pConfig->chrConfig.fanStopTemp >> 8;
    txBuf[54] = pConfig->chrConfig.fanStopTemp;
    txBuf[55] = pConfig->chrConfig.chrOverTempThr >> 8;
    txBuf[56] = pConfig->chrConfig.chrOverTempThr;
    txBuf[57] = pConfig->chrConfig.gunCurrDownThr >> 8;
    txBuf[58] = pConfig->chrConfig.gunCurrDownThr;
    txBuf[59] = pConfig->chrConfig.gunShutDownThr >> 8;
    txBuf[60] = pConfig->chrConfig.gunShutDownThr;
    tempData = pConfig->chrConfig.bmsTimeoutCof * 10;
    txBuf[61] = tempData >> 8;
    txBuf[62] = tempData;
    txBuf[63] = pConfig->chrConfig.contAdhesionThr >> 8;
    txBuf[64] = pConfig->chrConfig.contAdhesionThr;
    tempData = pConfig->chrConfig.gunMaxCurr;
    txBuf[65] = tempData >> 8;
    txBuf[66] = tempData;
    tempData = pConfig->chrConfig.dcBusVoltA;
    txBuf[67] = tempData >> 8;
    txBuf[68] = tempData;
    tempData = pConfig->chrConfig.dcBusVoltB;
    txBuf[69] = tempData >> 8;
    txBuf[70] = tempData;
    txBuf[81] = pConfig->rectConfig.rectNum >> 8;
    txBuf[82] = pConfig->rectConfig.rectNum;
    txBuf[83] = pConfig->rectConfig.rectPro >> 8;
    txBuf[84] = pConfig->rectConfig.rectPro;
    tempData = pConfig->rectConfig.rectRatedCurr;
    txBuf[85] = tempData >> 8;
    txBuf[86] = tempData;
    txBuf[87] = pConfig->rectConfig.rectFailDelayTime >> 8;
    txBuf[88] = pConfig->rectConfig.rectFailDelayTime;
    tempData = pConfig->rectConfig.rectRatedPower * 10;
    txBuf[89] = tempData >> 8;
    txBuf[90] = tempData;
    txBuf[91] = pConfig->rectConfig.rectCommBreakTime >> 8;
    txBuf[92] = pConfig->rectConfig.rectCommBreakTime;
    tempData = pConfig->rectConfig.rectOutputOverVolt;
    txBuf[93] = tempData >> 8;
    txBuf[94] = tempData;
    tempData = pConfig->rectConfig.rectOutputMaxVolt;
    txBuf[95] = tempData >> 8;
    txBuf[96] = tempData;
    tempData = pConfig->rectConfig.rectOutputMinVolt;
    txBuf[97] = tempData >> 8;
    txBuf[98] = tempData;
    txBuf[99] = pConfig->rectConfig.rectMaxLimitCurr >> 8;
    txBuf[100] = pConfig->rectConfig.rectMaxLimitCurr;
    txBuf[101] = pConfig->rectConfig.rectMinLimitCurr >> 8;
    txBuf[102] = pConfig->rectConfig.rectMinLimitCurr;
    txBuf[109] = pConfig->meterConfig.acMeterBaud >> 8;
    txBuf[110] = pConfig->meterConfig.acMeterBaud;
    txBuf[111] = pConfig->meterConfig.acMeter1Addr >> 56;
    txBuf[112] = pConfig->meterConfig.acMeter1Addr >> 48;
    txBuf[113] = pConfig->meterConfig.acMeter1Addr >> 40;
    txBuf[114] = pConfig->meterConfig.acMeter1Addr >> 32;
    txBuf[115] = pConfig->meterConfig.acMeter1Addr >> 24;
    txBuf[116] = pConfig->meterConfig.acMeter1Addr >> 16;
    txBuf[117] = pConfig->meterConfig.acMeter1Addr >> 8;
    txBuf[118] = pConfig->meterConfig.acMeter1Addr;
    txBuf[119] = pConfig->meterConfig.acMeter2Addr >> 56;
    txBuf[120] = pConfig->meterConfig.acMeter2Addr >> 48;
    txBuf[121] = pConfig->meterConfig.acMeter2Addr >> 40;
    txBuf[122] = pConfig->meterConfig.acMeter2Addr >> 32;
    txBuf[123] = pConfig->meterConfig.acMeter2Addr >> 24;
    txBuf[124] = pConfig->meterConfig.acMeter2Addr >> 16;
    txBuf[125] = pConfig->meterConfig.acMeter2Addr >> 8;
    txBuf[126] = pConfig->meterConfig.acMeter2Addr;
    txBuf[127] = pConfig->meterConfig.acMeterCoe >> 8;
    txBuf[128] = pConfig->meterConfig.acMeterCoe;
    txBuf[129] = pConfig->meterConfig.acMeterBaud >> 8;
    txBuf[130] = pConfig->meterConfig.acMeterBaud;
    txBuf[131] = pConfig->meterConfig.acMeter1Addr >> 56;
    txBuf[132] = pConfig->meterConfig.acMeter1Addr >> 48;
    txBuf[133] = pConfig->meterConfig.acMeter1Addr >> 40;
    txBuf[134] = pConfig->meterConfig.acMeter1Addr >> 32;
    txBuf[135] = pConfig->meterConfig.acMeter1Addr >> 24;
    txBuf[136] = pConfig->meterConfig.acMeter1Addr >> 16;
    txBuf[137] = pConfig->meterConfig.acMeter1Addr >> 8;
    txBuf[138] = pConfig->meterConfig.acMeter1Addr;
    txBuf[139] = pConfig->meterConfig.acMeter2Addr >> 56;
    txBuf[140] = pConfig->meterConfig.acMeter2Addr >> 48;
    txBuf[141] = pConfig->meterConfig.acMeter2Addr >> 40;
    txBuf[142] = pConfig->meterConfig.acMeter2Addr >> 32;
    txBuf[143] = pConfig->meterConfig.acMeter2Addr >> 24;
    txBuf[144] = pConfig->meterConfig.acMeter2Addr >> 16;
    txBuf[145] = pConfig->meterConfig.acMeter2Addr >> 8;
    txBuf[146] = pConfig->meterConfig.acMeter2Addr;
    txBuf[147] = pConfig->meterConfig.acMeterCoe >> 8;
    txBuf[148] = pConfig->meterConfig.acMeterCoe;
    txBuf[149] = pConfig->meterConfig.commInterrNum >> 8;
    txBuf[150] = pConfig->meterConfig.commInterrNum;

    u16_crc = CCU_CalculateCrc(&(txBuf[0]), 151);
    txBuf[151] = (U8_T)u16_crc;
    txBuf[152] = (U8_T)(u16_crc >> 8);
    U32_T ret = com_send_data(g_ccu_com_fd, txBuf, 153);
    pthread_mutex_unlock(&g_ccuLock); // 解锁
    if (ret > 0)
    {
        CCU_PrintHexData("发送CCU配置信息", txBuf, 153);
    }
    return ret;
}