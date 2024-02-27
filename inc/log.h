#ifndef __LOG__H__
#define __LOG__H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

#define SD_CARD_PATH "/mnt/mmcblk0p1"       // SD卡路径
#define LOG_DIR "log"                       // 日志目录
#define MAX_SAVE_DAYS 10                    // 最大保存天数
#define DELETE_LOG_FILES 5                  // 超过保存天数时删除的日志文件数量
#define MAX_LOG_SIZE (5 * 1024 * 1024)      // 每个日志文件的最大大小，这里设置为5MB
#define LOG_LEVEL_THRESHOLD LOG_LEVEL_WARNING // 日志记录的最低级别

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_WHITE "\x1b[37m"

typedef enum
{
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

void init_log_system();
void close_log_system();
void log_message(LogLevel level, const char *message, const char *file, const char *func, int line);


#define LOG_ERROR(message) log_message(LOG_LEVEL_ERROR, message, __FILE__, __func__, __LINE__)
#define LOG_WARNING(message) log_message(LOG_LEVEL_WARNING, message, __FILE__, __func__, __LINE__)
#define LOG_INFO(message) log_message(LOG_LEVEL_INFO, message, __FILE__, __func__, __LINE__)
#define LOG_DEBUG(message) log_message(LOG_LEVEL_DEBUG, message, __FILE__, __func__, __LINE__)


#ifdef __cplusplus
}
#endif

#endif