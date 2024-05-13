/************************************************************
Copyright (C) 深圳英可瑞科技开发有限公司
文 件 名：ToolFunc.c
版    本：1.00
创建日期：2020-09-10
作    者：郭数理
功能描述：工具函数实现，实现互斥信号量，休眠等函数

函数列表：

修改记录：
	作者      日期          版本       修改内容
	郭数理     2020-09-10  1.00     创建
**************************************************************/


#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include "ToolFunc.h"


union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};


/*************************************************************
函数名称: inc_mutex_sem_delete
函数功能: 删除互斥信号量
输入参数: sem_id -- 信号量标识符
输出参数: 无
返回值  ：无
**************************************************************/
void inc_mutex_sem_delete(int sem_id)
{
	union semun sem_union;
	sem_union.val = 0;
	
	if (sem_id < 0)
		return;

	semctl(sem_id, 0, IPC_RMID, sem_union);
}

/*************************************************************
函数名称: inc_mutex_sem_create
函数功能: 创建互斥信号量
输入参数: key -- 关键字
输出参数: 无
返回值  ：信号量ID，错误返-1
**************************************************************/
int inc_mutex_sem_create(key_t key)
{
	int sem_id;
	union semun sem_union;
	sem_union.val = 1;

	sem_id = semget(key, 1, IPC_CREAT | 0666);
	if (sem_id >= 0)
	{		
		if(semctl(sem_id, 0, SETVAL, sem_union) < 0)
		{
			inc_mutex_sem_delete(sem_id);
			return -1;
		}
	}

	return sem_id;
}

/*************************************************************
函数名称: inc_mutex_sem_open
函数功能: 获取其它进程创建的互斥信号量
输入参数: key -- 关键字
输出参数: 无
返回值  ：信号量ID，错误返-1
**************************************************************/
int inc_mutex_sem_open(key_t key)
{
	return semget(key, 0, 0);
}

/*************************************************************
函数名称: inc_mutex_sem_p
函数功能: 等待互斥信号量
输入参数: sem_id -- 信号量标识符
输出参数: 无
返回值  ：信号量ID，错误返-1
**************************************************************/
int inc_mutex_sem_p(int sem_id)
{
    struct sembuf sops={0, -1, SEM_UNDO};//IPC_NOWAIT, SEM_UNDO

	if (sem_id < 0)
		return sem_id;
    return (semop(sem_id, &sops, 1));
}

/*************************************************************
函数名称: inc_mutex_sem_v
函数功能: 释放互斥信号量
输入参数: sem_id -- 信号量标识符
输出参数: 无
返回值  ：信号量ID，错误返-1
**************************************************************/
int inc_mutex_sem_v(int sem_id)
{
    struct sembuf sops={0, 1, SEM_UNDO};//IPC_NOWAIT, SEM_UNDO

	if (sem_id < 0)
		return sem_id;
    return (semop(sem_id, &sops, 1));
}

/*************************************************************
函数名称: inc_shm_create
函数功能: 创建共享内存
输入参数: key      -- 关键字
          size -- 内存的大小
输出参数: 无
返回值  ：共享内存ID，错误返-1
**************************************************************/
int inc_shm_create(key_t key, int size)
{
	return shmget(key, size, IPC_CREAT | 0666);
}

/*************************************************************
函数名称: inc_shm_open
函数功能: 获取其它进程创建共享内存
输入参数: key      -- 关键字
          size -- 内存的大小
输出参数: 无
返回值  ：共享内存ID，错误返-1
**************************************************************/
int inc_shm_open(key_t key, int size)
{
	return shmget(key, size, 0);
}

/*************************************************************
函数名称: inc_shm_at
函数功能: 关联共享内存
输入参数: shm_id     -- 共享内存标识符
输出参数: 无
返回值  ：成功则返回指向共享内存段的地址，错误返NULL
**************************************************************/
void *inc_shm_at(int shm_id)
{
	void *p = NULL;
	
	if (shm_id < 0)
		return NULL;

	p = shmat(shm_id, NULL, 0);

	if ((int)p == -1)
		return NULL;
	else
		return p;
}

