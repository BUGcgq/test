#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "APIS_CcuComm.h"
#include "ToolFunc.h"

typedef enum
{
    STATE_SEND_SYS_INFO,
    STATE_SEND_GUN1,
    STATE_SEND_GUN2,
    STATE_SEND_RECT_MODULE
} SendState;

// 记录上次发送的时间
U32_T lastSendTime = 0;
static bool waitingForResponse = false;
static U8_T g_SnedMsgCnt = 0;
static U8_T g_ChargeState[2] = {0};

static void CCU_sendTelemetryStateMachine()
{
    static SendState currentState = STATE_SEND_SYS_INFO;
    switch (currentState)
    {

    case STATE_SEND_SYS_INFO:
        ET_CCU_TelemetrySysInfo();
        currentState = STATE_SEND_GUN1; // 切换回第一个状态
        break;

    case STATE_SEND_GUN1:
        if (g_ChargeState[0] == 1)
        {
            ET_CCU_TelemetryChargeGunData(0);
        }
        else
        {
            ET_CCU_TelemetryFreeGunData(0);
        }
        currentState = STATE_SEND_GUN2; // 切换到下一个状态
        break;

    case STATE_SEND_GUN2:
        if (g_ChargeState[1] == 1)
        {
            ET_CCU_TelemetryChargeGunData(1);
        }
        else
        {
            ET_CCU_TelemetryFreeGunData(1);
        }
        currentState = STATE_SEND_RECT_MODULE; // 切换到下一个状态
        break;

    case STATE_SEND_RECT_MODULE:
        ET_CCU_TelemetryRectModuleData();
        currentState = STATE_SEND_SYS_INFO; // 切换到下一个状态
        break;

    default:
        currentState = STATE_SEND_SYS_INFO; // 默认切换回第一个状态
        break;
    }
}

/* 实现回调函数 */
int RecvErrorHandler(U8_T error_code, U8_T error_type)
{
    printf("Received error: Error Code - %d, Error Type - %d\n", error_code, error_type);
    // 返回成功
    return 0;
}

/* 实现回调函数 */
int RecvMessageHandler(U16_T funcAddr)
{
    g_SnedMsgCnt = 0;
    // 返回成功
    return 0;
}

int RecvConfigFinishHandler()
{
}

int RecvConfigReqHandler()
{
    CFG_CCU_INFO_T Config;
    memset(&Config, 0, sizeof(CFG_CCU_INFO_T));

    Config.ctrlConfig.proVer = 1001;
    Config.ctrlConfig.ctrlDebug = 1;
    Config.ctrlConfig.chrBusNum = 2;
    Config.ctrlConfig.haveParCont = 1;
    Config.ctrlConfig.haveAcCont = 1;
    Config.ctrlConfig.acContStat = 0;
    Config.ctrlConfig.dcContStat = 0;
    Config.ctrlConfig.parContStat = 0;
    Config.ctrlConfig.standbyLockGun = 1;
    Config.ctrlConfig.gunResDetect = 1;
    Config.ctrlConfig.disChrFault = 0;
    Config.ctrlConfig.outPutUnctrl = 0;
    Config.ctrlConfig.coolSysType = 1;
    Config.ctrlConfig.coolSysActivate = 1;
    Config.ctrlConfig.gunLockStatType = 1;
    Config.ctrlConfig.gunElecLockCtrl = 1;

    // 充电桩配置
    Config.chrConfig.insBridDelay = 10;
    Config.chrConfig.fanStartTemp = 30;
    Config.chrConfig.fanStopTemp = 25;
    Config.chrConfig.chrOverTempThr = 60;
    Config.chrConfig.gunCurrDownThr = 50;
    Config.chrConfig.gunShutDownThr = 70;
    Config.chrConfig.bmsTimeoutCof = 5;
    Config.chrConfig.contAdhesionThr = 3;
    Config.chrConfig.gunMaxCurr = 100.0;
    Config.chrConfig.dcBusVoltA = 500.0;
    Config.chrConfig.dcBusVoltB = 600.0;

    // 整流模块配置
    Config.rectConfig.rectNum = 2;
    Config.rectConfig.rectPro = 1234;
    Config.rectConfig.rectRatedCurr = 100.0;
    Config.rectConfig.rectFailDelayTime = 500;
    Config.rectConfig.rectRatedPower = 10.0;
    Config.rectConfig.rectCommBreakTime = 1000;
    Config.rectConfig.rectOutputOverVolt = 300.0;
    Config.rectConfig.rectOutputMaxVolt = 250.0;
    Config.rectConfig.rectOutputMinVolt = 200.0;
    Config.rectConfig.rectMaxLimitCurr = 20;
    Config.rectConfig.rectMinLimitCurr = 5;

    ET_CCU_SetCcuConfig(&Config);
    // 返回成功
    return 0;
}

