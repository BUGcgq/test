#ifndef __FTP__H__
#define __FTP__H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "curl/curl.h"



int ocpp_download_file(const char *url, const char *local_file_path, int mode);
int ocpp_upload_file(const char *url, const char *local_file_path, int mode);



#ifdef __cplusplus
}
#endif

#endif