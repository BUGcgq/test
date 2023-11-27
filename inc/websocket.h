#ifndef __WEBSOCKET__H__
#define __WEBSOCKET__H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <libwebsockets.h>


#define OCPP_CONNECT_SEND_BUFFER 2048 // 发送缓存区大小
#define SERVER_RECONNECT_INTERVAL                                  5                              // 断线重连间隔，单位为秒
typedef struct
{
	bool isWss;						//
	char *ssl_ca_filepath;			// 如果为wss连接,需指明CA证书文件路径
	char *ssl_private_key_filepath; // 私钥路径
	char *protocolName;				// websocket子协议名称 eg:ocpp1.6
	unsigned short port;			// 协议端口
	char *address;					// 连接地址
	char *path;						// 服务器路径
	char *username;					// 用户名
	char *password;					// 密码
	bool isConnect;					// 是否与服务器建立连接
	void (*send)(const char *const message, size_t len);
	void (*receive)(void *message, int len); // 需上层实现对接收的处理
} ocpp_connect_t;


struct queDataSrv_rep
{
    int gunNo;//枪号
    char startDate[32];// 查询起始时间
    char stopDate[32];// 查询终止时间
    int askType;//查询类型
    char logQueryNo[38];//查询流水号
};
void ocpp_connect_init(ocpp_connect_t *connect);
#ifdef __cplusplus
}
#endif

#endif