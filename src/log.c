#include "log.h"

static const char *LOG_LEVEL_STRING[] =
    {
        "ZERO",
        "ERROR",
        "WARNING",
        "INFO",
        "DEBUG"};

typedef struct
{
    LogLevel logLevel;
    FILE *fp;
    int logOk;
    pthread_mutex_t logMutex;
} LogConfig;

static LogConfig g_logCfg = {
    .logLevel = LOG_LEVEL_DEBUG, // 初始化 logLevel 为 LOG_LEVEL_INFO
    .fp = NULL,                  // 初始化 fp 为 NULL
    .logOk = 0,
    .logMutex = PTHREAD_MUTEX_INITIALIZER // 初始化 logMutex
};
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-07
 * 函 数 名：Get the curtime object
 * 描    述: 获取当前时间    时间格式：2011-11-15 12:47:34:888
 *
 * 参    数: buffer - [参数说明]
 * 参    数: size - [参数说明]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static void LOG_getCurtime(char *buffer, size_t size)
{
    if (buffer == NULL || size == 0)
    {
        return;
    }
    time_t t;
    struct tm *p;
    struct timeval tv;
    int len;
    int millsec;

    t = time(NULL);
    p = localtime(&t);

    gettimeofday(&tv, NULL);
    millsec = (int)(tv.tv_usec / 1000);

    len = snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                   p->tm_year + 1900, p->tm_mon + 1,
                   p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, millsec);

    if (len >= 0 && len < size)
    {
        buffer[len] = '\0'; // 确保字符串以 NULL 结尾
    }
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-07
 * 函 数 名：get_timestamp
 * 描    述: 获取当前日期    日期格式：格式化日期：YYMMDD
 *
 * 参    数: buffer - [参数说明]
 * 参    数: size - [参数说明]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static void LOG_getTimestamp(char *buffer, size_t size)
{
    if (buffer == NULL || size < 9)
    {
        fprintf(stderr, "Invalid buffer or buffer size too small.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(buffer, size, "%Y%m%d", local_time);
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：open_log_file
 * 描    述: 打开文件
 *
 * 参    数: date_time_str - [时间]
 * 参    数: mode  0 - 清空打开 ,1 - 追加打开
 * 返回类型：文件描述符
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
FILE *LOG_openFile(const char *dirPath, const char *dateTime, int mode)
{
    char file_name[100];
    snprintf(file_name, sizeof(file_name), "%s/%s.log", dirPath, dateTime);
    const char *mode_str = (mode == 0) ? "w" : "a"; // 根据 mode 选择写入方式

    FILE *fp = fopen(file_name, mode_str);
    if (fp != NULL)
    {
        setvbuf(fp, NULL, _IOFBF, LOG_BUFFER_SIZE);
    }

    return fp;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-06-04
 * 函 数 名：LOG_getDirNum
 * 描    述: 获取目录下所以目录数量
 *
 * 参    数: path - [目录]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int LOG_getFileNum(const char *path)
{
    if (path == NULL)
    {
        fprintf(stderr, "Invalid path\n");
        return -1;
    }

    DIR *dp = opendir(path);
    if (dp == NULL)
    {
        perror("opendir");
        return -1;
    }

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            const char *ext = strrchr(entry->d_name, '.');
            if (ext && strcmp(ext, ".log") == 0)
            {
                count++;
            }
        }
    }

    closedir(dp);
    return count;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-06-04
 * 函 数 名：LOG_DelOldDirs
 * 描    述: 删除旧天数的目录
 *
 * 参    数: path - [目录]
 * 参    数: currentDate - [当前时间]
 * 参    数: maxDays - [最大天数]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int LOG_DelOldFiles(const char *path, const char *currentDateStr)
{
    struct dirent *entry;
    DIR *dp = opendir(path);
    if (dp == NULL)
    {
        perror("opendir");
        return -1;
    }

    int currentDate = atoi(currentDateStr) - LOG_MAX_DEL_FILES;
    if (currentDate == 0)
    {
        fprintf(stderr, "Invalid current date: %s\n", currentDateStr);
        closedir(dp);
        return -1;
    }

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            const char *filename = entry->d_name;
            size_t len = strlen(filename);
            if (len == 12 && strncmp(filename + 8, ".log", 4) == 0)
            {
                char fileDateStr[9];
                strncpy(fileDateStr, filename, 8);
                fileDateStr[8] = '\0';
                int fileDate = atoi(fileDateStr);
                if (fileDate < currentDate)
                {
                    char filePath[1024];
                    snprintf(filePath, sizeof(filePath), "%s/%s", path, filename);
                    if (remove(filePath) == 0)
                    {
                        printf("Deleted file: %s\n", filePath);
                    }
                    else
                    {
                        perror("remove");
                    }
                }
            }
        }
    }

    closedir(dp);
    return 0;
}

/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-03-01
 * 描述 : 获取文件大小
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
off_t LOG_getFileSize(FILE *fp)
{
    struct stat st;
    if (fstat(fileno(fp), &st) == -1)
    {
        perror("fstat");
        return -1; // 返回-1表示获取失败
    }
    return st.st_size; // 返回文件大小
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：log_message
 * 描    述:根据日志等级打印到终端或插入到消息队列
 *
 * 参    数: level - [日志等级]
 * 参    数: file - [文件名]
 * 参    数: func - [函数名字]
 * 参    数: line - [行号]
 * 参    数: format - [参数说明]
 * 参    数: ... - [参数说明]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void log_message(LogLevel level, const char *file, const char *func, int line, const char *format, ...)
{

    if (level > g_logCfg.logLevel)
    {
        return;
    }

    char formatted_time[32];
    LOG_getCurtime(formatted_time, sizeof(formatted_time));

    char log_line[LOG_BUFFER_SIZE];
    char msg[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(log_line, sizeof(log_line), format, args);
    va_end(args);

    snprintf(msg, sizeof(msg), "[%s][%s]--[%s]--[%d]  %s\n", LOG_LEVEL_STRING[level], formatted_time, file, line, log_line);

    // 打印到终端（排除日志级别为 ZERO 的情况）
    if (level != LOG_LEVEL_ZERO)
    {
        printf("%s", msg);
    }

    pthread_mutex_lock(&g_logCfg.logMutex);
    if (g_logCfg.fp != NULL && g_logCfg.logOk)
    {
        fprintf(g_logCfg.fp, "%s", msg);
    }
    pthread_mutex_unlock(&g_logCfg.logMutex);
}

/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-02-28
 * 描述 : 日志控制线程
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
void *flush_thread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&g_logCfg.logMutex);
        if (access(LOG_FILE_DIR_PATH, F_OK) == 0)
        {
            char date_time_str[9];
            LOG_getTimestamp(date_time_str, 9);
            if (g_logCfg.fp != NULL)
            {
                if (LOG_getFileNum(LOG_FILE_DIR_PATH) > LOG_MAX_SAVE_DAYS)
                {
                    LOG_DelOldFiles(LOG_FILE_DIR_PATH, date_time_str);
                }

                if (LOG_getFileSize(g_logCfg.fp) > LOG_FILE_MAX_SIZE)
                {
                    fclose(g_logCfg.fp);
                    g_logCfg.fp = LOG_openFile(LOG_FILE_DIR_PATH, date_time_str, 0);
                }
            }
            else
            {
                g_logCfg.fp = LOG_openFile(LOG_FILE_DIR_PATH, date_time_str, 1);
            }

            fflush(g_logCfg.fp);
        }
        else
        {
            if (g_logCfg.fp != NULL)
            {
                fclose(g_logCfg.fp);
                g_logCfg.fp = NULL;
            }
        }
        pthread_mutex_unlock(&g_logCfg.logMutex);
        sleep(WRITE_MSG_INTERVAL);
    }
    return NULL;
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-07
 * 函 数 名：set_log_level
 * 描    述: 设置日志等级
 *
 * 参    数: level - [日志级别]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void set_log_level(LogLevel level)
{
    pthread_mutex_lock(&g_logCfg.logMutex);
    g_logCfg.logLevel = level;
    pthread_mutex_unlock(&g_logCfg.logMutex);
}

/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-02-28
 * 描述 : 初始化日志系统
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
void init_log_system()
{
    char date[9];
    LOG_getTimestamp(date, sizeof(date));
    g_logCfg.fp = LOG_openFile(LOG_FILE_DIR_PATH, date, 1);
    pthread_t tid;
    if (pthread_create(&tid, NULL, flush_thread, NULL) != -1)
    {
        g_logCfg.logOk = 1;
        pthread_detach(tid); // 线程分离，不需要等待它结束
    }
}
