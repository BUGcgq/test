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
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdarg.h>


#define LOG_BUFFER_SIZE 1024                // 缓冲区大小
#define LOG_FILE_MAX_SIZE 3 * 1024 * 1024   // 3MB
#define LOG_FILE_DIR_PATH "/app/core/log"
#define LOG_MAX_SAVE_DAYS 10  // 超最大保存天数要保留的个数
#define LOG_MAX_DEL_FILES 5   // 最大保存天数
#define WRITE_MSG_INTERVAL 10 // 写缓存区的时间间隔

typedef enum
{
    LOG_LEVEL_ZERO = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX
} LogLevel;


void init_log_system();
void set_log_level(LogLevel level);
void log_message(LogLevel level, const char *file, const char *func, int line, const char *format, ...);

#define LOG_CAN(message, ...) log_message(LOG_LEVEL_ZERO, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) log_message(LOG_LEVEL_ERROR, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOG_WARNING(message, ...) log_message(LOG_LEVEL_WARNING, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOG_INFO(message, ...) log_message(LOG_LEVEL_INFO, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)
#define LOG_DEBUG(message, ...) log_message(LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, message, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif