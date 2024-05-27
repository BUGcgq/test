#ifndef __FTP__H__
#define __FTP__H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "curl/curl.h"



int ET_FTP_downloadFile(const char *url, const char *local_file_path, int mode);
int ET_FTP_uploadFile(const char *url, const char *local_file_path, int mode);



#ifdef __cplusplus
}
#endif

#endif