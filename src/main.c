#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "log.h"

int main()
{
	init_log_system();

	while (1)
	{
		// 写入日志
		LOG_ERROR( "This is an informational LOG_ERROR");
		LOG_WARNING( "This is an informational LOG_WARNING");
		LOG_INFO( "This is an informational LOG_INFO");
		LOG_DEBUG( "This is an informational LOG_DEBUG");
		sleep(10);
	}

	// 关闭日志系统
	close_log_system();

	return 0;
}
