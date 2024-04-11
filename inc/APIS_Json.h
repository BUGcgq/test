#ifndef _APIS_JSON_H_
#define _APIS_JSON_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <dirent.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

/* Macros for creating things quickly. */
#define cJSON_AddNullToObject(object, name) cJSON_AddItemToObject(object, name, cJSON_CreateNull())
#define cJSON_AddTrueToObject(object, name) cJSON_AddItemToObject(object, name, cJSON_CreateTrue())
#define cJSON_AddFalseToObject(object, name) cJSON_AddItemToObject(object, name, cJSON_CreateFalse())
#define cJSON_AddBoolToObject(object, name, b) cJSON_AddItemToObject(object, name, cJSON_CreateBool(b))
#define cJSON_AddNumberToObject(object, name, n) cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))
#define cJSON_AddStringToObject(object, name, s) cJSON_AddItemToObject(object, name, cJSON_CreateString(s))
#define cJSON_AddRawToObject(object, name, s) cJSON_AddItemToObject(object, name, cJSON_CreateRaw(s))
#define cJSON_AddDoubleberToObject(object, name, n) cJSON_AddItemToObject(object, name, cJSON_CreateNumber(n))

#define cJSON_GetObjectToNumber(jsonObj, key, dest) \
    do { \
        cJSON *val = cJSON_GetObjectItem(jsonObj, key); \
        if (val != NULL && cJSON_IsNumber(val)) { \
            dest = val->valueint; \
        } \
    } while(0)

#define cJSON_GetObjectToDouble(jsonObj, key, dest) \
    do { \
        cJSON *val = cJSON_GetObjectItem(jsonObj, key); \
        if (val != NULL && cJSON_IsNumber(val)) { \
            dest = val->valuedouble; \
        } \
    } while(0)

#define cJSON_GetObjectToString(jsonObj, key, dest, destSize) \
    do { \
        cJSON *val = cJSON_GetObjectItem(jsonObj, key); \
        if (val != NULL && cJSON_IsString(val) && val->valuestring != NULL) { \
            strncpy(dest, val->valuestring, destSize - 1); \
            dest[destSize - 1] = '\0'; \
        } \
    } while(0)

#define cJSON_GetObjectToBool(jsonObj, key, dest) \
    do { \
        cJSON *val = cJSON_GetObjectItem(jsonObj, key); \
        if (val != NULL && cJSON_IsBool(val)) { \
            dest = val->valueint != 0 ? true : false; \
        } \
    } while(0)


int saveJsonToFile(const char *jsonStr, const char *filePath);
char *readJsonFromFile(const char *filePath);

#ifdef __cplusplus
}
#endif

#endif