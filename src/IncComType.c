/**************************************************************************
文 件 名：                   IncComType.c
作    者：                   钟欲飞
创建日期：                   2012.01.17
功能描述：                   串口通信相关通用数据和子程序

修改日期       修改人          修改内容

**************************************************************************/

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <termios.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <errno.h>
#include    <pthread.h>
#include    <assert.h>
#include    <time.h>
#include    "IncComType.h"


/***************************************************************
函数功能：可重入函数，串口通信速率设置
入口参数：fd-串口句柄，baud-波特率
出口参数：无
返回参数：0-成功，非0-失败
创建日期：
更改描述：
***************************************************************/
int com_set_speed(int fd, COM_BAUD baud)
{
    int speed;
    struct   termios   newtio;
    if( tcgetattr(fd, &newtio) != 0 )
    {
        perror("Cannot get standard input description");
        return -1;
    }

    switch(baud)
    {
        case Baud_600:
            speed = B600;
            break;
        case Baud_1200:
            speed = B1200;
            break;
        case Baud_2400:
            speed = B2400;
            break;
        case Baud_4800:
            speed = B4800;
            break;
        case Baud_9600:
            speed = B9600;
            break;
        case Baud_19200:
            speed = B19200;
            break;
        case Baud_38400:
            speed = B38400;
            break;
        case Baud_57600:
            speed = B57600;
            break;
        case Baud_115200:
            speed = B115200;
            break;
    }

    tcflush(fd, TCIOFLUSH);  //清输入队列 TCIFLUSH｜TCOFLUSH｜TCIOFLUSH

    cfsetispeed(&newtio, speed);
    cfsetospeed(&newtio, speed);

    if( tcsetattr(fd, TCSANOW, &newtio) != 0 )
    {
        perror("Cannot set standard input description");
        return -1;
    }

    tcflush(fd, TCIOFLUSH);  //清输入队列 TCIFLUSH｜TCOFLUSH｜TCIOFLUSH

    return 0;
}

/***************************************************************
函数功能：可重入函数，串口通信奇偶校验设置
入口参数：fd-串口句柄，parity-奇偶校验
出口参数：无
返回参数：0-成功，非0-失败
创建日期：
更改描述：
***************************************************************/
int com_set_parity(int fd, COM_PARITY parity)
{
    struct   termios   newtio;
    if( tcgetattr(fd, &newtio) != 0 )
    {
        perror("Cannot get standard input description");
        return -1;
    }
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;  //8位数据

    switch(parity)
    {
        case _EVEN:
            newtio.c_cflag  |= PARENB; //允许奇偶校验
            newtio.c_cflag  &= ~PARODD; //偶校验
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;
        case _ODD:
            newtio.c_cflag  |= PARENB; //允许奇偶校验
            newtio.c_cflag  |= PARODD; //奇校验
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;
        case _NONE:
            newtio.c_cflag  &= ~PARENB; //禁止奇偶校验
            newtio.c_iflag  &= ~INPCK;  //禁止输入奇偶校验
            break;
        /*case MARK:
            newtio.c_cflag  &= ~PARENB; //禁止奇偶校验
            newtio.c_cflag  |= CSTOPB; //设置两个停止位
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;
        case SPACE:
            newtio.c_cflag  &= ~PARENB; //禁止奇偶校验
            newtio.c_cflag  &= ~CSTOPB; //设置一个停止位
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;*/
    }

    newtio.c_cflag  &= ~CSTOPB; //设置一个停止位

    tcflush(fd, TCIFLUSH);  //清输入队列 TCIFLUSH｜TCOFLUSH｜TCIOFLUSH

    if( tcsetattr(fd, TCSANOW, &newtio) != 0 )
    {
        perror("Cannot set standard input description");
        return -1;
    }

    return 0;
}

/***************************************************************
函数功能：可重入函数，串口通信速率设置
入口参数：fd-串口句柄，baud-波特率
出口参数：无
返回参数：0-成功，非0-失败
创建日期：
更改描述：
***************************************************************/
int com_set_para(int fd, COM_BAUD baud, COM_PARITY parity)
{
    int      speed;
    struct   termios   newtio;
    if( tcgetattr(fd, &newtio) != 0 )
    {
        perror("Cannot get standard input description");
        return -1;
    }
    tcflush(fd, TCIOFLUSH);  //清输入队列 TCIFLUSH｜TCOFLUSH｜TCIOFLUSH

	speed = B9600;
    //通信速率
    switch(baud)
    {
        case Baud_600:
            speed = B600;
            break;
        case Baud_1200:
            speed = B1200;
            break;
        case Baud_2400:
            speed = B2400;
            break;
        case Baud_4800:
            speed = B4800;
            break;
        case Baud_9600:
            speed = B9600;
            break;
        case Baud_19200:
            speed = B19200;
            break;
		case Baud_38400:
            speed = B38400;
            break;
        case Baud_57600:
            speed = B57600;
            break;
		case Baud_115200:
            speed = B115200;
            break;
    }
    cfsetispeed(&newtio, speed);
    cfsetospeed(&newtio, speed);

    //通信检验
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;  //8位数据
    switch(parity)
    {
        case _EVEN:
            newtio.c_cflag  |= PARENB; //允许奇偶校验
            newtio.c_cflag  &= ~PARODD; //偶校验
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;
        case _ODD:
            newtio.c_cflag  |= PARENB; //允许奇偶校验
            newtio.c_cflag  |= PARODD; //奇校验
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;
        case _NONE:
            newtio.c_cflag  &= ~PARENB; //禁止奇偶校验
            newtio.c_iflag  &= ~INPCK;  //禁止输入奇偶校验
            break;
        /*case MARK:
            newtio.c_cflag  &= ~PARENB; //禁止奇偶校验
            newtio.c_cflag  |= CSTOPB; //设置两个停止位
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;
        case SPACE:
            newtio.c_cflag  &= ~PARENB; //禁止奇偶校验
            newtio.c_cflag  &= ~CSTOPB; //设置一个停止位
            newtio.c_iflag  |= INPCK;  //允许输入奇偶校验
            break;*/
    }
    newtio.c_cflag  &= ~CSTOPB; //设置一个停止位

    if( tcsetattr(fd, TCSANOW, &newtio) != 0 )
    {
        perror("Cannot set standard input description");
        return -1;
    }

    tcflush(fd, TCIOFLUSH);  //清输入队列 TCIFLUSH｜TCOFLUSH｜TCIOFLUSH

    return 0;
}

