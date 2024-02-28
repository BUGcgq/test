#include "log.h"

static LogConfig global_log_config; // 静态全局变量
static int current_log_fd = -1;
static int msgid = -1;

int get_last_log_file_number(const char *log_dir_path, const char *date_time_str)
{
    int last_number = 0;

    // 拼接日志目录和日期字符串，获取日志目录下的文件
    DIR *dir = opendir(log_dir_path);
    if (!dir)
    {
        return -1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char *file_name = entry->d_name;
            if (strncmp(file_name, "log_", 4) == 0)
            {
                char *date_str = file_name + 4; // 跳过 "log_"
                if (strncmp(date_str, date_time_str, strlen(date_time_str)) == 0)
                {
                    // 匹配到日期部分
                    char *number_str = strchr(date_str, '_'); // 找到下一个下划线
                    if (number_str != NULL)
                    {
                        number_str++; // 跳过下划线
                        int number = atoi(number_str);
                        if (number > last_number)
                        {
                            last_number = number;
                        }
                    }
                }
            }
        }
    }

    closedir(dir);

    return last_number;
}

/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-02-27
 * 描述 : 根据日期创建日志目录和文件
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
void create_log_file(const char *dir_path, const char *date_time_str)
{
    char full_path[256]; // 假定路径长度不超过255字符
    snprintf(full_path, sizeof(full_path), "%s/%s", global_log_config.log_dir, date_time_str);
    if (access(full_path, F_OK) != 0)
    {
        mkdir(full_path, 0777);
    }
    // 创建文件路径
    // 获取当前日期时间的字符串形式，例如 "YYMMDD"
    int last_number = get_last_log_file_number(full_path, date_time_str);
    // 创建文件路径
    char file_name[100];
    snprintf(file_name, sizeof(file_name), "%s/log_%s_%03d.txt", full_path, date_time_str, last_number + 1);
    int fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        return;
    }

    current_log_fd = fd;
}
/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-02-28
 * 描述 :数量超过最大目录时全部清除日期最老文件夹
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
void check_and_delete_old_logs()
{
    DIR *dir = opendir(global_log_config.log_dir);
    if (!dir)
    {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    int exceed_count = 0; // 超过最大保存天数的目录数量

    // 遍历目录，统计超过最大保存天数的目录数量
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strlen(entry->d_name) == 8)
        {
            exceed_count++;
        }
    }

    closedir(dir);

    if (exceed_count < global_log_config.max_save_days)
    {
        return;
    }

    // 再次打开目录，删除超过最大保存天数的目录
    dir = opendir(global_log_config.log_dir);
    if (!dir)
    {
        perror("opendir");
        return;
    }

    struct tm *local_time = NULL;
    time_t now = time(NULL);
    local_time = localtime(&now);
    char date_time_str[15]; // YYMMDD
    strftime(date_time_str, sizeof(date_time_str), "%Y%m%d", local_time);
    int current_date = atoi(date_time_str);

    // 删除超过最大保存天数的目录
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && strlen(entry->d_name) == 8)
        {
            int dir_date = atoi(entry->d_name);
            int diff_days = abs(current_date - dir_date);
            if (diff_days > global_log_config.max_save_days - global_log_config.extra_delete)
            {
                char dir_path[256];
                snprintf(dir_path, sizeof(dir_path), "%s/%s", global_log_config.log_dir, entry->d_name);
                if (rmdir(dir_path) == -1)
                {
                    perror("rmdir");
                }
            }
        }
    }
    closedir(dir);
}
/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-03-01
 * 描述 : get_log_file_size
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
off_t get_log_file_size(int fd)
{
    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        return -1; // 返回-1表示获取失败
    }
    return st.st_size; // 返回文件大小
}
/**
 * *****************************************************************************
 * 作者 chenguiquan
 * 创建日期 : 2024-03-01
 * 描述 : 检查文件大小和目录日期
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
void check_and_create_new_log()
{
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char date_time_str[15]; // YYMMDD
    strftime(date_time_str, sizeof(date_time_str), "%Y%m%d", local_time);
    char full_path[256]; // 假定路径长度不超过255字符
    snprintf(full_path, sizeof(full_path), "%s/%s", global_log_config.log_dir, date_time_str);
    if (access(full_path, F_OK) != 0) // 检查日期文件夹是否存在
    {
        // 日期改变了
        if (current_log_fd != -1)
        {
            close_log_system();
        }
        create_log_file(global_log_config.log_dir, date_time_str); // 注意确保这个函数可以处理完整路径
    }
    else if (current_log_fd == -1) // 文件夹存在，但文件描述符为-1，需要打开或创建文件
    {
        create_log_file(full_path, date_time_str);
    }
    else
    {
        // 检查当前日志文件的大小
        if (get_log_file_size(current_log_fd) > global_log_config.max_log_size)
        {
            close_log_system();
            create_log_file(full_path, date_time_str);
        }
    }
}

void log_message(LogLevel level, const char *file, const char *func, int line, const char *format, ...)
{
    if (level > global_log_config.log_level)
    {
        return;
    }
    const char *level_str;
    const char *level_str_info;
    switch (level)
    {
    case LOG_LEVEL_ERROR:
        level_str = ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET;
        level_str_info = "ERROR";
        break;
    case LOG_LEVEL_WARNING:
        level_str = ANSI_COLOR_YELLOW "WARNING" ANSI_COLOR_RESET;
        level_str_info = "WARNING";
        break;
    case LOG_LEVEL_INFO:
        level_str = ANSI_COLOR_GREEN "INFO" ANSI_COLOR_RESET;
        level_str_info = "INFO";
        break;
    case LOG_LEVEL_DEBUG:
        level_str = ANSI_COLOR_WHITE "DEBUG" ANSI_COLOR_RESET;
        level_str_info = "DEBUG";
        break;
    default:
        level_str = "UNKNOWN";
        level_str_info = "UNKNOWN";
        break;
    }

    // 获取当前日期时间的字符串形式，例如 "YYMMDD"
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char formatted_time[20]; // 为了存储格式化后的时间字符串
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", local_time);

    char log_line[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(log_line, sizeof(log_line), format, args);
    va_end(args);
    char colored_msg[LOG_BUFFER_SIZE];
    snprintf(colored_msg, sizeof(colored_msg), "[%s][%s]--[%s]--[%d]  %s\n", level_str, formatted_time, file, line, log_line);
    // 打印到终端
    printf("%s", colored_msg);

    if (msgid != -1)
    {
        // 获取消息队列的属性
        struct msqid_ds msqid_ds_buf;
        if (msgctl(msgid, IPC_STAT, &msqid_ds_buf) == -1)
        {
            perror("msgctl IPC_STAT");
            return;
        }

        // 检查当前消息数量
        if (msqid_ds_buf.msg_qnum >= global_log_config.max_msg_num)
        {
            printf("Message queue is full. Cannot insert new message.\n");
            return;
        }
        struct logmsgbuf msgs;
        msgs.mtype = 1;
        snprintf(msgs.mtext, sizeof(msgs.mtext), "[%s][%s]--[%s]--[%d]  %s\n", level_str_info, formatted_time, file, line, log_line);
        // 发送消息
        if (msgsnd(msgid, &msgs, sizeof(msgs.mtext), 0) == -1)
        {
            perror("msgsnd");
        }
    }
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
void *log_thread_func(void *arg)
{
    struct logmsgbuf msgs;
    while (1)
    {
        if (msgrcv(msgid, &msgs, sizeof(msgs.mtext), 1, 0) == -1)
        {
            perror("msgrcv");
            sleep(1); // 等待1秒再重试
            continue; // 继续下一次循环
        }

        if (access(global_log_config.log_dir, F_OK) == 0)
        {
            check_and_create_new_log(); // 检查并创建新的日志文件，如果当前文件大小超过限制
            if (current_log_fd != -1)
            {
                // 如果当前有日志文件描述符，写入日志到文件
                dprintf(current_log_fd, "%s", msgs.mtext);
                fsync(current_log_fd); // 强制将缓冲区的数据写入磁盘
                check_and_delete_old_logs();
            }
        }
        else
        {
            if (current_log_fd != -1)
            {
                close_log_system(); // SD 卡未插入但有文件描述符仍然打开，关闭文件描述符
            }
        }
        // 清空缓冲区
        memset(&msgs, 0, sizeof(msgs));
    }
    return NULL;
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
void init_log_system(LogConfig *config)
{
    if (config == NULL)
    {
        fprintf(stderr, "Error: Invalid configuration provided.\n");
        return;
    }

    // 检查日志目录
    if ( strlen(config->log_dir) < 1 )
    {
        fprintf(stderr, "Error: Log directory or file prefix cannot be empty.\n");
        return;
    }

    // 检查日志文件最大大小是否合理
    if (config->max_log_size <= 0)
    {
        fprintf(stderr, "Error: Invalid maximum log size.\n");
        return;
    }

    // 检查最大保存天数和删除个数是否合理
    if (config->max_save_days <= 0 || config->extra_delete < 0)
    {
        fprintf(stderr, "Error: Invalid maximum save days or extra delete count.\n");
        return;
    }

    // 检查日志级别是否合理
    if (config->log_level < LOG_LEVEL_ERROR || config->log_level > LOG_LEVEL_DEBUG)
    {
        fprintf(stderr, "Error: Invalid log level.\n");
        return;
    }

    // 复制配置到全局变量
    memcpy(&global_log_config, config, sizeof(LogConfig));

    // 其他初始化操作
    key_t key = ftok("/tmp", 'a');
    if (key == -1)
    {
        perror("ftok");
        return;
    }

    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1)
    {
        perror("msgget");
        return;
    }
    // 检查和删除旧日志
    check_and_delete_old_logs();
    // 创建新的日志文件
    check_and_create_new_log();

    pthread_t log_thread;
    pthread_create(&log_thread, NULL, log_thread_func, NULL);
}

void close_log_system()
{
    if (current_log_fd != -1)
    {
        close(current_log_fd);
        current_log_fd = -1;
    }
}