/*************************************************************
函数名称: inc_shm_dt
函数功能: 断开共享内存
输入参数: shm_addr     -- 共享内存地址
输出参数: 无
返回值  ：成功则返回指向共享内存段的地址，错误返-1
**************************************************************/
int inc_shm_dt(const void * shm_addr)
{
	if (shm_addr == NULL)
		return -1;

	return shmdt(shm_addr);
}

/*************************************************************
函数名称: inc_shm_delete
函数功能: 删除共享内存
输入参数: shm_id     -- 共享内存标识符
输出参数: 无
返回值  ：无
**************************************************************/
void inc_shm_delete(int shm_id)
{
	shmctl(shm_id, IPC_RMID, NULL);
}

/*************************************************************
函数名称: inc_time_usleep
函数功能: 休眠函数
输入参数: us -- 休眠时间值，单位微秒
输出参数: 无
返回值  ：正常返回0，被信号中断返回-1
**************************************************************/
int inc_time_usleep(unsigned int us) 
{
    int ret;
    struct timespec req;
 
    req.tv_sec  = us/1000000;
    req.tv_nsec = (us%1000000) * 1000;
 
    ret = nanosleep(&req, NULL);
    return ret;
}

/*************************************************************
函数名称: inc_second_to_time_string
函数功能: 将秒数转换为时间字符串
输入参数: second   -- 自1970-01-01 00:00:00 UTC以来的秒数
输出参数: pu8_time -- 时间字符串，格式"2010-11-15 10:39:30"，本地时区，此缓冲区至少20个字节
返回值  ：无
**************************************************************/
void inc_second_to_time_string(U8_T *pu8_time, time_t second)
{
	struct tm tm_time;
	
	localtime_r(&second, &tm_time);
    strftime((char *)pu8_time, 20, "%Y-%m-%d %H:%M:%S", &tm_time);  //本地时区
}

/*************************************************************
函数名称: inc_time_string_to_second
函数功能: 将时间字符串转换为秒数
输入参数: pu8_time -- 时间字符串，格式"2010-11-15 10:39:30"，本地时区
输出参数: 无
返回值  ：自1970-01-01 00:00:00 UTC以来的秒数
**************************************************************/
time_t inc_time_string_to_second(U8_T *pu8_time)
{
	struct tm tm_time;
    strptime((char *)pu8_time, "%Y-%m-%d %H:%M:%S", &tm_time);  //本地时区
	return mktime(&tm_time);
}

/*************************************************************
函数名称: u32_inc_calculate_diff_time_ms
函数功能: 计算时间差
输入参数: pt_last_second          -- 上一次时间
输出参数: 无
返回值  ：时间差值，单位ms
**************************************************************/
U32_T u32_inc_calculate_diff_time_ms(struct timespec *pt_last_time)
{
	struct timespec t_current_time;
	U32_T u32_diff_time;

	clock_gettime(CLOCK_MONOTONIC, &t_current_time);
	u32_diff_time = (t_current_time.tv_sec - pt_last_time->tv_sec)*1000 + (t_current_time.tv_nsec - pt_last_time->tv_nsec)/1000000;
	
	return u32_diff_time;
}

/*************************************************************
函数名称: v_inc_update_remaining_time
函数功能: 更新剩余时间
输入参数: pt_last_second          -- 上一次时间
输出参数: pu32_remaining_time     -- 余下的时间
返回值  ：无
**************************************************************/
void v_inc_update_remaining_time(struct timespec *pt_last_time, U32_T *pu32_remaining_time)
{
	struct timespec t_current_time;
	U32_T u32_diff_time;

	clock_gettime(CLOCK_MONOTONIC, &t_current_time);
	//u32_diff_time = (t_current_time.tv_sec - pt_last_time->tv_sec)*1000 + (t_current_time.tv_nsec - pt_last_time->tv_nsec)/1000000;
	u32_diff_time = t_current_time.tv_sec - pt_last_time->tv_sec;
	if ((u32_diff_time > 0) && (*pu32_remaining_time > 0))
	{
		if (*pu32_remaining_time >= u32_diff_time)
    		*pu32_remaining_time -= u32_diff_time;
		else
			*pu32_remaining_time = 0;
    	*pt_last_time = t_current_time;
	}
}


/*************************************************************
函数名称: inc_file_copy
函数功能: 文件拷贝函数，将源文件内容拷贝到目的文件中
输入参数: source_file_name -- 源文件名
          dest_file_name   -- 目的文件各
输出参数: 无
返回值  ：正常返回0，被信号中断返回-1
**************************************************************/
void inc_file_copy(char *source_file_name, char *dest_file_name)
{
    FILE *fp_read = NULL;
    FILE *fp_write = NULL;
    char buffer[512] = { 0 };
    size_t ret_size = 0;
    
    if ((source_file_name == NULL) || (dest_file_name == NULL))
        return;

    fp_read = fopen(source_file_name, "rb");
    if (fp_read == NULL)
        return;

    fp_write = fopen(dest_file_name, "wb");
    if (fp_write == NULL)
        return;

    do
    {
        ret_size = fread(buffer, 1, sizeof(buffer), fp_read);
        fwrite(buffer, 1, ret_size, fp_write);
    } while (ret_size >= sizeof(buffer));
    
    fclose(fp_read);
    fclose(fp_write);
}

/*************************************************************
函数名称: inc_file_read_integer
函数功能: 从文件中读一个整数，主要用于操作sys下的硬件相关文件
输入参数: file_name -- 文件名
          number    -- 用于存放读到的数据
输出参数: 无
返回值  ：-1读取失败，0读取成功
**************************************************************/
int inc_file_read_integer(char *file_name, int *number)
{
    FILE *fp = NULL;
    
    if (file_name == NULL)
        return -1;

    fp = fopen(file_name, "r");
    if (fp == NULL)
        return -1;
        
    fscanf(fp, "%d", number);
    fclose(fp);
    
    return 0;
}

/*************************************************************
函数名称: inc_file_write_integer
函数功能: 向文件中写入一个整数，主要用于操作sys下的硬件相关文件
输入参数: file_name -- 文件名
          number    -- 要写入的整数
输出参数: 无
返回值  ：无
**************************************************************/
void inc_file_write_integer(char *file_name, int number)
{
    FILE *fp = NULL;
    
    if (file_name == NULL)
        return;

    fp = fopen(file_name, "w");
    if (fp == NULL)
        return;
        
    fprintf(fp, "%d", number);
    fclose(fp);
}

/*************************************************************
函数名称: inc_file_write_string
函数功能: 向文件中写入一个字符串，主要用于操作sys下的硬件相关文件
输入参数: file_name -- 文件名
          string    -- 要写入的字符串
输出参数: 无
返回值  ：无
**************************************************************/
void inc_file_write_string(char *file_name, char *string)
{
    FILE *fp = NULL;
    
    if (file_name == NULL)
        return;

    fp = fopen(file_name, "w");
    if (fp == NULL)
        return;
        
    fprintf(fp, "%s", string);
    fclose(fp);
}

/*************************************************************
函数名称: inc_file_read_string
函数功能: 从文件中读一个字符串
输入参数: file_name -- 文件名
          string    -- 存放读取的字符串
          len       -- string缓冲区的长度
          line_number -- 读取的行号
输出参数: 无
返回值  ：无
**************************************************************/
void inc_file_read_string(char *file_name, char *string, int len, int line_number)
{
    FILE *fp = NULL;
    char buf[1024];
    
    if (file_name == NULL)
        return;

    fp = fopen(file_name, "r");
    if (fp == NULL)
        return;

	if (line_number > 1)
	{
		int i;
		for (i=0; i<line_number-1; i++)
			fgets(buf, sizeof(buf), fp);
	}
	
    fgets(string, len, fp);      
    fclose(fp);
    
    if (strcmp(&(string[strlen(string)-2]), "\r\n") == 0)
    	string[strlen(string)-2] = 0;   //去掉\r\n
    else if (strcmp(&(string[strlen(string)-1]), "\n") == 0)
    	string[strlen(string)-1] = 0;   //去掉\n
}



