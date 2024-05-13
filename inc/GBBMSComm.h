#ifndef __GBBMS_COMM_H__
#define __GBBMS_COMM_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
    unsigned int maxChgVolt; // 最高允许充电总电压
} GB_BHM_DATA_T;

typedef struct
{
    int protVer;                   // BMS通信协议版本，例如版本V1.0，表示为0x000100
    unsigned char batType;         // 电池类型，01H,铅酸电池;02H:镍氢电池;03H:磷酸铁锂电池;04H:锰酸锂电池;05H:钴酸锂电池;
                                    // 06H:三元材料电池;07H:聚合物锂离子电池;08H;钛酸锂电池;FFH:其他电池
    unsigned short ratedCapacity;           // 整车动力蓄电池系统额定容量，数据范围0~1000A.h，精度0.1
    unsigned short ratedVolt;               // 整车动力蓄电池系统额定总电压，数据范围0~750V，精度0.1
    char manufacturer[4];          // 电池生产厂商，标准ASCII
    int batID;                     // 电池组序号
    unsigned char producYear;  // 电池组生产日期（年）
    unsigned char producMonth; // 电池组生产日期（月）
    unsigned char producDay;   //  池组生产日期（日）
    int chgTimes;                  // 电池充电次数
    char property;                  // 电池产权标识
    unsigned char vin[17];         // 车辆识别码VIN
} GB_BRM_DATA_T;

typedef struct
{
    unsigned short cellMaxChgVolt;    // 单体动力蓄电池最高允许充电电压，数据范围0~24V，精度0.01
    unsigned short maxChgCurr;        // 最高允许充电电流，数据范围0~400A，精度0.1
    unsigned short nominalTotalElect; // 动力蓄电池标称总能量，数据范围0~1000kW.h，精度0.1
    unsigned short maxChgVolt;        // 最高允许充电总电压，数据范围0~750V，精度0.1
    unsigned short maxTemp;  // 最高允许动力蓄电池温度，数据范围-50度~200度
    unsigned short soc;               // 整车动力蓄电池荷电状态(SOC)，数据范围0~100%，精度0.1，这个SOC是充电开始时的SOC
    unsigned short batVolt;           // 整车动力蓄电池总电压，数据范围0~750V，精度0.1
} GB_BCP_DATA_T;

typedef struct
{
    unsigned char bmsChgReady; // BMS是否充电准备好，0x00: BMS未做好充电准备；0xAA: BMS完成充电准备；0xFF: 无效
} GB_BRO_DATA_T;

typedef struct
{
    unsigned short batNeedVolt;     // 电压需求，数据范围0~750V，精度0.1
    unsigned short battNeedCurr;    // 电流需求，数据范围0~400V，精度0.1
    unsigned char chgMode; // 充电模式，0x01:恒压充电;0x02:恒流充电
} GB_BCL_DATA_T;

typedef struct
{
    unsigned short chgVolt;                      // 充电电压测量值，数据范围0~750V，精度0.1
    unsigned short chgCurr;                      // 充电电流测量值，数据范围0~400A，精度0.1
    unsigned short cellMaxChgVolt;               // 最高单体动力蓄电池电压，数据范围0~24V，精度0.01
    unsigned short cellMaxVoltGroupNum; // 最高单体动力蓄电池电压所在组号，范围1~16
    unsigned char batSoc;               // 当前荷电状态，数据范围0~100%
    unsigned short remainChgTime;       // 估算剩余充电时间，数据范围0~600min
} GB_BCS_DATA_T;

typedef struct
{
    unsigned char cellMaxVoltID; // 最高单体动力蓄电池电压所在编号，数据范围1~256
    short batMaxTemp;            // 最高动力蓄电池温度，数据范围-50度~200度
    unsigned char maxTempID;     // 最高温度检测点编号，数据范围1~128
    short batMinTemp;            // 最低动力蓄电池温度，数据范围-50度~200度
    unsigned char minTempID;     // 最低温度检测点编号，数据范围1~128
    unsigned short batState;              // 电池状态
                    // bit0:单体动力蓄电池电压过高
                    // bit1:单体动力蓄电池电压过低
                    // bit2:整车动力蓄电池荷电状态SOC过高
                    // bit3:整车动力蓄电池荷电状态SOC过低
                    // bit4:动力蓄电池充电过流
                    // bit5:动力蓄电池温度过高
                    // bit6:动力蓄电池绝缘状态不正常
                    // bit7:动力蓄电池组输出连接器连接状态不正常
                    // bit8:充电允许，0：禁止，1：允许
} GB_BSM_DATA_T;


typedef struct
{
    unsigned short cellBatVolt[128]; // 各个单体动力蓄电池电压值
} GB_BMV_DATA_T;

typedef struct
{
    unsigned char cellBatTemp[128]; // 各个单体动力蓄电池温度值
} GB_BMT_DATA_T;

typedef struct
{
    unsigned char powerBat[16]; // 动力蓄电池预留字段
} GB_BSP_DATA_T;

typedef struct
{
    unsigned char bmsStopReason;       // BMS中止充电原因
    unsigned short bmsStopFaultReason; // BMS中止充电故障原因
    unsigned short bmsStopErrorReason; // BMS中止错误原因
} GB_BST_DATA_T;

typedef struct
{
	unsigned char          batEndSoc;                      //中止荷电状态SOC，数据范围0~100%
	unsigned short          cellEndMinVolt;                //动力蓄电池单体最低电压，数据范围0~24V，精度0.01
	unsigned short         cellEndMaxVolt;                //动力蓄电池单体最高电压，数据范围0~24V，精度0.01
	char         batEndMinTemp;                //动力蓄电池最低温度，数据范围-50度~200度
	char         batEndMaxTemp;                //动力蓄电池最高温度，数据范围-50度~200度
} GB_BSD_DATA_T;


typedef struct
{
    GB_BHM_DATA_T bhm; //BMS握手报文
    GB_BRM_DATA_T brm; //BMS和车辆辨识报文
    GB_BCP_DATA_T bcp; //动力蓄电池充电参数报文
    GB_BRO_DATA_T bro; //充电准备就绪报文
    GB_BCL_DATA_T bcl; //电池充电需求报文
    GB_BCS_DATA_T bcs; //电池充电总状态报文
    GB_BSM_DATA_T bsm; //发送动力蓄电池状态信息报文
    GB_BMV_DATA_T bmv; //单体动力蓄电池电压报文
    GB_BMT_DATA_T bmt; //动力蓄电池温度报文
    GB_BSP_DATA_T bsp; //动力蓄电池预留报文
    GB_BST_DATA_T bst; //中止充电报文
    GB_BSD_DATA_T bsd; //BMS统计数据报文
} GB_BMS_DATA_T;

typedef enum
{
	BMS_RECV_BHM = 0,   //BHM
    BMS_RECV_RMM,       //BRM
    BMS_RECV_BCP,       //BCP
    BMS_RECV_BRO,       //BRO
    BMS_RECV_BCL,       //BCL
    BMS_RECV_BCS,       //BCS
    BMS_RECV_BSM,       //BSM
    BMS_RECV_BMV,       //BMV
    BMS_RECV_BMT,       //BMT
    BMS_RECV_BSP,       //BSP
    BMS_RECV_BST,       //BST
    BMS_RECV_BSD        //BSD 
} BMS_RECV_DATA_E;                                //bms数据枚举



#ifdef __cplusplus
}
#endif

#endif