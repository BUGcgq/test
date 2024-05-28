#ifndef __CCU_COMM_H_
#define __CCU_COMM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "Type.h"
#include "IncComType.h"

#define CCU_DEBUG_MSG                  1     //调试打印
#define CCU_COM_RX_BUF_SIZE            512   //缓冲区大小
#define CCU_COM_TX_BUF_SIZE            512    //缓冲区大小

// 读操作
#define READ_HOLDING_REGISTERS    0x03    // 读取寄存器
#define READ_INPUT_REGISTERS      0x04    // 遥测读取

// 写操作
#define WRITE_HOLDING_REGISTER    0x06    // 写寄存器
#define MASK_WRITE_REGISTER       0x10    // 写块寄存器
 // 诊断
#define DIAGNOSTICS               0x08    // 诊断
#define ERRORCODE                 0x80    // 错误码
#define SLAVE_ADDRESS   0x40   // 从机地址
#define MASTER_ADDRESS  0x01   // 主机地址

#define RECT_MODULE_MAX_NUM              60                       //模块最大数量
//------------充电机协议地址位定义--------------------
//读取遥测数据命令，发送读数据命令时只需要发送起始地址和长度即可
#define TELEMETRY_SYS_DATA_START_ADDR        0x0000              //充电桩系统数据起始地址
#define TELEMETRY_GUN1_DATA_START_ADDR       0x100               //遥测枪1数据起始地址
#define TELEMETRY_GUN2_DATA_START_ADDR       0x200               //遥测枪2数据起始地址
#define TELEMETRY_RECT_DATA_START_ADDR       0x300               //遥测模块数据起始地址


#define TELEMETRY_SYS_DATA_LEN               0X07                //充电桩系统数据长度
#define TELEMETRY_IDLE_DATA_LEN              0x20                //遥测空闲数据长度
#define TELEMETRY_CHARGE_DATA_LEN            0x54                //遥测充电中数据长度
#define TELEMETRY_RECT_DATA_LEN              0x0A                //一个模块数据长度
//设置配置
#define CONTROLLER_CONFIG_START_ADDR         0x1000               //控制器配置数据起始地址
#define CONTROLLER_CONFIG_DATA_LEN           0X10                 //控制器配置数据长度

#define CHARGER_CONFIG_START_ADDR            0x1015               //充电桩配置数据起始地址
#define CHARGER_CONFIG_DATA_LEN              0XB                  //充电桩配置数据长度

#define RECT_MODULE_CONFIG_START_ADDR        0x1025               //模块配置数据起始地址
#define RECT_MODULE_CONFIG_DATA_LEN          0XB                  //模块桩配置数据长度

#define CCU_CONFIG_START_ADDR                0x1000               //CCU配置数据起始地址
#define CCU_CONFIG_DATA_LEN                  0X030                //CCU配置数据长度

#define GUN1_CHARGE_CMD_START_ADDR           0x1038               //枪1充电命令起始地址  
#define GUN2_CHARGE_CMD_START_ADDR           0x1038               //枪2充电命令起始地址  
#define GUN_CHARGE_CMD_DATA_LEN              0x5                   //枪充电命令长度

typedef struct DATA_SYS_INFO_T
{
    U16_T version;            // 充电桩内部通信协议版本
    U16_T ccuSwVer;       // CCU监控软件主版本号
    U16_T tcuHwVer;       // CCU监控硬件主版本号
    F32_T caseTemp; // 充电桩环境温度
    U16_T diStatus; // CCU控制器DI状态
} DATA_SYS_INFO_T,*pstDATA_SYS_INFO_T;

