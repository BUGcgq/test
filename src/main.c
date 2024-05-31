#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "APIS_CcuComm.h"
#include "ToolFunc.h"

// 枚举类型定义
typedef enum
{
    STATE_SEND_SYS_INFO,
    STATE_SEND_GUN1,
    STATE_SEND_GUN2,
    STATE_SEND_RECT_MODULE,
    STATE_MAX // 用于跟踪状态的最大值
} SendState;
// 记录上次发送的时间
U32_T lastSendTime = 0;
static bool waitingForResponse = false;
static U8_T g_SnedMsgCnt = 0;
static U8_T g_ChargeState[2] = {0};
static U8_T g_rectNum = 2;

static void CCU_sendTelemetryStateMachine()
{
    static SendState currentState = STATE_SEND_SYS_INFO;

    switch (currentState)
    {
    case STATE_SEND_SYS_INFO:
        ET_CCU_TelemetrySysInfo();
        break;

    case STATE_SEND_GUN1:

        ET_CCU_TelemetryChargeGunData(0);

        break;

    case STATE_SEND_GUN2:

        ET_CCU_TelemetryChargeGunData(1);

        break;

    case STATE_SEND_RECT_MODULE:
        ET_CCU_TelemetryRectModuleData();
        break;

    default:
        break;
    }

    // 更新状态
    currentState = (SendState)((currentState + 1) % STATE_MAX);
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
    printf("配置请求成功\n");
}

int RecvConfigReqHandler()
{
    CFG_CCU_INFO_T Config;
    memset(&Config, 0, sizeof(CFG_CCU_INFO_T));

    Config.ctrlConfig.proVer = 0x10;
    Config.ctrlConfig.ctrlDebug = 0;
    Config.ctrlConfig.chrBusNum = 2;
    Config.ctrlConfig.haveParCont = 1;
    Config.ctrlConfig.haveAcCont = 1;
    Config.ctrlConfig.acContStat = 0;
    Config.ctrlConfig.dcContStat = 0;
    Config.ctrlConfig.parContStat = 0;
    Config.ctrlConfig.standbyLockGun = 1;
    Config.ctrlConfig.gunResDetect = 1;
    Config.ctrlConfig.disChrFault = 1;
    Config.ctrlConfig.outPutUnctrl = 1;
    Config.ctrlConfig.coolSysType = 0;
    Config.ctrlConfig.coolSysActivate = 0;
    Config.ctrlConfig.gunLockStatType = 1;
    Config.ctrlConfig.gunElecLockCtrl = 0;

    // 充电桩配置
    Config.chrConfig.insBridDelay = 2;
    Config.chrConfig.fanStartTemp = 50;
    Config.chrConfig.fanStopTemp = 40;
    Config.chrConfig.chrOverTempThr = 80;
    Config.chrConfig.gunCurrDownThr = 120;
    Config.chrConfig.gunShutDownThr = 70;
    Config.chrConfig.bmsTimeoutCof = 10;
    Config.chrConfig.contAdhesionThr = 100;
    Config.chrConfig.gunMaxCurr = 250.0;
    Config.chrConfig.dcBusVoltA = 500.0;
    Config.chrConfig.dcBusVoltB = 600.0;

    // 整流模块配置
    Config.rectConfig.rectNum = g_rectNum;
    Config.rectConfig.rectPro = 0;
    Config.rectConfig.rectRatedCurr = 100.0;
    Config.rectConfig.rectFailDelayTime = 5;
    Config.rectConfig.rectRatedPower = 10.0;
    Config.rectConfig.rectCommBreakTime = 10;
    Config.rectConfig.rectOutputOverVolt = 300.0;
    Config.rectConfig.rectOutputMaxVolt = 100.0;
    Config.rectConfig.rectOutputMinVolt = 20.0;
    Config.rectConfig.rectMaxLimitCurr = 100;
    Config.rectConfig.rectMinLimitCurr = 3;

    // 电表
    Config.meterConfig.acMeterBaud = 2400;
    Config.meterConfig.acMeter1Addr = 0;
    Config.meterConfig.acMeter2Addr = 0;
    Config.meterConfig.acMeterCoe = 40;
    Config.meterConfig.dcMeterBaud = 2400;
    Config.meterConfig.dcMeter1Addr = 0;
    Config.meterConfig.dcMeter2Addr = 0;
    Config.meterConfig.dcMeterCoe = 40;
    Config.meterConfig.commInterrNum = 5;
    ET_CCU_SetCcuConfig(&Config);
    // 返回成功
    return 0;
}

