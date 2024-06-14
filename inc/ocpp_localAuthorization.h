#ifndef __OCPP_LOCAL_AUTHORIZATION__H__
#define __OCPP_LOCAL_AUTHORIZATION__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sqlite3.h>

#define LOCALAUTH_DB_PATH "/app/etc/localAuthorization.db"

typedef struct{
	char IdTag[21];                                        //卡号
	char status[32];                                   //卡状态
	char expiryDate[32];                                                           //有效期
	char parentIdTag[21];                                  //父卡号

}ocpp_localAuthorization_cache_record_t;

typedef struct{
    char IdTag[21];                                       //卡号
    char status[32];                                     //卡状态
    char  expiryDate[32];                                                          //有效期
    char parentIdTag[21];                                 //父卡号

}ocpp_localAuthorization_list_entry_t;


int ocpp_localAuthorization_Cache_clearCache(void);
int ocpp_localAuthorization_Cache_write(ocpp_localAuthorization_cache_record_t * cache_record);
bool ocpp_localAuthorization_Cache_isValid(const char *idTag);
bool ocpp_localAuthorization_Cache_search(const char * idTag);



int ocpp_localAuthorization_List_write(ocpp_localAuthorization_list_entry_t * list_entry);
int ocpp_localAuthorization_Version_setVersion(const char *tableName, int version);
int ocpp_localAuthorization_List_getListVersion(const char *tableName);
bool ocpp_localAuthorization_List_search(const char * idTag);
bool ocpp_localAuthorization_List_isValid(const char *idTag);

int ocpp_localAuthorization_init();


#ifdef __cplusplus
}
#endif

#endif