typedef struct
{
    U8_T chgState;          // 充电机状态
    U16_T gunConnState;      // 充电枪连接状态
    U16_T gunLockState;      // 充电枪锁状态
    U16_T gunResState;       // 充电枪归位状态
    F32_T ccs1Volt;         // CCS1电压
    F32_T gunTemp1;           // 枪温度1
    F32_T gunTemp2;           // 枪温度2
    U16_T gunState1;        // 枪状态STATE1
    U16_T gunState2;        // 枪状态STATE2
    U16_T gunState3;        // 枪状态STATE3
    U16_T gunMaxRate;       // 枪最大功率
    U16_T gunMaxCurr;       // 枪最大输出电流
    U32_T gunStarFailReson; // 枪启动失败原因
    U64_T gunstopReson;     // 枪停机原因
    F32_T meterVolt;        // 电表测量电压值
    F32_T meterCurr;        // 电表测量电流值
    F32_T meterRead;          // 电表当前读数
    U16_T busPosToGroundVolt; // 枪母线正对地电压
    U16_T busNegToGroundVolt; // 枪母线负对地电压
    U16_T busPosToGroundRes;  // 枪母线正对地电阻
    U16_T busNegToGroundRes;  // 枪母线负对地电阻
    U16_T busOutVoltSideA;    // 母线输出电压A侧
    U16_T busOutCuSideA;      // 母线输出电流A侧
    U16_T busOutVoltSideB;    // 母线输出电压B侧
    U16_T busOutCuSideB;      // 母线输出电流B侧
} DATA_GUN_INFO_T;


typedef struct
{
    F32_T cellmaxlAlwChgVolt;   // 单体动力蓄电池最高允许充电电压，数据范围0~24V，精度0.01
    F32_T maxAlwChgCurr;        // 最高允许充电电流，数据范围0~400A，精度0.1
    F32_T nominalTotalElect;    // 动力蓄电池标称总能量，数据范围0~1000kW.h，精度0.1
    F32_T maxAlwChgVolt;        // 最高允许充电总电压，数据范围0~750V，精度0.1
    S16_T maxAlwTemp;           // 最高允许动力蓄电池温度，数据范围-50度~200度
    U8_T  batEleSOC;            // 整车动力蓄电池荷电状态
    U8_T  bmsFlag;              // 接收bms报文标志
    F32_T batNeedVolt;          // 电压需求，数据范围0~750V，精度0.1
    F32_T battNeedCurr;        // 电流需求，数据范围0~400V，精度0.1
    U8_T  chgMode;            // 充电模式，0x01:恒压充电;0x02:恒流充电
    F32_T chgVolt;                      // 充电电压测量值，数据范围0~750V，精度0.1
    F32_T chgCurr;                      // 充电电流测量值，数据范围0~400A，精度0.1
    F32_T cellMaxChgVolt;               // 最高单体动力蓄电池电压，数据范围0~24V，精度0.01
    F32_T cellMaxVoltGroupNum; // 最高单体动力蓄电池电压所在组号，范围1~16
    U8_T  batSoc;               // 当前荷电状态，数据范围0~100%
    U16_T remainChgTime;       // 估算剩余充电时间，数据范围0~600min
    U8_T  cellMaxVoltID; // 最高单体动力蓄电池电压所在编号，数据范围1~256
    S16_T batMaxTemp;            // 最高动力蓄电池温度，数据范围-50度~200度
    U8_T  maxTempID;     // 最高温度检测点编号，数据范围1~128
    S16_T batMinTemp;            // 最低动力蓄电池温度，数据范围-50度~200度
    U8_T  minTempID;     // 最低温度检测点编号，数据范围1~128
    U16_T batState;              // 电池状态
    U8_T  batEndSoc;                      //中止荷电状态SOC，数据范围0~100%
    F32_T cellEndMinVolt;                //动力蓄电池单体最低电压，数据范围0~24V，精度0.01
    F32_T cellEndMaxVolt;                //动力蓄电池单体最高电压，数据范围0~24V，精度0.01
    S16_T batEndMinTemp;                //动力蓄电池最低温度，数据范围-50度~200度
    S16_T batEndMaxTemp;                //动力蓄电池最高温度，数据范围-50度~200度
    U8_T vin[18];         // 车辆识别码VIN
} DATA_BMS_INFO_T;


