#ifndef __OCPP_CHARGING_PROFILE__H__
#define __OCPP_CHARGING_PROFILE__H__

#include <sqlite3.h>

#define CHARGINGPROFILE_DB_PATH "/app/etc/chargingProfile.db"
typedef struct
{
    int startPeriod;
    double limit;
    int numberPhases;
} chargingSchedulePeriod;

// 定义 ChargingSchedule 结构体
typedef struct
{
    int duration;
    char startSchedule[32];
    char chargingRateUnit[3];
    double minChargingRate;
    chargingSchedulePeriod ChargingSchedulePeriod[48];
    int numPeriods;
} ChargingSchedule;

typedef struct
{
    int chargingProfileId;
    int transactionId;
    int stackLevel;
    char chargingProfilePurpose[50];
    char chargingProfileKind[20];
    char recurrencyKind[10];
    char validFrom[32];
    char validTo[32];
    ChargingSchedule chargingSchedule;
} chargingProfile;


int ocpp_ChargingProfile_init();
int ocpp_ChargingProfile_read(int connectorId, int duration, chargingProfile *profile);
int ocpp_ChargingProfile_delete(int chargingProfileId);
int ocpp_ChargingProfile_Clear();

#endif