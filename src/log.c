#include "log.h"

static LogConfig global_log_config;                                  // 静态全局变量
static pthread_mutex_t log_config_mutex = PTHREAD_MUTEX_INITIALIZER; // 初始化互斥锁
static int current_log_fd = -1;
static int msgid = -1;
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
static void get_curtime(char *buffer, size_t size)
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
static void get_timestamp(char *buffer, size_t size)
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
 * 函 数 名：将日志写入到磁盘
 * 描    述:
 *
 * 参    数: log_buffer - [写人的缓存冲区]
 * 参    数: len - [写入长度]
 * 参    数: fd - [文件描述符号]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static void write_buffered_log(const char *buffer, size_t len, int fd)
{
    if (fd != -1 && buffer && len > 0)
    {
        write(fd, buffer, len); // 强制将缓冲区的数据写入磁盘
        fsync(fd);
    }
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：Get the last log file number object
 * 描    述: 获取目录下最大文件编号
 *
 * 参    数: log_dir_path - [目录]
 * 参    数: date_time_str - [时间]
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static int get_last_log_file_number(const char *log_dir_path, const char *date_time_str)
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
 * 作    者: 陈贵全
 * 创建日期: 2024-03-05
 * 函 数 名：open_log_file
 * 描    述: 打开文件
 *
 * 参    数: full_path - [目录路径]
 * 参    数: date_time_str - [时间]
 * 参    数: last_number - [文件编号]
 * 返回类型：文件描述符
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static int open_log_file(const char *full_path, const char *date_time_str, int last_number)
{
    char file_name[100];
    snprintf(file_name, sizeof(file_name), "%s/log_%s_%03d.txt", full_path, date_time_str, last_number + 1);

    int fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
    {
        perror("open");
    }
    return fd;
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
static void create_log_file(LogLevel level)
{
    char date_time_str[9];
    get_timestamp(date_time_str, 9);
    char full_path[256]; // 假定路径长度不超过255字符
    if (level == LOG_LEVEL_ZERO)
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", global_log_config.zero_dir, date_time_str);
    }
    else
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", global_log_config.log_dir, date_time_str);
    }

    if (access(full_path, F_OK) != 0)
    {
        mkdir(full_path, 0777);
    }

    if (current_log_fd != -1)
    {
        close_log_system();
    }

    // 获取当前日期时间的字符串形式，例如 "YYMMDD"
    int last_number = get_last_log_file_number(full_path, date_time_str);

    current_log_fd = open_log_file(full_path, date_time_str, last_number);
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
static void check_and_delete_old_logs(char *full_path)
{
    DIR *dir = opendir(full_path);
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
    dir = opendir(full_path);
    if (!dir)
    {
        perror("opendir");
        return;
    }

    struct tm *local_time = NULL;
    time_t now = time(NULL);
    local_time = localtime(&now);
    char date_time_str[9]; // YYMMDD
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
                snprintf(dir_path, sizeof(dir_path), "%s/%s", full_path, entry->d_name);
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
 * 描述 : 获取文件大小
 *
 * 特殊说明：无
 * 修改记录:无
 * *****************************************************************************
 */
static off_t get_log_file_size(int fd)
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
static void check_and_create_new_log(LogLevel level, char *dir_path)
{
    char date_time_str[9];
    get_timestamp(date_time_str, 9);
    char full_path[256]; // 假定路径长度不超过255字符
    snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, date_time_str);
    if (access(full_path, F_OK) != 0) // 检查日期文件夹是否存在
    {
        if (current_log_fd != -1)
        {
            close_log_system();
        }
        create_log_file(level); // 注意确保这个函数可以处理完整路径
    }
    else if (current_log_fd == -1) // 文件夹存在，但文件描述符为-1，需要打开或创建文件
    {
        create_log_file(level);
    }
    else
    {
        // 检查当前日志文件的大小
        if (get_log_file_size(current_log_fd) > global_log_config.max_log_size)
        {
            close_log_system();
            create_log_file(level);
        }
    }
}
/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-07
 * 函 数 名：send_log_msg
 * 描    述: 日志插入到消息队列
 *
 * 参    数: msgid - [参数说明]
 * 参    数: msgs - [参数说明]
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
// 发送日志消息函数
static void send_log_msg(int msgid, struct logmsgbuf msgs)
{
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

        // 发送消息
        if (msgsnd(msgid, &msgs, sizeof(msgs.mtext), 0) == -1)
        {
            perror("msgsnd");
        }
    }
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
    if (level > global_log_config.log_level)
    {
        return;
    }
    const char *level_str;
    const char *level_str_info;
    switch (level)
    {
    case LOG_LEVEL_ZERO:
        level_str = ANSI_COLOR_RED "ZERO" ANSI_COLOR_RESET;
        level_str_info = "ZERO";
        if (global_log_config.log_level != 0)
        {
            return;
        }
        break;
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
    char formatted_time[32]; // 为了存储格式化后的时间字符串
    get_curtime(formatted_time, 32);

    char log_line[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(log_line, sizeof(log_line), format, args);
    va_end(args);

    // 打印到终端
    if (global_log_config.log_level != LOG_LEVEL_ZERO)
    {
        char colored_msg[LOG_BUFFER_SIZE];
        snprintf(colored_msg, sizeof(colored_msg), "[%s][%s]--[%s]--[%d]  %s\n", level_str, formatted_time, file, line, log_line);
        printf("%s", colored_msg);
    }

    struct logmsgbuf msgs;
    msgs.mtype = 1;
    snprintf(msgs.mtext, sizeof(msgs.mtext), "[%s][%s]--[%s]--[%d]  %s\n", level_str_info, formatted_time, file, line, log_line);
    send_log_msg(msgid, msgs);
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-07
 * 函 数 名：is_logDir_accessible
 * 描    述: 判断目录是否可用
 * 
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
static int is_log_directory_accessible(void)
{
    // 当日志级别为LOG_LEVEL_ZERO且zero_dir目录存在
    if (global_log_config.log_level == LOG_LEVEL_ZERO && access(global_log_config.zero_dir, F_OK) == 0)
    {
        return 1;
    }
    // 当日志级别不是LOG_LEVEL_ZERO且log_dir目录存在
    else if (global_log_config.log_level != LOG_LEVEL_ZERO && access(global_log_config.log_dir, F_OK) == 0)
    {
        return 1;
    }
    // 其他情况，目录不可访问
    return 0;
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
    char log_buffer[LOG_BUFFER_SIZE];                  // 局部缓冲区
    int log_buffer_pos = 0, message_len;               // 缓冲区当前位置
    memset(log_buffer, 0, LOG_BUFFER_SIZE);            // 初始化缓冲区
    time_t last_write_time, current_time = time(NULL); // 初始化上次写入时间
    while (1)
    {
        current_time = time(NULL);
        if (current_time - last_write_time >= global_log_config.write_msg_interval && log_buffer_pos > 0)
        {
            write_buffered_log(log_buffer, log_buffer_pos, current_log_fd);
            log_buffer_pos = 0;
            memset(log_buffer, 0, LOG_BUFFER_SIZE);
            last_write_time = current_time; // 更新上次写入时间
        }

        if (msgrcv(msgid, &msgs, sizeof(msgs.mtext), 1, 0) == -1)
        {
            perror("msgrcv");
            sleep(1); // 等待1秒再重试
            continue; // 继续下一次循环
        }

        if (is_log_directory_accessible())
        {
            check_and_create_new_log(global_log_config.log_level, (global_log_config.log_level == LOG_LEVEL_ZERO) ? global_log_config.zero_dir : global_log_config.log_dir);
            message_len = strlen(msgs.mtext);
            if (log_buffer_pos + message_len >= LOG_BUFFER_SIZE)
            {
                write_buffered_log(log_buffer, log_buffer_pos, current_log_fd);
                log_buffer_pos = 0;                     // 重置缓冲区位置
                memset(log_buffer, 0, LOG_BUFFER_SIZE); // 清空缓冲区
            }

            memcpy(log_buffer + log_buffer_pos, msgs.mtext, message_len);
            log_buffer_pos += message_len;

            check_and_delete_old_logs((global_log_config.log_level == LOG_LEVEL_ZERO) ? global_log_config.zero_dir : global_log_config.log_dir);
        }
        else
        {
            close_log_system(); // SD 卡未插入但有文件描述符仍然打开，关闭文件描述符
        }

        // 清空缓冲区
        memset(&msgs, 0, sizeof(msgs));
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
    pthread_mutex_lock(&log_config_mutex);
    global_log_config.log_level = level;
    if (level == LOG_LEVEL_ZERO)
    {
        global_log_config.max_log_size = 20 * 1024 * 1024; // 20MB
        global_log_config.max_msg_num = 40;
    }
    else
    {
        global_log_config.max_log_size = 3 * 1024 * 1024; // 3MB
        global_log_config.max_msg_num = 20;
    }

    pthread_mutex_unlock(&log_config_mutex); // 解锁
    create_log_file(level);
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
    if (strlen(config->log_dir) < 1 || strlen(config->zero_dir) < 1)
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
    if (config->log_level < LOG_LEVEL_ZERO || config->log_level >= LOG_LEVEL_MAX)
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

    pthread_t log_thread;
    pthread_create(&log_thread, NULL, log_thread_func, NULL);
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-07
 * 函 数 名：close_log_system
 * 描    述: 关闭日志
 *
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
void close_log_system()
{
    if (current_log_fd != -1)
    {
        close(current_log_fd);
        current_log_fd = -1;
    }
}

// int main()
// {
// 	LogConfig config;
// 	memcpy(config.log_dir, "/mnt/mmcblk0p1/log", sizeof(config.log_dir));
// 	memcpy(config.zero_dir, "/mnt/mmcblk0p1/can", sizeof(config.zero_dir));
// 	config.max_log_size = 3 * 1024 * 1024, // 3MB
// 	config.max_save_days = 10,		   // 最大保存天数
// 	config.extra_delete = 5,		   // 超天数删除最老的5条
// 	config.max_msg_num = 20,		   // 消息队列最大写入条数
// 	config.write_msg_interval = 10;     //写缓冲区时间
// 	config.log_level = LOG_LEVEL_INFO; // 设置为需要的日志级别

// 	// 写入日志
// 	init_log_system(&config);
// 	// while (1)
// 	// {
// 	// 	LOG_INFO("LOG_INFO");
// 	// 	sleep(10);
// 	// }

// 	// 关闭日志系统

// 	if(ocpp_download_file("ftp://inc02:za%40g0Pg6J@14.29.244.45:21/inc/kltt/KE-6720-M.bin.tar.bz2","/app/core/KE-6720-M.bin.tar.bz2",1) == 0)
// 	LOG_INFO("下载成功");
// 	else
// 	LOG_INFO("下载失败");
// 	close_log_system();
// 	return 0;
// }