typedef struct DATA_CHARGER_INFO_T
{
    DATA_GUN_INFO_T     gunData;
    DATA_BMS_INFO_T     bmsData;
} DATA_CHARGER_INFO_T,*pstDATA_CHARGER_INFO_T;


typedef struct DATA_RECT_INFO_T
{
    U8_T commState;//通信状态
    U32_T rectState;//整流模块状态定义
    F32_T rectOutVolt;//整流模块输出电压
    F32_T rectOutCurr;//整流模块输出电流
    F32_T rectLimitPoint;//整流模块限流点
    F32_T rectTemp;//整流模块温度
    F32_T rectOutPower;//整流模块输出功率
} DATA_RECT_INFO_T,*pstDATA_RECT_INFO_T;



typedef struct CFG_CONTROLLER_INFO_T
{
    U16_T proVer;        // 充电桩内部通信协议版本
    U8_T ctrlDebug;      // 控制器调试
    U8_T chrBusNum;    // 充电机母线段数
    U8_T haveParCont;  // 有无并联接触器
    U8_T haveAcCont;   // 有无交流接触器
    U8_T acContStat;    // 交流接触器状态采集
    U8_T dcContStat;    // 直流接触器状态采集
    U8_T parContStat;   // 并联接触器状态采集
    U8_T standbyLockGun;     // 待机锁枪
    U8_T gunResDetect;     // 充电枪归位检测
    U8_T disChrFault;     // 泄放回路故障
    U8_T outPutUnctrl;       // 输出不可控
    U8_T coolSysType;        // 充电系统散热类型
    U8_T coolSysActivate;    // 散热系统开启方式
    U8_T gunLockStatType;    // 充电枪锁状态类型
    U8_T gunElecLockCtrl;    // 充电枪电子锁控制
} CFG_CONTROLLER_INFO_T,*pstCFG_CONTROLLER_INFO_T;



typedef struct CFG_CHARGER_INFO_T
{
    U8_T insBridDelay;     //绝缘电桥投切延时
    S16_T fanStartTemp;      //风机启动温度
    S16_T fanStopTemp;       //风机停机温度
    S16_T chrOverTempThr;    //充电桩过温阀值
    S16_T gunCurrDownThr;    //充电枪过温降流阀值
    S16_T gunShutDownThr;    //充电枪过温停机阀值
    U8_T bmsTimeoutCof;      //BMS通信超时系数
    U8_T contAdhesionThr;    //车内接触器粘连阀值
    F32_T gunMaxCurr;        //充电枪最大电流
    F32_T dcBusVoltA;       //一段输入侧直流母线电压
    F32_T dcBusVoltB;       //二段输入侧直流母线电压
} CFG_CHARGER_INFO_T, *pstCFG_CHARGER_INFO_T;



typedef struct CFG_RECT_MODULE_INFO_T
{
    U8_T rectNum;   // 整流模块数量
    U16_T rectPro; // 整流模块协议
    F32_T rectRatedCurr; // 整流模块额定电流
    U16_T rectFailDelayTime; // 整流模块故障延时时间
    F32_T rectRatedPower; // 整流模块额定功率
    U16_T rectCommBreakTime; // 整流模块通讯中断时间
    F32_T rectOutputOverVolt; // 整流模块输出过压
    F32_T rectOutputMaxVolt; // 整流模块输出电压最大值
    F32_T rectOutputMinVolt; // 整流模块输出电压最小值
    U8_T rectMaxLimitCurr; // 整流模块限流最大值
    U8_T rectMinLimitCurr; // 整流模块限流最小值
} CFG_RECT_MODULE_INFO_T,*pstCFG_RECT_MODULE_INFO_T;


typedef struct CMD_CHARGE_INFO_T
{
    U8_T assignPower;     //辅助电源控制
    U8_T dualGunCharge;  // 是否允许双枪给同一辆车充电
    U8_T ctrCmd;         //控制命令
    U8_T chargeTypeCtr;  //充放电控制
    F32_T powerCtr;      //功率控制值
} CMD_CHARGE_INFO_T,*pstCMD_CHARGE_INFO_T;


typedef struct CFG_CCU_INFO_T
{
    CFG_CONTROLLER_INFO_T ctrlConfig;     //控制器配置
    CFG_CHARGER_INFO_T chrConfig;       // 充电桩配置
    CFG_RECT_MODULE_INFO_T rectConfig;  //整流模块配置
} CFG_CCU_INFO_T,*pstCFG_CCU_INFO_T;


/* 定义回调函数类型 */
typedef U32_T (*CCU_RECV_ERROR_F)(U16_T funCode,U16_T errorCode);
typedef U32_T (*CCU_RECV_CONFIG_FINISH_F)();
typedef U32_T (*CCU_RECV_MESSAGE_F)();
typedef U32_T (*CCU_RECV_CONFIG_REQ_F)();
typedef U32_T (*CCU_RECV_DI_STATE_F)(U16_T diState);
typedef U32_T (*CCU_RECV_CHARGE_STATE_F)(U16_T gunID,U16_T chrState);
typedef U32_T (*CCU_RECV_GUN_STATE_F)(U16_T gunID,U16_T gunState);
typedef U32_T (*CCU_RECV_RECT_FAULT_F)(U16_T rectID,U16_T faultCode);
typedef U32_T (*CCU_RECV_CCU_STATE1_FAULT_F)(U16_T gunID,U16_T faultCode);
typedef U32_T (*CCU_RECV_CCU_STATE2_FAULT_F)(U16_T gunID,U16_T faultCode);

typedef enum
{
    CCU_RECV_ERROR = 0,      //错误帧回调
    CCU_RECV_CONFIG_FINISH = 0,      //配置成功
    CCU_RECV_MESSAGE,        //接收信息
    CCU_RECV_CONFIG_REQ,      //配置请求
    CCU_RECV_DI_STATE,  //di状态发生改变
    CCU_RECV_CHARGE_STATE,  //充电机状态发生改变
    CCU_RECV_GUN_STATE, //枪状态发生改变
    CCU_RECV_RECT_FAULT, //模块故障
    CCU_RECV_CCU_STATE1_FAULT,  //State1故障
    CCU_RECV_CCU_STATE2_FAULT //State2故障
} CCU_SERVICE_TYPE_E;


typedef struct
{
    U8_T *portPath;              // 串口设备文件路径
    COM_BAUD comBaud;            // 波特率
    COM_PARITY parity;           // 奇偶校验
} CCU_CFG_T;

U32_T ET_CCU_CommInit(CCU_CFG_T *config);
void ET_CCU_SetRectNum(U16_T num);
U32_T ET_CCU_UsartRecvData(void);
U32_T ET_CCU_TelemetrySysInfo();
U32_T ET_CCU_TelemetryFreeGunData(U8_T gunID);
U32_T ET_CCU_TelemetryChargeGunData(U8_T gunID);
U32_T ET_CCU_TelemetryRectModuleData();

void ET_CCU_RegisterCallback(CCU_SERVICE_TYPE_E service, void *callback);

U32_T ET_CCU_GetSysInfoData(pstDATA_SYS_INFO_T pSysData);
U32_T ET_CCU_GetChargeData(U8_T gunID,pstDATA_CHARGER_INFO_T pGunData);
U32_T ET_CCU_GetRectModuleData(U8_T rectId,pstDATA_RECT_INFO_T pRectData);


U32_T ET_CCU_SetConTroConfig(pstCFG_CONTROLLER_INFO_T pCtrConfig);
U32_T ET_CCU_SetChargerConfig(pstCFG_CHARGER_INFO_T pChrConfig);
U32_T ET_CCU_SetRectModConfig(pstCFG_RECT_MODULE_INFO_T pRectModConfig);
U32_T ET_CCU_SetCcuConfig(pstCFG_CCU_INFO_T pConfig);

U32_T ET_CCU_SendChargeCmd(pstCMD_CHARGE_INFO_T pChrCmd, U32_T gunID);

#ifdef __cplusplus
}
#endif

#endif
