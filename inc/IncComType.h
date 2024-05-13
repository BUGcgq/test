/**************************************************************************
文 件 名：                   IncComType.h
作    者：                   钟欲飞
创建日期：                   2012.01.17
功能描述：                   串口通信相关通用数据和子程序

修改日期       修改人          修改内容

**************************************************************************/

#ifndef     _INC_COM_TYPE_H_
#define     _INC_COM_TYPE_H_

#ifdef __cplusplus
    extern "C"{
#endif

/************************************包函文件******************************/
#include    <pthread.h>
#include "Type.h"

/************************************常量定义******************************/


/*************************************宏定义******************************/



/************************************类型定义******************************/
typedef enum
{
	Baud_600,
	Baud_1200,
	Baud_2400,
	Baud_4800,
	Baud_9600,
	Baud_19200,
	Baud_38400,
	Baud_57600,
	Baud_115200
}COM_BAUD;

typedef enum
{
	_ODD,
	_EVEN,
	_NONE
}COM_PARITY;


/************************************函数申明******************************/
int com_set_speed(int fd, COM_BAUD baud);
int com_set_parity(int fd, COM_PARITY parity);
int com_set_para(int fd, COM_BAUD baud, COM_PARITY parity);
int com_open_port(char *pszDev, COM_BAUD baud, COM_PARITY parity);
void com_close_port(int fd);
int com_recv_data(int fd, U8_T *recv_buff, int recv_len);
int com_send_data(int fd, U8_T *send_buff, int send_len);

#ifdef __cplusplus
        }
#endif

#endif	//_INC_COM_TYPE_H_
