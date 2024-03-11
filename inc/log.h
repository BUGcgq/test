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
#define LOG_BUFFER_SIZE 1024 // 缓冲区大小
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_WHITE "\x1b[37m"

typedef enum
{
    LOG_LEVEL_ZERO = 0,
    LOG_LEVEL_ERROR ,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX
} LogLevel;

// 定义消息结构体
struct logmsgbuf
{
    long mtype;
    char mtext[LOG_BUFFER_SIZE];
};

typedef struct
{
    char log_dir[256];        // 日志目录
    char zero_dir[256];        // can目录
    int max_log_size;         // 单个日志文件的最大大小（字节）
    int max_save_days;        // 最大保存天数
    int extra_delete;         // 删除旧文件的个数
    int max_msg_num;          // 最大消息队列数
    int write_msg_interval;    // 写缓存区的时间间隔  
    LogLevel log_level;       // 日志级别
} LogConfig;

void init_log_system(LogConfig *config);
void close_log_system();
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