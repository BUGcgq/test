#include "log.h"

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static int current_log_fd = -1;    // 当前日志文件的文件描述符
static char current_log_path[100]; // 当前日志文件的路径

int get_last_log_file_number(const char *log_dir_path, const char *date_time_str)
{
    int last_number = 0;

    // 拼接日志目录和日期字符串，获取日志目录下的文件
    DIR *dir = opendir(log_dir_path);
    if (!dir)
    {
        perror("opendir");
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
    // 创建目录
    char dir_name[100];
    snprintf(dir_name, sizeof(dir_name), "%s/%s/%s", SD_CARD_PATH, LOG_DIR, dir_path);
    if (access(dir_name, F_OK) == -1)
    {
        // 目录不存在，创建目录
        mkdir(dir_name, 0777);
    }

    // 获取当前日期时间的字符串形式，例如 "YYMMDD"
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char current_date_time_str[15];
    strftime(current_date_time_str, sizeof(current_date_time_str), "%y%m%d", local_time);

    // 如果当前日期与传入的日期不同，或者当前没有日志文件，或者文件路径与当前路径不同，创建新的日志文件
    if (strcmp(current_date_time_str, date_time_str) != 0 || current_log_fd == -1 || strstr(current_log_path, dir_name) == NULL)
    {
        // 获取最后一个日志文件的编号
        int last_number = get_last_log_file_number(dir_name, date_time_str);
        // 创建文件路径
        char file_name[100];
        snprintf(file_name, sizeof(file_name), "%s/log_%s_%03d.txt", dir_name, date_time_str, last_number + 1);

        int fd = open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1)
        {
            perror("open");
            return;
        }

        if (current_log_fd != -1)
        {
            // 如果当前有日志文件，关闭当前日志文件
            close(current_log_fd);
        }

        current_log_fd = fd;
        snprintf(current_log_path, sizeof(current_log_path), "%s", file_name);
    }
}

void check_and_delete_old_logs()
{
    char log_path[100];
    snprintf(log_path, sizeof(log_path), "%s/%s", SD_CARD_PATH, LOG_DIR);

    DIR *dir = opendir(log_path);
    if (!dir)
    {
        perror("opendir");
        return;
    }

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char oldest_date[15]; // 最老的日期
    strftime(oldest_date, sizeof(oldest_date), "%y%m%d", local_time);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR || strstr(entry->d_name, "log_") == NULL)
        {
            // 忽略非日志文件和文件夹
            continue;
        }

        char file_path[100];
        snprintf(file_path, sizeof(file_path), "%s/%s", log_path, entry->d_name);

        struct stat file_stat;
        if (stat(file_path, &file_stat) == -1)
        {
            perror("stat");
            continue;
        }

        // 获取文件的修改时间
        time_t file_time = file_stat.st_mtime;
        struct tm *file_local_time = localtime(&file_time);
        char file_date[15];
        strftime(file_date, sizeof(file_date), "%y%m%d", file_local_time);

        // 计算文件的保存天数
        int diff_days = (now - file_time) / (24 * 3600);

        if (diff_days > MAX_SAVE_DAYS)
        {
            // 超过最大保存天数，删除文件
            if (remove(file_path) == -1)
            {
                perror("remove");
            }
        }
        else if (strcmp(file_date, oldest_date) < 0)
        {
            // 更新最老的日期
            strcpy(oldest_date, file_date);
        }
    }

    // 关闭目录
    closedir(dir);

    // 删除最老日期的文件，保留MAX_LOG_FILES个文件
    int log_count = 0;
    dir = opendir(log_path);
    if (!dir)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG && strstr(entry->d_name, oldest_date) != NULL)
        {
            // 匹配到最老日期的日志文件
            char file_path[100];
            snprintf(file_path, sizeof(file_path), "%s/%s", log_path, entry->d_name);
            if (log_count >= DELETE_LOG_FILES)
            {
                // 超过删除日志文件数量，删除文件
                if (remove(file_path) == -1)
                {
                    perror("remove");
                }
            }
            else
            {
                log_count++;
            }
        }
    }

    // 关闭目录
    closedir(dir);
}