int ChargeStateHandler(U16_T gunID, U16_T chargeState)
{
    g_ChargeState[gunID] = chargeState;
    return 0;
}

// // 打印 DATA_GUN_INFO_T 的数据
// void printGunInfo(const DATA_GUN_INFO_T *gunInfo)
// {
//     if (gunInfo == NULL)
//     {
//         printf("Invalid gunInfo pointer\n");
//         return;
//     }

//     printf("Gun Information:\n");
//     printf("  Charge State: %u\n", gunInfo->chgState);
//     printf("  Gun Connection State: %u\n", gunInfo->gunConnState);
//     printf("  Gun Lock State: %u\n", gunInfo->gunLockState);
//     printf("  Gun Reset State: %u\n", gunInfo->gunResState);
//     printf("  CCS1 Voltage: %u\n", gunInfo->ccs1Volt);
//     printf("  Gun Temperature 1: %.2f\n", gunInfo->gunTemp1);
//     printf("  Gun Temperature 2: %.2f\n", gunInfo->gunTemp2);
//     printf("  Gun State 1: %u\n", gunInfo->gunState1);
//     printf("  Gun State 2: %u\n", gunInfo->gunState2);
//     printf("  Gun State 3: %u\n", gunInfo->gunState3);
//     printf("  Gun Max Rate: %u\n", gunInfo->gunMaxRate);
//     printf("  Gun Max Current: %u\n", gunInfo->gunMaxCurr);
//     printf("  Gun Start Fail Reason: %u\n", gunInfo->gunStarFailReson);
//     printf("  Gun Stop Reason: %llu\n", gunInfo->gunstopReson);
//     printf("  Meter Voltage: %.2f\n", gunInfo->meterVolt);
//     printf("  Meter Current: %.2f\n", gunInfo->meterCurr);
//     printf("  Meter Read: %.2f\n", gunInfo->meterRead);
//     printf("  Bus Positive to Ground Voltage: %u\n", gunInfo->busPosToGroundVolt);
//     printf("  Bus Negative to Ground Voltage: %u\n", gunInfo->busNegToGroundVolt);
//     printf("  Bus Positive to Ground Resistance: %u\n", gunInfo->busPosToGroundRes);
//     printf("  Bus Negative to Ground Resistance: %u\n", gunInfo->busNegToGroundRes);
//     printf("  Bus Output Voltage Side A: %u\n", gunInfo->busOutVoltSideA);
//     printf("  Bus Output Current Side A: %u\n", gunInfo->busOutCuSideA);
//     printf("  Bus Output Voltage Side B: %u\n", gunInfo->busOutVoltSideB);
//     printf("  Bus Output Current Side B: %u\n", gunInfo->busOutCuSideB);
// }

// // 打印 DATA_BMS_INFO_T 的数据
// void printBmsInfo(const DATA_BMS_INFO_T *bmsInfo)
// {
//     if (bmsInfo == NULL)
//     {
//         printf("Invalid bmsInfo pointer\n");
//         return;
//     }

//     printf("BMS Information:\n");
//     printf("  Cell Max Allowed Charge Voltage: %.2f\n", bmsInfo->cellmaxlAlwChgVolt);
//     printf("  Max Allowed Charge Current: %.2f\n", bmsInfo->maxAlwChgCurr);
//     printf("  Nominal Total Energy: %.2f\n", bmsInfo->nominalTotalElect);
//     printf("  Max Allowed Charge Voltage: %.2f\n", bmsInfo->maxAlwChgVolt);
//     printf("  Max Allowed Temperature: %d\n", bmsInfo->maxAlwTemp);
//     printf("  Battery Electrical SOC: %u\n", bmsInfo->batEleSOC);
//     printf("  BMS Flag: %u\n", bmsInfo->bmsFlag);
//     printf("  Battery Need Voltage: %.2f\n", bmsInfo->batNeedVolt);
//     printf("  Battery Need Current: %.2f\n", bmsInfo->battNeedCurr);
//     printf("  Charge Mode: %u\n", bmsInfo->chgMode);
//     printf("  Charge Voltage: %.2f\n", bmsInfo->chgVolt);
//     printf("  Charge Current: %.2f\n", bmsInfo->chgCurr);
//     printf("  Cell Max Charge Voltage: %.2f\n", bmsInfo->cellMaxChgVolt);
//     printf("  Cell Max Voltage Group Number: %.2f\n", bmsInfo->cellMaxVoltGroupNum);
//     printf("  Battery SOC: %u\n", bmsInfo->batSoc);
//     printf("  Remaining Charge Time: %u\n", bmsInfo->remainChgTime);
//     printf("  Cell Max Voltage ID: %u\n", bmsInfo->cellMaxVoltID);
//     printf("  Battery Max Temperature: %d\n", bmsInfo->batMaxTemp);
//     printf("  Max Temperature ID: %u\n", bmsInfo->maxTempID);
//     printf("  Battery Min Temperature: %d\n", bmsInfo->batMinTemp);
//     printf("  Min Temperature ID: %u\n", bmsInfo->minTempID);
//     printf("  Battery State: %u\n", bmsInfo->batState);
//     printf("  Battery End SOC: %u\n", bmsInfo->batEndSoc);
//     printf("  Cell End Min Voltage: %.2f\n", bmsInfo->cellEndMinVolt);
//     printf("  Cell End Max Voltage: %.2f\n", bmsInfo->cellEndMaxVolt);
//     printf("  Battery End Min Temperature: %d\n", bmsInfo->batEndMinTemp);
//     printf("  Battery End Max Temperature: %d\n", bmsInfo->batEndMaxTemp);
//     printf("  VIN: %.17s\n", bmsInfo->vin); // Adjust format for VIN length
// }

