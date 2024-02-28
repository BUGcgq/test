#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "log.h"

int main()
{
	// 定义并初始化配置结构体
    LogConfig config;
    memcpy(config.log_dir, "/mnt/mmcblk0p1/log", sizeof(config.log_dir));
    config.max_log_size = 3 * 1024 * 1024, // 3MB
    config.max_save_days = 10,         // 最大保存天数
    config.extra_delete = 5,           // 超天数删除最老的5条
    config.max_msg_num = 20,           // 消息队列最大写入条数
    config.log_level = LOG_LEVEL_DEBUG; // 设置为需要的日志级别
    init_log_system(&config);

	while (1)
	{
		// 写入日志
		LOG_ERROR("LOG_ERROR");
		LOG_WARNING("LOG_WARNING");
		LOG_INFO("LOG_INFO");
		LOG_DEBUG("LOG_DEBUG");
		sleep(10);
	}

	// 关闭日志系统
	close_log_system();

	return 0;
}