// /*************************************************************
// 函数名称: inc_wdt_login
// 函数功能: 向看门狗任务注册
// 输入参数: u32_task_id -- 任务ID，看文件IncDogMsg.h中的定义
//           u32_timeout -- 超时值，单位S
// 输出参数: 无
// 返回值  ：无
// **************************************************************/
// void inc_wdt_login(U32_T u32_task_id, U32_T u32_timeout)
// {
//     DOG_MSG_T t_dog_msg;

//     t_dog_msg.type = 1;
//     t_dog_msg.u32_prog_id = u32_task_id;
//     t_dog_msg.u32_msg_type = DOG_MSG_QUEUE_TYPE_LOG_IN;
//     t_dog_msg.u32_timeout = u32_timeout;

//     inc_message_write((char *)(&t_dog_msg), sizeof(t_dog_msg));
// }

// /*************************************************************
// 函数名称: inc_wdt_feed
// 函数功能: 喂狗
// 输入参数: u32_task_id -- 任务ID，看文件IncDogMsg.h中的定义
//           u32_timeout -- 超时值，单位S
// 输出参数: 无
// 返回值  ：无
// **************************************************************/
// void inc_wdt_feed(U32_T u32_task_id, U32_T u32_timeout)
// {
//     DOG_MSG_T t_dog_msg;

//     t_dog_msg.type = 1;
//     t_dog_msg.u32_prog_id = u32_task_id;
//     t_dog_msg.u32_msg_type = DOG_MSG_QUEUE_TYPE_HEARTBEAT;
//     t_dog_msg.u32_timeout = u32_timeout;

//     inc_message_write((char *)(&t_dog_msg), sizeof(t_dog_msg));
// }

/*************************************************************
函数名称: inc_shell_cmd_execute
函数功能: shell命令执行
输入参数: cmd -- 命令字符串
输出参数: 无
返回值  ：0正常，-1失败
**************************************************************/
int inc_shell_cmd_execute(const char * cmd)
{ 
   FILE *fp; 
   int   res; 
   char  buf[1024]; 

   if (cmd == NULL) 
   { 
       printf("inc_shell_cmd_execute cmd is NULL!\n"); 
       return -1; 
   } 

   if ((fp = popen(cmd, "r") ) == NULL) 
   { 
       printf("popen error: %s/n", strerror(errno)); 
       return -1; 
   } 
   else 
   { 
       while(fgets(buf, sizeof(buf), fp)) 
       { 
           printf("%s", buf); 
       } 

       if ((res = pclose(fp)) == -1) 
       { 
           printf("close popen fp error!\n"); 
           return res; 
       } 
       else if (res == 0) 
       { 
           return res; 
       } 
       else 
       { 
           printf("popen res is :%d\n", res); 
           return res; 
       } 
   } 
}

/*************************************************************
函数名称: u32_inc_get_system_time_ms
函数功能: 获取系统上电运行已来的毫秒时间
输入参数: 无
输出参数: 无
返回值  ：运行时间，单位ms
**************************************************************/
U32_T u32_inc_get_system_time_ms(void)
{
    struct timespec t_current_time;
    U32_T u32_time_ms;

    clock_gettime(CLOCK_MONOTONIC, &t_current_time);
    u32_time_ms = t_current_time.tv_sec * 1000 + t_current_time.tv_nsec / 1000000;

    return u32_time_ms;
}

/*************************************************************
函数名称: u32_inc_get_diff_time_ms
函数功能: 计算时间差
输入参数: pu32_last_time          -- 上一次时间
输出参数: 无
返回值  ：时间差值，单位ms
**************************************************************/
U32_T u32_inc_get_diff_time_ms(U32_T *pu32_last_time)
{
    U32_T u32_curr_time, u32_diff_time;

    u32_curr_time = u32_inc_get_system_time_ms();
    if(u32_curr_time >= (*pu32_last_time))
    {
        u32_diff_time = u32_curr_time - (*pu32_last_time);
    }
    else
    {
        u32_diff_time = ~(*pu32_last_time) + u32_curr_time;
    }

    return u32_diff_time;
}
