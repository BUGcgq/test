#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "ocpp_localAuthorization.h"

static sqlite3 *ocpp_AL = NULL;

static int open_database(const char *db_name)
{
	if (ocpp_AL == NULL)
	{
		if (sqlite3_open(db_name, &ocpp_AL) != SQLITE_OK)
		{
			fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(ocpp_AL));
			return -1;
		}
	}
	return 0;
}

static void close_database()
{
	if (ocpp_AL != NULL)
	{
		sqlite3_close(ocpp_AL);
		ocpp_AL = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////-本地认证缓存-////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @description: 创建authorizationCache数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
static int ocpp_localAuthorization_Cache_create_table()
{
	if (ocpp_AL == NULL)
	{
		return -1;
	}
	char *zErrMsg = 0;
	const char *sql = "CREATE TABLE IF NOT EXISTS AuthorizationCache ("
					  "NO INTEGER PRIMARY KEY AUTOINCREMENT,"
					  "IdTag TEXT NOT NULL UNIQUE,"
					  "status TEXT NOT NULL,"
					  "expiryDate TEXT NOT NULL,"
					  "parentIdTag TEXT"
					  ");";

	int rc = sqlite3_exec(ocpp_AL, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("创建 AuthorizationCache 表失败: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

/**
 * @description: 清空authorizationCache数据表
 * @param:
 * @return: 0-成功,-1-失败
 */
int ocpp_localAuthorization_Cache_clearCache(void)
{
	if (ocpp_AL == NULL)
		return -1;

	char sql[128]; // 修改缓冲区大小
	snprintf(sql, sizeof(sql), "DELETE FROM AuthorizationCache");

	char *ErrMsg = NULL;
	int rc = sqlite3_exec(ocpp_AL, sql, NULL, NULL, &ErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("ERROR: %s\n", ErrMsg);
		sqlite3_free(ErrMsg);
		return -1; // 清空表失败，返回错误码
	}

	return 0; // 清空表成功
}

/**
 * @description: 查看的 idTagInfo 是否存在缓存中
 * @param:
 * @return: true = 存在,fail = 不存在
 */
bool ocpp_localAuthorization_Cache_search(const char *idTag)
{
	if (ocpp_AL == NULL || idTag == NULL)
	{
		return false;
	}

	char sql[512];
	snprintf(sql, sizeof(sql), "SELECT * FROM AuthorizationCache WHERE IdTag = '%s';", idTag);

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("准备 SQL 查询失败: %s\n", sqlite3_errmsg(ocpp_AL));
		return false;
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		// 查询结果是 SQLITE_ROW，表示存在
		sqlite3_finalize(stmt);
		return true;
	}
	else if (rc == SQLITE_DONE)
	{
		// 查询结果是 SQLITE_DONE，表示不存在
		sqlite3_finalize(stmt);
		return false;
	}
	else
	{
		// 查询失败
		sqlite3_finalize(stmt);
		return false;
	}
}

/**
 * @description: 将一条缓存记录插入AuthorizationCache
 * @param {ocpp_localAuthorization_cache_record_t} cache_record
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_Cache_insert(const char *idTag, const char *status, const char *expiryDate, const char *parentIdTag)
{
	if (ocpp_AL == NULL || idTag == NULL || expiryDate == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化、无效的数据或空的IdTag或空的expiryDate
	}

	const char *sql;
	if (parentIdTag != NULL)
	{
		sql = "INSERT INTO AuthorizationCache (IdTag, status, expiryDate, parentIdTag) VALUES (?, ?, ?, ?);";
	}
	else
	{
		sql = "INSERT INTO AuthorizationCache (IdTag, status, expiryDate) VALUES (?, ?, ?);";
	}

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("准备 SQL 插入语句失败: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, idTag, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, status, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, expiryDate, -1, SQLITE_STATIC);
	if (parentIdTag != NULL)
	{
		sqlite3_bind_text(stmt, 4, parentIdTag, -1, SQLITE_STATIC);
	}

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		printf("执行 SQL 插入语句失败: %s\n", sqlite3_errmsg(ocpp_AL));
		sqlite3_finalize(stmt);
		return -1;
	}

	sqlite3_finalize(stmt);
	return 0; // 插入成功
}

/**
 * @description: 更新缓存中的 IdTagInfo
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_Cache_UpdateIdTagInfo(const char *idTag, const char *status, const char *expiryDate, const char *parentIdTag)
{
	if (ocpp_AL == NULL || idTag == NULL || expiryDate == NULL)
	{
		return -1; // 处理错误情况，例如数据库未初始化、无效的数据或空的IdTag或空的expiryDate
	}

	const char *sql;
	if (parentIdTag != NULL)
	{
		sql = "UPDATE AuthorizationCache SET status = ?, expiryDate = ?, parentIdTag = ? WHERE IdTag = ?;";
	}
	else
	{
		sql = "UPDATE AuthorizationCache SET status = ?, expiryDate = ?, parentIdTag = NULL WHERE IdTag = ?;";
	}

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("准备 SQL 更新语句失败: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1;
	}

	// 绑定参数
	sqlite3_bind_text(stmt, 1, status, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, expiryDate, -1, SQLITE_STATIC);
	if (parentIdTag != NULL)
	{
		sqlite3_bind_text(stmt, 3, parentIdTag, -1, SQLITE_STATIC);
	}
	sqlite3_bind_text(stmt, 4, idTag, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		printf("执行 SQL 更新语句失败: %s\n", sqlite3_errmsg(ocpp_AL));
		sqlite3_finalize(stmt);
		return -1;
	}

	sqlite3_finalize(stmt);
	return 0; // 更新成功
}

// 删除 AuthorizationCache 表中那些过期的数据
static int ocpp_localAuthorization_Cache_RemoveAllExpired(void)
{
	if (ocpp_AL == NULL)
	{
		printf("数据库未初始化\n");
		return -1;
	}

	// 准备 SQL 查询
	const char *sql = "DELETE FROM AuthorizationCache WHERE expiryDate < DATETIME('now');";
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("准备 SQL 查询失败: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1;
	}

	// 执行 SQL 查询
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE)
	{
		printf("执行 SQL 查询失败: %s\n", sqlite3_errmsg(ocpp_AL));
		sqlite3_finalize(stmt);
		return -1;
	}

	// 释放资源
	sqlite3_finalize(stmt);
	return 0;
}
/**
 * @description: 将接收到的 idTagInfo 写入缓存
 * @param:
 * @return: 0-成功,-1-失败
 */
int ocpp_localAuthorization_Cache_write(ocpp_localAuthorization_cache_record_t *cache_record)
{

	if (ocpp_AL == NULL || cache_record == NULL || cache_record->IdTag == NULL)
	{
		return -1;
	}

	if (ocpp_localAuthorization_Cache_search(cache_record->IdTag) == true) // 如果缓存存在该卡号
	{
		ocpp_localAuthorization_Cache_UpdateIdTagInfo(cache_record->IdTag, cache_record->status, cache_record->expiryDate, cache_record->parentIdTag); // 更新它的消息
	}
	else // 如果缓存不存在该卡号
	{
		ocpp_localAuthorization_Cache_insert(cache_record->IdTag, cache_record->status, cache_record->expiryDate, cache_record->parentIdTag);
	}

	ocpp_localAuthorization_Cache_RemoveAllExpired();
}

bool ocpp_localAuthorization_Cache_isValid(const char *idTag)
{
	if (ocpp_AL == NULL || idTag == NULL)
	{
		printf("数据库未初始化或输入为空\n");
		return false;
	}

	char sql[512];
	snprintf(sql, sizeof(sql), "SELECT status, expiryDate FROM AuthorizationCache WHERE IdTag='%s';", idTag);

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("准备 SQL 查询失败: %s\n", sqlite3_errmsg(ocpp_AL));
		return false;
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		const char *status = (const char *)sqlite3_column_text(stmt, 0);
		const char *expiryDateStr = (const char *)sqlite3_column_text(stmt, 1);

		if (strcmp(status, "Accepted") == 0)
		{
			// 解析 expiryDate
			struct tm expiryDate;
			memset(&expiryDate, 0, sizeof(struct tm));
			strptime(expiryDateStr, "%Y-%m-%dT%H:%M:%S", &expiryDate);

			// 获取当前日期
			time_t currentTime = time(NULL);
			struct tm *localTime = localtime(&currentTime);

			// 比较日期部分
			if (expiryDate.tm_year > localTime->tm_year ||																						  // 年份晚于当前年份
				(expiryDate.tm_year == localTime->tm_year && expiryDate.tm_mon > localTime->tm_mon) ||											  // 年份相同但月份晚于当前月份
				(expiryDate.tm_year == localTime->tm_year && expiryDate.tm_mon == localTime->tm_mon && expiryDate.tm_mday >= localTime->tm_mday)) // 年份和月份相同但日期晚于或等于当前日期
			{
				sqlite3_finalize(stmt);
				return true; // 在有效期内且状态为 "Accepted"
			}
		}
	}
	else if (rc != SQLITE_DONE)
	{
		printf("执行 SQL 查询失败: %s\n", sqlite3_errmsg(ocpp_AL));
	}

	sqlite3_finalize(stmt);
	return false; // 不在有效期内或状态不为 "Accepted"
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////-本地认证列表-////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int ocpp_localAuthorization_List_create_table()
{
	if (ocpp_AL == NULL)
	{
		printf("数据库未初始化\n");
		return -1;
	}

	int rc = 0;
	char *zErrMsg = 0;
	char sql[512];

	snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS AuthorizationList("
							   "NO                integer PRIMARY KEY,"
							   "IdTag             TEXT    NOT NULL UNIQUE,"
							   "status            TEXT    NOT NULL,"
							   "expiryDate        TEXT,"
							   "parentIdTag       TEXT    );");

	rc = sqlite3_exec(ocpp_AL, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("创建 AuthorizationList 表失败 %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

static int ocpp_localAuthorization_Version_create_table()
{
	if (ocpp_AL == NULL)
	{
		printf("数据库未初始化\n");
		return -1;
	}

	char *zErrMsg = 0;

	char sql[512];
	snprintf(sql, sizeof(sql), "CREATE TABLE IF NOT EXISTS Version ("
							   "TableName TEXT PRIMARY KEY,"
							   "Version INTEGER);");

	int rc = sqlite3_exec(ocpp_AL, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("创建 Version 表失败: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

int ocpp_localAuthorization_Version_setVersion(const char *tableName, int version)
{
	if (ocpp_AL == NULL || tableName == NULL)
	{
		printf("数据库未初始化或表名为空\n");
		return -1;
	}

	char *zErrMsg = 0;
	char sql[512];

	snprintf(sql, sizeof(sql), "INSERT OR REPLACE INTO Version (TableName, Version) VALUES ('%s', %d);", tableName, version);

	int rc = sqlite3_exec(ocpp_AL, sql, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("设置版本号失败: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

int ocpp_localAuthorization_List_getListVersion(const char *tableName)
{
	if (ocpp_AL == NULL || tableName == NULL)
	{
		return -1;
	}

	int version = -1; // 默认返回值为-1，表示失败

	char sql[512];
	if (snprintf(sql, sizeof(sql), "SELECT Version FROM Version WHERE TableName='%s';", tableName) >= sizeof(sql))
	{
		printf("表名过长\n");
		return -1;
	}

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("准备 SQL 查询失败: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1;
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		version = sqlite3_column_int(stmt, 0);
	}
	else if (rc != SQLITE_DONE)
	{
		printf("执行 SQL 查询失败: %s\n", sqlite3_errmsg(ocpp_AL));
		version = -1; // 失败时将版本号设为 -1
	}

	sqlite3_finalize(stmt);
	return version;
}


/**
 * @description: 查看的 idTagInfo 是否存在中
 * @param:
 * @return: true = 存在, false = 不存在
 */
bool ocpp_localAuthorization_List_search(const char *idTag)
{
	if (ocpp_AL == NULL || idTag == NULL)
	{
		return false; // 处理错误情况，例如数据库未初始化或传入了无效的 idTag
	}

	char sql[512];
	snprintf(sql, sizeof(sql), "SELECT IdTag FROM AuthorizationList WHERE IdTag = ?;");

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_AL));
		return false; // 处理数据库查询准备失败的情况
	}

	sqlite3_bind_text(stmt, 1, idTag, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	bool found = (rc == SQLITE_ROW);

	sqlite3_finalize(stmt);
	return found;
}

/**
 * @description: AuthorizationList
 * @param {const char *} idTag
 * @param {const char *} status
 * @param {const char *} expiryDate
 * @param {const char *} parentIdTag
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_List_insert(const char *idTag, const char *status, const char *expiryDate, const char *parentIdTag)
{
	if (ocpp_AL == NULL || idTag == NULL || status == NULL)
	{
		printf("Invalid input parameters\n");
		return -1; // 处理错误情况，例如数据库未初始化、传入了无效的参数等
	}

	char sql[512];
	snprintf(sql, sizeof(sql), "INSERT INTO AuthorizationList (IdTag, status, expiryDate, parentIdTag) VALUES (?, ?, ?, ?);");

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1; // 处理数据库查询准备失败的情况
	}

	sqlite3_bind_text(stmt, 1, idTag, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, status, -1, SQLITE_STATIC);

	if (expiryDate != NULL)
	{
		sqlite3_bind_text(stmt, 3, expiryDate, -1, SQLITE_STATIC);
	}
	else
	{
		sqlite3_bind_null(stmt, 3);
	}

	if (parentIdTag != NULL)
	{
		sqlite3_bind_text(stmt, 4, parentIdTag, -1, SQLITE_STATIC);
	}
	else
	{
		sqlite3_bind_null(stmt, 4);
	}

	rc = sqlite3_step(stmt);

	sqlite3_finalize(stmt);

	if (rc != SQLITE_DONE)
	{
		printf("Error inserting data: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1; // 处理数据插入失败的情况
	}

	return 0;
}

/**
 * @description: 更新列表中的 IdTagInfo
 * @param {const char *} idTag
 * @param {const char *} status
 * @param {const char *} expiryDate
 * @param {const char *} parentIdTag
 * @return {*} 0 = success, -1 = fail
 */
static int ocpp_localAuthorization_List_UpdateIdTagInfo(const char *idTag, const char *status, const char *expiryDate, const char *parentIdTag)
{
	if (ocpp_AL == NULL || idTag == NULL || status == NULL)
	{
		printf("Invalid input parameters\n");
		return -1; // 处理错误情况，例如数据库未初始化、传入了无效的参数等
	}

	char sql[512];
	snprintf(sql, sizeof(sql), "UPDATE AuthorizationList SET status = ?, expiryDate = ?, parentIdTag = ? WHERE IdTag = ?;");

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1; // 处理数据库查询准备失败的情况
	}

	sqlite3_bind_text(stmt, 1, status, -1, SQLITE_STATIC);

	if (expiryDate != NULL)
	{
		sqlite3_bind_text(stmt, 2, expiryDate, -1, SQLITE_STATIC);
	}
	else
	{
		sqlite3_bind_null(stmt, 2);
	}

	if (parentIdTag != NULL)
	{
		sqlite3_bind_text(stmt, 3, parentIdTag, -1, SQLITE_STATIC);
	}
	else
	{
		sqlite3_bind_null(stmt, 3);
	}

	sqlite3_bind_text(stmt, 4, idTag, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);

	sqlite3_finalize(stmt);

	if (rc != SQLITE_DONE)
	{
		printf("Error updating data: %s\n", sqlite3_errmsg(ocpp_AL));
		return -1; // 处理数据更新失败的情况
	}

	return 0;
}

/**
 * @description: 将接收到的 idTagInfo 写入缓存
 * @param:
 * @return: 0-成功,-1-失败
 */
int ocpp_localAuthorization_List_write(ocpp_localAuthorization_list_entry_t *list_entry)
{

	if (ocpp_AL == NULL || list_entry == NULL)
		return -1;

	if (ocpp_localAuthorization_List_search(list_entry->IdTag) == true)
	{
		ocpp_localAuthorization_List_UpdateIdTagInfo(list_entry->IdTag, list_entry->status, list_entry->expiryDate, list_entry->parentIdTag);
	}
	else
	{
		ocpp_localAuthorization_List_insert(list_entry->IdTag, list_entry->status, list_entry->expiryDate, list_entry->parentIdTag);
	}
}

/**
 * @description: 判断卡号是否有效
 * @param:
 * @return: true = 有效,false = 无效
 */
/**
 * @description: 检查 idTag 是否在有效期内
 * @param {const char *} idTag - 待检查的 idTag
 * @return {bool} true = 在有效期内，false = 不在有效期内或其他错误
 */
bool ocpp_localAuthorization_List_isValid(const char *idTag)
{
	if (ocpp_AL == NULL || idTag == NULL)
	{
		printf("Database not initialized or invalid parameters\n");
		return false; // 处理错误情况，例如数据库未初始化或传入了无效的参数
	}

	char sql[512];
	snprintf(sql, sizeof(sql), "SELECT status, expiryDate FROM AuthorizationList WHERE IdTag = ?;");

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(ocpp_AL, sql, -1, &stmt, NULL);
	if (rc != SQLITE_OK)
	{
		printf("Error preparing SQL: %s\n", sqlite3_errmsg(ocpp_AL));
		return false; // 处理数据库查询准备失败的情况
	}

	sqlite3_bind_text(stmt, 1, idTag, -1, SQLITE_STATIC);

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW)
	{
		const char *status = (const char *)sqlite3_column_text(stmt, 0);
		const char *expiryDateStr = (const char *)sqlite3_column_text(stmt, 1);
		sqlite3_finalize(stmt);

		if (strcmp(status, "Accepted") == 0)
		{
			if (expiryDateStr == NULL || strlen(expiryDateStr) == 0)
			{
				return true; // 如果 expiryDate 为空，只要 status 是 "Accepted" 就返回 true
			}

			// 解析 expiryDate
			struct tm expiryDate;
			memset(&expiryDate, 0, sizeof(struct tm));
			strptime(expiryDateStr, "%Y-%m-%dT%H:%M:%S", &expiryDate);

			// 获取当前时间
			time_t currentTime = time(NULL);
			struct tm *localTime = localtime(&currentTime);

			// 比较时间
			if (mktime(&expiryDate) > mktime(localTime))
			{
				return true; // 在有效期内且 status 为 "Accepted"
			}
		}
	}

	sqlite3_finalize(stmt);
	return false; // 处理查询结果获取失败的情况或条件不满足的情况
}

/**
 * @description: 初始化本地认证
 * @param:
 * @return:
 */
int ocpp_localAuthorization_init()
{
	if (open_database(LOCALAUTH_DB_PATH) != 0)
	{
		return -1;
	}
	if (ocpp_localAuthorization_Cache_create_table() == -1)
	{
		printf("create localAuthorization_Cache fail\n");
		close_database();
		return -1;
	}
	if (ocpp_localAuthorization_Version_create_table() == -1)
	{
		printf("ocpp_localAuthorization_Version_create_table fail \n");
		close_database();
		return -1;
	}

	if (ocpp_localAuthorization_List_create_table() == -1)
	{
		printf("ocpp_localAuthorization_List_create_table  fail\n");
		close_database();
		return -1;
	}

	return 0;
}
