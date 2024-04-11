#ifndef _CONFIG_H_
#define _CONFIG_H_

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



int saveJsonToFile(const char *jsonStr, const char *filePath);
char *readJsonFromFile(const char *filePath);

#ifdef __cplusplus
}
#endif

#endif