/***************************************************************
函数功能：可重入函数，打开串口通信设备文件
入口参数：pszDev-串口设备文件路径，baud-波特率，parity-奇偶校验
出口参数：无
返回参数：成功，fd-串口句柄；失败，-1
创建日期：
更改描述：
***************************************************************/
int com_open_port(char *pszDev, COM_BAUD baud, COM_PARITY parity)
{
    int             fd,speed;
    struct termios  newtio;

    bzero(&newtio, sizeof(newtio));

    fd = open(pszDev, O_RDWR|O_NOCTTY|O_NDELAY);//O_NONBLOCK 非阻塞方式
    if( fd == -1 )
    {
        char buff[64];
        snprintf(buff,sizeof(buff),"open_port:Unable to open %s",pszDev);
        perror(buff);
        return -1;
    }

    if( tcgetattr(fd, &newtio) == -1 )
    {
        perror("Cannot get standard input description");
        return -1;
    }

    //模式设置   c_lflag
    //newtio.c_lflag  &= ~(ICANON|ECHO|ECHOE|ISIG);
    //输出设置   c_oflag
    //newtio.c_oflag  &= ~OPOST;
    //输入设置   c_iflag
    //newtio.c_iflag  &= ~(IXON|IXOFF|IXANY);
    //防止发0D时变为0A
    //newtio.c_iflag &= ~(INLCR|ICRNL|IGNCR);
    //newtio.c_oflag &= ~(ONLCR|OCRNL);


    //消除发送模式规则
    newtio.c_lflag = 0;
    newtio.c_oflag = 0;
    newtio.c_iflag = 0;

    //消除字符屏蔽规则
    newtio.c_cc[VINTR]    = 0;
    newtio.c_cc[VQUIT]    = 0;
    newtio.c_cc[VERASE]   = 0;
    newtio.c_cc[VKILL]    = 0;
    newtio.c_cc[VEOF]     = 0;
    newtio.c_cc[VTIME]    = 1; //0.1秒
    newtio.c_cc[VMIN]     = 0;
    newtio.c_cc[VSWTC]    = 0;
    newtio.c_cc[VSTART]   = 0;
    newtio.c_cc[VSTOP]    = 0;
    newtio.c_cc[VSUSP]    = 0;
    newtio.c_cc[VEOL]     = 0;
    newtio.c_cc[VREPRINT] = 0;
    newtio.c_cc[VDISCARD] = 0;
    newtio.c_cc[VWERASE]  = 0;
    newtio.c_cc[VLNEXT]   = 0;
    newtio.c_cc[VEOL2]    = 0;

    tcflush(fd, TCIFLUSH);
    if( tcsetattr(fd, TCSANOW, &newtio) != 0 )
    {
        perror("Cannot set standard input description");
        return -1;
    }

    com_set_speed(fd, baud);
    com_set_parity(fd, parity);

    return(fd);
}

/***************************************************************
函数功能：可重入函数，关闭串口通信设备文件
入口参数：fd-串口句柄
出口参数：无
返回参数：无
创建日期：
更改描述：
***************************************************************/
void com_close_port(int fd)
{
    close(fd);
}


/***************************************************************
函数功能：可重入函数，串口接收数据
入口参数：fd-串口句柄
		recv_buff - 接收缓冲区指针
		recv_len - 接收最大长度
出口参数：无
返回参数：接收的字节数
创建日期：
更改描述：
***************************************************************/
int com_recv_data(int fd, U8_T *recv_buff, int recv_len)
{
	int ret,left;
	
	left = recv_len;
	while (left > 0)
	{
		ret = 0;
		ret = read(fd,recv_buff,left);
		if (ret > 0)
		{
			left -= ret;
			recv_buff += ret;
			break;
		}
		else
		{
			break;
		}		
	}
	
	return (recv_len - left);

}

/***************************************************************
函数功能：可重入函数，串口发送数据
入口参数：fd-串口句柄
		send_buff - 发送缓冲区指针
		send_len - 发送数据长度
出口参数：无
返回参数：发送的字节数
创建日期：
更改描述：
***************************************************************/
int com_send_data(int fd, U8_T *send_buff, int send_len)
{
	int ret;
	int	nleft;
	char *ptmp;

	ret = 0;
	nleft = send_len;
    ptmp = (char *)send_buff;

	while (nleft > 0)
	{
		ret = write(fd, ptmp, nleft);
		if (ret < 0)	 
		{
			if (nleft == send_len)
			{
				return(-1);/* error */
			}
			else
				break;// erro, return amount written so far
		
		} 
		else if(ret == 0)
			break;// EOF
   
		nleft -= ret;
		ptmp += ret;
	}
	return(send_len - nleft);

}

