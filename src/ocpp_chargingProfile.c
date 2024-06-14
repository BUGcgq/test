#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "ocpp_chargingProfile.h"

static sqlite3 *ocpp_cf = NULL;

static int open_database(const char *db_name)
{
    if (ocpp_cf == NULL)
    {
        if (sqlite3_open(db_name, &ocpp_cf) != SQLITE_OK)
        {
            fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(ocpp_cf));
            return -1;
        }
    }
    return 0;
}

static void close_database()
{
    if (ocpp_cf != NULL)
    {
        sqlite3_close(ocpp_cf);
        ocpp_cf = NULL;
    }
}

static void rollback(sqlite3 *db)
{
    if (sqlite3_exec(db, "ROLLBACK;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error during rollback: %s\n", sqlite3_errmsg(db));
    }
}

static int ocpp_ChargingProfile_create_tables()
{
    if (ocpp_cf == NULL)
    {
        fprintf(stderr, "Database connection is not open.\n");
        return -1;
    }

    char *errMsg = 0;

    // 开始事务
    if (sqlite3_exec(ocpp_cf, "BEGIN TRANSACTION;", NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }

    // 创建 ChargingProfiles 表
    const char *chargingProfilesTableSQL =
        "CREATE TABLE IF NOT EXISTS ChargingProfiles ("
        "chargingProfileId INT PRIMARY KEY NOT NULL,"
        "connectorId INT NOT NULL,"
        "transactionId INT,"
        "stackLevel INT NOT NULL,"
        "chargingProfilePurpose TEXT NOT NULL,"
        "chargingProfileKind TEXT NOT NULL,"
        "recurrencyKind TEXT,"
        "validFrom TEXT,"
        "validTo TEXT"
        ");";

    if (sqlite3_exec(ocpp_cf, chargingProfilesTableSQL, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        rollback(ocpp_cf);
        return -1;
    }

    // 创建 ChargingSchedule 表
    const char *chargingScheduleTableSQL =
        "CREATE TABLE IF NOT EXISTS ChargingSchedule ("
        "chargingScheduleId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "chargingProfileId INT NOT NULL,"
        "duration INT,"
        "startSchedule TEXT,"
        "chargingRateUnit TEXT NOT NULL,"
        "minChargingRate REAL,"
        "numPeriods INT,"
        "FOREIGN KEY (chargingProfileId) REFERENCES ChargingProfiles(chargingProfileId)"
        ");";

    if (sqlite3_exec(ocpp_cf, chargingScheduleTableSQL, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        rollback(ocpp_cf);
        return -1;
    }

    // 创建 ChargingSchedulePeriod 表
    const char *chargingSchedulePeriodTableSQL =
        "CREATE TABLE IF NOT EXISTS ChargingSchedulePeriod ("
        "chargingSchedulePeriodId INTEGER PRIMARY KEY AUTOINCREMENT,"
        "chargingProfileId INT NOT NULL,"
        "startPeriod INT NOT NULL,"
        "restrict REAL NOT NULL,"
        "numberPhases INT NOT NULL,"
        "FOREIGN KEY (chargingProfileId) REFERENCES ChargingProfiles(chargingProfileId)"
        ");";

    if (sqlite3_exec(ocpp_cf, chargingSchedulePeriodTableSQL, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        rollback(ocpp_cf);
        return -1;
    }

    // 提交事务
    if (sqlite3_exec(ocpp_cf, "COMMIT;", NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        rollback(ocpp_cf);
        return -1;
    }

    return 0;
}

int ocpp_ChargingProfile_insert(int connectorId, const chargingProfile *ChargingProfile)
{
    if (ocpp_cf == NULL || ChargingProfile == NULL)
    {
        fprintf(stderr, "Invalid database connection or ChargingProfile pointer.\n");
        return -1;
    }

    // 开始事务
    if (sqlite3_exec(ocpp_cf, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1; // 开始事务失败
    }

    sqlite3_stmt *stmt = NULL;

    // 插入到 ChargingProfiles 表
    const char *chargingProfilesInsertSQL =
        "INSERT INTO ChargingProfiles (chargingProfileId, connectorId, transactionId, stackLevel, chargingProfilePurpose, chargingProfileKind, recurrencyKind, validFrom, validTo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(ocpp_cf, chargingProfilesInsertSQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // SQL 准备失败
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, ChargingProfile->chargingProfileId);
    sqlite3_bind_int(stmt, 2, connectorId);
    sqlite3_bind_int(stmt, 3, ChargingProfile->transactionId);
    sqlite3_bind_int(stmt, 4, ChargingProfile->stackLevel);
    sqlite3_bind_text(stmt, 5, ChargingProfile->chargingProfilePurpose, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, ChargingProfile->chargingProfileKind, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, ChargingProfile->recurrencyKind, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, ChargingProfile->validFrom, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, ChargingProfile->validTo, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Insertion into ChargingProfiles failed: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        sqlite3_finalize(stmt);
        return -1; // 插入失败
    }

    sqlite3_finalize(stmt); // 释放语句对象

    // 插入到 ChargingSchedule 表
    const char *chargingScheduleInsertSQL =
        "INSERT INTO ChargingSchedule (chargingProfileId, duration, startSchedule, chargingRateUnit, minChargingRate, numPeriods) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(ocpp_cf, chargingScheduleInsertSQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // SQL 准备失败
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, ChargingProfile->chargingProfileId);
    sqlite3_bind_int(stmt, 2, ChargingProfile->chargingSchedule.duration);
    sqlite3_bind_text(stmt, 3, ChargingProfile->chargingSchedule.startSchedule, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, ChargingProfile->chargingSchedule.chargingRateUnit, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, ChargingProfile->chargingSchedule.minChargingRate);
    sqlite3_bind_int(stmt, 6, ChargingProfile->chargingSchedule.numPeriods);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Insertion into ChargingSchedule failed: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        sqlite3_finalize(stmt);
        return -1; // 插入失败
    }

    sqlite3_finalize(stmt); // 释放语句对象

    // 插入到 ChargingSchedulePeriod 表
    const char *chargingSchedulePeriodInsertSQL =
        "INSERT INTO ChargingSchedulePeriod (chargingProfileId, startPeriod, restrict, numberPhases) "
        "VALUES (?, ?, ?, ?);";

    int i;
    for (i = 0; i < ChargingProfile->chargingSchedule.numPeriods && i < 48; i++)
    {
        if (sqlite3_prepare_v2(ocpp_cf, chargingSchedulePeriodInsertSQL, -1, &stmt, NULL) != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
            rollback(ocpp_cf);
            return -1; // SQL 准备失败
        }

        // 绑定参数
        sqlite3_bind_int(stmt, 1, ChargingProfile->chargingProfileId);
        sqlite3_bind_int(stmt, 2, ChargingProfile->chargingSchedule.ChargingSchedulePeriod[i].startPeriod);
        sqlite3_bind_double(stmt, 3, ChargingProfile->chargingSchedule.ChargingSchedulePeriod[i].limit);
        sqlite3_bind_int(stmt, 4, ChargingProfile->chargingSchedule.ChargingSchedulePeriod[i].numberPhases);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            fprintf(stderr, "Insertion into ChargingSchedulePeriod failed: %s\n", sqlite3_errmsg(ocpp_cf));
            rollback(ocpp_cf);
            sqlite3_finalize(stmt);
            return -1; // 插入失败
        }

        sqlite3_finalize(stmt); // 释放语句对象
    }

    // 提交事务
    if (sqlite3_exec(ocpp_cf, "COMMIT;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // 提交事务失败
    }

    return 0; // 成功
}

int ocpp_ChargingProfile_delete(int chargingProfileId)
{
    if (ocpp_cf == NULL)
    {
        fprintf(stderr, "Invalid database connection.\n");
        return -1;
    }

    char *errMsg = 0;

    // 开始事务
    if (sqlite3_exec(ocpp_cf, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1; // 开始事务失败
    }

    sqlite3_stmt *stmt = NULL;

    // 删除 ChargingSchedulePeriod 表中相关记录
    const char *deleteChargingSchedulePeriodSQL =
        "DELETE FROM ChargingSchedulePeriod WHERE chargingProfileId = ?;";

    if (sqlite3_prepare_v2(ocpp_cf, deleteChargingSchedulePeriodSQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // SQL 准备失败
    }

    sqlite3_bind_int(stmt, 1, chargingProfileId);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Deletion from ChargingSchedulePeriod failed: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        sqlite3_finalize(stmt);
        return -1; // 删除失败
    }

    sqlite3_finalize(stmt); // 释放语句对象

    // 删除 ChargingSchedule 表中相关记录
    const char *deleteChargingScheduleSQL =
        "DELETE FROM ChargingSchedule WHERE chargingProfileId = ?;";

    if (sqlite3_prepare_v2(ocpp_cf, deleteChargingScheduleSQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // SQL 准备失败
    }

    sqlite3_bind_int(stmt, 1, chargingProfileId);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Deletion from ChargingSchedule failed: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        sqlite3_finalize(stmt);
        return -1; // 删除失败
    }

    sqlite3_finalize(stmt); // 释放语句对象

    // 删除 ChargingProfiles 表中相关记录
    const char *deleteChargingProfilesSQL =
        "DELETE FROM ChargingProfiles WHERE chargingProfileId = ?;";

    if (sqlite3_prepare_v2(ocpp_cf, deleteChargingProfilesSQL, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // SQL 准备失败
    }

    sqlite3_bind_int(stmt, 1, chargingProfileId);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        fprintf(stderr, "Deletion from ChargingProfiles failed: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        sqlite3_finalize(stmt);
        return -1; // 删除失败
    }

    sqlite3_finalize(stmt); // 释放语句对象

    // 提交事务
    if (sqlite3_exec(ocpp_cf, "COMMIT;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // 提交事务失败
    }

    return 0; // 成功
}

int ocpp_ChargingProfile_Clear()
{
    if (ocpp_cf == NULL)
    {
        fprintf(stderr, "Invalid database connection.\n");
        return -1;
    }

    char *errMsg = 0;

    // 开始事务
    if (sqlite3_exec(ocpp_cf, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1; // 开始事务失败
    }

    // 删除 ChargingSchedulePeriod 表
    const char *dropChargingSchedulePeriodSQL = "DROP TABLE IF EXISTS ChargingSchedulePeriod;";
    if (sqlite3_exec(ocpp_cf, dropChargingSchedulePeriodSQL, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        rollback(ocpp_cf);
        return -1;
    }

    // 删除 ChargingSchedule 表
    const char *dropChargingScheduleSQL = "DROP TABLE IF EXISTS ChargingSchedule;";
    if (sqlite3_exec(ocpp_cf, dropChargingScheduleSQL, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        rollback(ocpp_cf);
        return -1;
    }

    // 删除 ChargingProfiles 表
    const char *dropChargingProfilesSQL = "DROP TABLE IF EXISTS ChargingProfiles;";
    if (sqlite3_exec(ocpp_cf, dropChargingProfilesSQL, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        rollback(ocpp_cf);
        return -1;
    }

    // 提交事务
    if (sqlite3_exec(ocpp_cf, "COMMIT;", 0, 0, 0) != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(ocpp_cf));
        rollback(ocpp_cf);
        return -1; // 提交事务失败
    }

    return 0; // 成功
}

int ocpp_ChargingProfile_read(int connectorId, int duration, chargingProfile *profile)
{
    if (ocpp_cf == NULL || profile == NULL)
    {
        fprintf(stderr, "Invalid database connection or profile pointer.\n");
        return -1;
    }

    sqlite3_stmt *stmt = NULL;
    char query[512];
    int result = 0;

    // 构建查询语句
    snprintf(query, sizeof(query),
             "SELECT cp.chargingProfileId, cp.transactionId, cp.stackLevel, cp.chargingProfilePurpose, cp.chargingProfileKind, "
             "cp.recurrencyKind, cp.validFrom, cp.validTo, cs.duration, cs.startSchedule, cs.chargingRateUnit, "
             "cs.minChargingRate, cs.numPeriods "
             "FROM ChargingProfiles cp "
             "JOIN ChargingSchedule cs ON cp.chargingProfileId = cs.chargingProfileId "
             "WHERE cp.connectorId = ?%s "
             "ORDER BY cp.stackLevel DESC LIMIT 1;",
             duration ? " AND cs.duration = ?" : "");

    // 预处理查询语句
    if (sqlite3_prepare_v2(ocpp_cf, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, connectorId);
    if (duration)
    {
        sqlite3_bind_int(stmt, 2, duration);
    }

    // 执行查询
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        // 读取 ChargingProfiles 数据
        profile->chargingProfileId = sqlite3_column_int(stmt, 0);
        profile->transactionId = sqlite3_column_int(stmt, 1);
        profile->stackLevel = sqlite3_column_int(stmt, 2);
        strcpy(profile->chargingProfilePurpose, (const char *)sqlite3_column_text(stmt, 3));
        strcpy(profile->chargingProfileKind, (const char *)sqlite3_column_text(stmt, 4));
        strcpy(profile->recurrencyKind, (const char *)sqlite3_column_text(stmt, 5));
        strcpy(profile->validFrom, (const char *)sqlite3_column_text(stmt, 6));
        strcpy(profile->validTo, (const char *)sqlite3_column_text(stmt, 7));

        // 读取 ChargingSchedule 数据
        profile->chargingSchedule.duration = sqlite3_column_int(stmt, 8);
        strcpy(profile->chargingSchedule.startSchedule, (const char *)sqlite3_column_text(stmt, 9));
        strcpy(profile->chargingSchedule.chargingRateUnit, (const char *)sqlite3_column_text(stmt, 10));
        profile->chargingSchedule.minChargingRate = sqlite3_column_double(stmt, 11);
        profile->chargingSchedule.numPeriods = sqlite3_column_int(stmt, 12);
    }
    else
    {
        sqlite3_finalize(stmt);
        return -1; // 没有找到记录
    }

    sqlite3_finalize(stmt);

    // 查询 ChargingSchedulePeriod 表
    snprintf(query, sizeof(query),
             "SELECT startPeriod, restrict, numberPhases "
             "FROM ChargingSchedulePeriod "
             "WHERE chargingProfileId = ?;");

    // 预处理查询语句
    if (sqlite3_prepare_v2(ocpp_cf, query, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(ocpp_cf));
        return -1;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, profile->chargingProfileId);

    int periodIndex = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && periodIndex < 48)
    {
        profile->chargingSchedule.ChargingSchedulePeriod[periodIndex].startPeriod = sqlite3_column_int(stmt, 0);
        profile->chargingSchedule.ChargingSchedulePeriod[periodIndex].limit = sqlite3_column_double(stmt, 1);
        profile->chargingSchedule.ChargingSchedulePeriod[periodIndex].numberPhases = sqlite3_column_int(stmt, 2);
        periodIndex++;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int ocpp_ChargingProfile_init()
{
    if (open_database(CHARGINGPROFILE_DB_PATH) != 0)
    {
        return -1;
    }

    // 数据库不存在或打开失败，尝试创建数据库和表
    if (ocpp_ChargingProfile_create_tables() != 0)
    {
        close_database();
        return -1;
    }
    return 0;
}


// int main()
// {
// 	if (ocpp_ChargingProfile_init() != 0)
// 	{
// 		return -1;
// 	}

// 	// 插入示例数据
// 	chargingProfile profile = {
// 		.chargingProfileId = 1,
// 		.transactionId = 123,
// 		.stackLevel = 2,
// 		.chargingProfilePurpose = "Purpose",
// 		.chargingProfileKind = "Kind",
// 		.recurrencyKind = "Kind",
// 		.validFrom = "2023-01-01T00:00:00Z",
// 		.validTo = "2023-12-31T23:59:59Z",
// 		.chargingSchedule = {
// 			.duration = 3600,
// 			.startSchedule = "2023-01-01T00:00:00Z",
// 			.chargingRateUnit = "W",
// 			.minChargingRate = 10.0,
// 			.numPeriods = 2,
// 			.ChargingSchedulePeriod = {
// 				{.startPeriod = 0, .limit = 10.0, .numberPhases = 3},
// 				{.startPeriod = 1800, .limit = 5.0, .numberPhases = 3}}}};

// 	ocpp_ChargingProfile_insert(1, &profile);
// 	chargingProfile profile;
// 	memset(&profile, 0, sizeof(profile)); // 清空 profile 结构体
// 	int i;
// 	// 读取数据
// 	if (ocpp_ChargingProfile_read(1, 1, &profile) == 0)
// 	{
// 		printf("Charging Profile ID: %d\n", profile.chargingProfileId);
// 		printf("Transaction ID: %d\n", profile.transactionId);
// 		printf("Stack Level: %d\n", profile.stackLevel);
// 		printf("Charging Profile Purpose: %s\n", profile.chargingProfilePurpose);
// 		printf("Charging Profile Kind: %s\n", profile.chargingProfileKind);
// 		printf("Recurrency Kind: %s\n", profile.recurrencyKind);
// 		printf("Valid From: %s\n", profile.validFrom);
// 		printf("Valid To: %s\n", profile.validTo);
// 		printf("Charging Schedule Duration: %d\n", profile.chargingSchedule.duration);
// 		printf("Charging Schedule Start Schedule: %s\n", profile.chargingSchedule.startSchedule);
// 		printf("Charging Rate Unit: %s\n", profile.chargingSchedule.chargingRateUnit);
// 		printf("Min Charging Rate: %.2f\n", profile.chargingSchedule.minChargingRate);
// 		printf("Number of Periods: %d\n", profile.chargingSchedule.numPeriods);

// 		for (i = 0; i < profile.chargingSchedule.numPeriods; i++)
// 		{
// 			printf("Period %d Start: %d\n", i, profile.chargingSchedule.ChargingSchedulePeriod[i].startPeriod);
// 			printf("Period %d Limit: %.2f\n", i, profile.chargingSchedule.ChargingSchedulePeriod[i].limit);
// 			printf("Period %d Number of Phases: %d\n", i, profile.chargingSchedule.ChargingSchedulePeriod[i].numberPhases);
// 		}
// 	}
// 	else
// 	{
// 		printf("No matching profile found.\n");
// 	}


// 	return 0;
// }