int ChargeStateHandler(U16_T gunID, U16_T chargeState)
{
    g_ChargeState[gunID] = chargeState;
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
    ET_CCU_RegisterCallback(CCU_RECV_CONFIG_FINISH, RecvConfigFinishHandler);
    ET_CCU_RegisterCallback(CCU_RECV_CONFIG_REQ, RecvConfigReqHandler);
    ET_CCU_RegisterCallback(CCU_RECV_CHARGE_STATE, ChargeStateHandler);

    ET_CCU_CommInit(&serPort);
    ET_CCU_SetRectNum(g_rectNum);
    U32_T sendTime = 0;
    U32_T recvTime = 0;

    int i;

    DATA_SYS_INFO_T SysData;
    DATA_CHARGER_INFO_T GunData[2];
    DATA_RECT_INFO_T RectData[6];
    memset(&SysData, 0, sizeof(DATA_SYS_INFO_T));
    memset(GunData, 0, sizeof(GunData));
    memset(RectData, 0, sizeof(RectData));

    CMD_CHARGE_INFO_T chgCmd;
    memset(&chgCmd, 0, sizeof(CMD_CHARGE_INFO_T));

    chgCmd.ctrCmd = 1;
    ET_CCU_SendChargeCmd(&chgCmd, 0);
    
    while (1)
    {
        CCU_sendTelemetryStateMachine();
        usleep(130 * 1000);
        ET_CCU_UsartRecvData();
        if (u32_inc_get_diff_time_ms(&recvTime) >= 2000)
        {

            ET_CCU_GetSysInfoData(&SysData);
            printf("系统数据:\n");
            printf("  充电桩内部通信协议版本: %X\n", SysData.version);
            printf("  CCU监控软件主版本号: %X\n", SysData.ccuSwVer);
            printf("  CCU监控硬件主版本号: %X\n", SysData.tcuHwVer);
            printf("  充电桩环境温度: %.2f°C\n", SysData.caseTemp);
            printf("  CCU控制器DI状态: %X\n", SysData.diStatus);

            for (i = 0; i < 2; i++)
            {
                ET_CCU_GetChargeData(i, &GunData[i]);
                printf("  枪%d数据 :\n", i + 1);
                printf("  充电机状态: %u\n", GunData[i].gunData.chgState);
                printf("  充电枪连接状态: %u\n", GunData[i].gunData.gunConnState);
                printf("  充电枪锁状态: %u\n", GunData[i].gunData.gunLockState);
                printf("  充电枪归位状态: %u\n", GunData[i].gunData.gunResState);
                printf("  CCS1电压: %.2f\n", GunData[i].gunData.ccs1Volt);
                printf("  枪温度1: %.2f\n", GunData[i].gunData.gunTemp1);
                printf("  枪温度2: %.2f\n", GunData[i].gunData.gunTemp2);
                printf("  枪状态STATE1: %u\n", GunData[i].gunData.gunState1);
                printf("  枪状态STATE2: %u\n", GunData[i].gunData.gunState2);
                printf("  枪状态STATE3: %u\n", GunData[i].gunData.gunState3);
                printf("  枪最大功率: %.2f\n", GunData[i].gunData.gunMaxRate);
                printf("  枪最大输出电流: %.2f\n", GunData[i].gunData.gunMaxCurr);
                printf("  枪启动失败原因: %u\n", GunData[i].gunData.gunStarFailReson);
                printf("  枪停机原因: %llu\n", GunData[i].gunData.gunstopReson);
                printf("  电表测量电压值: %.2f\n", GunData[i].gunData.meterVolt);
                printf("  电表测量电流值: %.2f\n", GunData[i].gunData.meterCurr);
                printf("  电表当前读数: %.2f\n", GunData[i].gunData.meterRead);
                printf("  枪母线正对地电压: %.2f\n", GunData[i].gunData.busPosToGroundVolt);
                printf("  枪母线负对地电压: %.2f\n", GunData[i].gunData.busNegToGroundVolt);
                printf("  枪母线正对地电阻: %.2f\n", GunData[i].gunData.busPosToGroundRes);
                printf("  枪母线负对地电阻: %.2f\n", GunData[i].gunData.busNegToGroundRes);
                printf("  母线输出电压A侧: %.2f\n", GunData[i].gunData.busOutVoltSideA);
                printf("  母线输出电流A侧: %.2f\n", GunData[i].gunData.busOutCuSideA);
                printf("  母线输出电压B侧: %.2f\n", GunData[i].gunData.busOutVoltSideB);
                printf("  母线输出电流B侧: %.2f\n", GunData[i].gunData.busOutCuSideB);

                printf("  单体动力蓄电池最高允许充电电压: %.2f\n", GunData[i].bmsData.cellmaxlAlwChgVolt);
                printf("  最高允许充电电流: %.2f\n", GunData[i].bmsData.maxAlwChgCurr);
                printf("  动力蓄电池标称总能量: %.2f\n", GunData[i].bmsData.nominalTotalElect);
                printf("  最高允许充电总电压: %.2f\n", GunData[i].bmsData.maxAlwChgVolt);
                printf("  最高允许动力蓄电池温度: %d\n", GunData[i].bmsData.maxAlwTemp);
                printf("  整车动力蓄电池荷电状态: %u\n", GunData[i].bmsData.batEleSOC);
                printf("  接收bms报文标志: %u\n", GunData[i].bmsData.bmsFlag);
                printf("  电压需求: %.2f\n", GunData[i].bmsData.batNeedVolt);
                printf("  电流需求: %.2f\n", GunData[i].bmsData.battNeedCurr);
                printf("  充电模式: %u\n", GunData[i].bmsData.chgMode);
                printf("  充电电压测量值: %.2f\n", GunData[i].bmsData.chgVolt);
                printf("  充电电流测量值: %.2f\n", GunData[i].bmsData.chgCurr);
                printf("  最高单体动力蓄电池电压: %.2f\n", GunData[i].bmsData.cellMaxChgVolt);
                printf("  最高单体动力蓄电池电压所在组号: %d\n", GunData[i].bmsData.cellMaxVoltGroupNum);
                printf("  Battery SOC: %u\n", GunData[i].bmsData.batSoc);
                printf("  估算剩余充电时间: %u\n", GunData[i].bmsData.remainChgTime);
                printf("  最高单体动力蓄电池电压所在编号: %u\n", GunData[i].bmsData.cellMaxVoltID);
                printf("  最高动力蓄电池温度: %d\n", GunData[i].bmsData.batMaxTemp);
                printf("  最高温度检测点编号: %u\n", GunData[i].bmsData.maxTempID);
                printf("  最低动力蓄电池温度: %d\n", GunData[i].bmsData.batMinTemp);
                printf("  最低温度检测点编号: %u\n", GunData[i].bmsData.minTempID);
                printf("  电池状态: %u\n", GunData[i].bmsData.batState);
                printf("  Battery End SOC: %u\n", GunData[i].bmsData.batEndSoc);
                printf("  动力蓄电池单体最低电压: %.2f\n", GunData[i].bmsData.cellEndMinVolt);
                printf("  动力蓄电池单体最高电压: %.2f\n", GunData[i].bmsData.cellEndMaxVolt);
                printf("  动力蓄电池最低温度: %d\n", GunData[i].bmsData.batEndMinTemp);
                printf("  动力蓄电池最高温度: %d\n", GunData[i].bmsData.batEndMaxTemp);
                printf("  vim: %s\n", GunData[i].bmsData.vin);
            }

            for (i = 0; i < g_rectNum; i++)
            {
                ET_CCU_GetRectModuleData(i, &RectData[i]);
                printf(" 模块%d数据\n", i + 1);
                printf("    通信状态: %u\n", RectData[i].commState);
                printf("    模块状态: %u\n", RectData[i].rectState);
                printf("    模块输出电压: %.1f V\n", RectData[i].rectOutVolt);
                printf("    模块输出电流: %.1f A\n", RectData[i].rectOutCurr);
                printf("    限流点: %.1f\n", RectData[i].rectLimitPoint);
                printf("    温度: %.1f °C\n", RectData[i].rectTemp);
                printf("    输出功率: %.1f W\n", RectData[i].rectOutPower);
            }

            recvTime = u32_inc_get_system_time_ms();
        }
    }
}
