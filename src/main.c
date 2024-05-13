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
SendState currentState = STATE_SEND_SYS_INFO;
static bool waitingForResponse = false;
static U8_T g_SnedMsgCnt = 0;
static U8_T g_ChargeState[2] = {0};

void sendCommandsInSequence()
{
    U32_T currentTime = u32_inc_get_system_time_ms();

    if (u32_inc_get_diff_time_ms(&lastSendTime) >= 2000)
    {
        lastSendTime = currentTime;

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

        g_SnedMsgCnt++;
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
    ET_CCU_SetRectNum(6);
    U32_T teleSysInfoTime = 0;
    U32_T teleGun1DataTime = 0;
    U32_T teleGun2DataTime = 0;
    U32_T teleRectModuleTime = 0;
    while (1)
    {

        sendCommandsInSequence();

        ET_CCU_UsartRecvData();

        usleep(50 * 1000); // 50000 微秒等于 50 毫秒
    }
}