void check_and_create_new_log()
{
    if (current_log_fd == -1 && access(SD_CARD_PATH, F_OK) == 0)
    {
        // 如果当前没有日志文件，创建新的日志文件
        time_t now = time(NULL);
        struct tm *local_time = localtime(&now);
        char date_time_str[15]; // YYMMDD
        strftime(date_time_str, sizeof(date_time_str), "%y%m%d", local_time);

        char dir_name[20];
        snprintf(dir_name, sizeof(dir_name), "%s", date_time_str);
        create_log_file(dir_name, date_time_str);
    }
    else
    {
        // 如果当前有日志文件，检查文件大小
        struct stat st;
        if (fstat(current_log_fd, &st) == -1)
        {
            perror("fstat");
            return;
        }

        if (st.st_size > MAX_LOG_SIZE)
        {
            // 文件大小超过限制，关闭当前文件
            close(current_log_fd);
            current_log_fd = -1;

            // 重新创建日志文件
            check_and_create_new_log();
        }
    }
}

void log_message(LogLevel level, const char *message, const char *file, const char *func, int line)
{
    if (level > LOG_LEVEL_THRESHOLD)
    {
        return;
    }
    const char *level_str;

    switch (level)
    {
    case LOG_LEVEL_ERROR:
        level_str = ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET;
        break;
    case LOG_LEVEL_WARNING:
        level_str = ANSI_COLOR_YELLOW "WARNING" ANSI_COLOR_RESET;
        break;
    case LOG_LEVEL_INFO:
        level_str = ANSI_COLOR_GREEN "INFO" ANSI_COLOR_RESET;
        break;
    case LOG_LEVEL_DEBUG:
        level_str = ANSI_COLOR_WHITE "DEBUG" ANSI_COLOR_RESET;
        break;
    default:
        level_str = "UNKNOWN";
        break;
    }

    // 加锁
    pthread_mutex_lock(&log_mutex);

    // 获取当前日期时间的字符串形式，例如 "YYMMDD"
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char date_time_str[15];
    strftime(date_time_str, sizeof(date_time_str), "%y%m%d", local_time);
    char formatted_time[20]; // 为了存储格式化后的时间字符串
    strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", local_time);
    // 打印日志到终端
    printf("[%s][%s]--[%s]--[%d]  %s\n", level_str, formatted_time, file, line, message);
    if (access(SD_CARD_PATH, F_OK) == 0)
    {
        if (current_log_fd == -1 || access(current_log_path, F_OK) != 0)
        {
            // 如果当前没有日志文件或者文件已失效，创建新的日志文件
            char dir_name[20];
            snprintf(dir_name, sizeof(dir_name), "%s", date_time_str);
            create_log_file(dir_name, date_time_str);
        }
    }
    else
    {
        if (current_log_fd != -1)
        {
            // SD 卡未插入但有文件描述符仍然打开，关闭文件描述符
            close_log_system();
        }
        // SD 卡未插入，直接返回
        pthread_mutex_unlock(&log_mutex);
        return;
    }

    if (current_log_fd != -1)
    {
        // 如果当前有日志文件描述符，写入日志到文件
        dprintf(current_log_fd, "[%s][%s]--[%s]--[%d]  %s\n", level_str, formatted_time, file, line, message);
        fsync(current_log_fd); // 强制将缓冲区的数据写入磁盘
    }

    // 解锁
    pthread_mutex_unlock(&log_mutex);

    if (current_log_fd != -1)
    {
        // 检查并创建新的日志文件，如果当前文件大小超过限制
        check_and_create_new_log();
    }
}

void init_log_system()
{
    // 创建日志目录
    char log_dir_path[100];
    snprintf(log_dir_path, sizeof(log_dir_path), "%s/%s", SD_CARD_PATH, LOG_DIR);
    mkdir(log_dir_path, 0777);

    // 检查和删除旧日志
    check_and_delete_old_logs();

    // 创建新的日志文件
    check_and_create_new_log();
}

void close_log_system()
{
    if (current_log_fd != -1)
    {
        close(current_log_fd);
        current_log_fd = -1;
    }
}