// // 打印 DATA_CHARGER_INFO_T 数组中的数据
// void printChargerInfoArray(const DATA_CHARGER_INFO_T *chargerInfoArray, size_t size)
// {
//     if (chargerInfoArray == NULL)
//     {
//         printf("Invalid chargerInfoArray pointer\n");
//         return;
//     }

//     for (size_t i = 0; i < size; ++i)
//     {
//         printf("Charger Info #%zu:\n", i + 1);
//         printGunInfo(&chargerInfoArray[i].gunData);
//         printBmsInfo(&chargerInfoArray[i].bmsData);
//         printf("\n");
//     }
// }

static int CCU_DataSync()
{
    int i;
    DATA_SYS_INFO_T SysData;
    DATA_CHARGER_INFO_T GunData[2];
    DATA_RECT_INFO_T RectData[6];

    memset(&SysData, 0, sizeof(SysData));
    memset(GunData, 0, sizeof(GunData));
    memset(RectData, 0, sizeof(RectData));

    ET_CCU_GetSysInfoData(&SysData);

    for (i = 0; i < 2; i++)
    {
        ET_CCU_GetChargeData(i, &GunData[i]);
    }

    for (i = 0; i < 6; i++)
    {
        ET_CCU_GetRectModuleData(i, &RectData[i]);
    }

    return 0;
}

void main(void)
{
    CCU_CFG_T serPort;

    serPort.portPath = "/dev/ttyS6";
    serPort.comBaud = Baud_115200;
    serPort.parity = _NONE;

    ET_CCU_RegisterCallback(CCU_RECV_ERROR, RecvErrorHandler);
    ET_CCU_RegisterCallback(CCU_RECV_MESSAGE, RecvMessageHandler);
    ET_CCU_RegisterCallback(CCU_RECV_CONFIG_REQ, RecvConfigReqHandler);
    ET_CCU_RegisterCallback(CCU_RECV_CHARGE_STATE, ChargeStateHandler);

    ET_CCU_CommInit(&serPort);
    ET_CCU_SetRectNum(2);
    U32_T sendTime = 0;
    U32_T recvTime = 0;
    U32_T teleGun2DataTime = 0;
    U32_T teleRectModuleTime = 0;
    int i;
    while (1)
    {

        if (u32_inc_get_diff_time_ms(&sendTime) >= 2000)
        {
            // ET_CCU_TelemetrySysInfo();
            ET_CCU_TelemetryRectModuleData();
            sendTime = u32_inc_get_system_time_ms();
        }
        if (u32_inc_get_diff_time_ms(&recvTime) >= 2000)
        {
            DATA_RECT_INFO_T RectData[2];
            for (i = 0; i < 2; i++)
            {
                ET_CCU_GetRectModuleData(i, &RectData[i]);
                printf("Rectifier Information %d:\n", i + 1);
                printf(" Rectifier commState: %u\n", RectData[i].commState);
                printf("  Rectifier rectState: %u\n", RectData[i].rectState);
                printf("  Rectifier Output Voltage: %.2f V\n", RectData[i].rectOutVolt);
                printf("  Rectifier Output Current: %.2f A\n", RectData[i].rectOutCurr);
                printf("  Rectifier Limit Point: %.2f\n", RectData[i].rectLimitPoint);
                printf("  Rectifier Temperature: %.2f °C\n", RectData[i].rectTemp);
                printf("  Rectifier Output Power: %.2f W\n", RectData[i].rectOutPower);
            }

            recvTime = u32_inc_get_system_time_ms();
        }

        ET_CCU_UsartRecvData();

        usleep(50 * 1000); // 50000 微秒等于 50 毫秒
    }
}
