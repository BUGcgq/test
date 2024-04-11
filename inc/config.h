#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <dirent.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct CFG_OcppServer
{
	char bisSupported;       //是否使用OCPP
	char bisWss;             // 是否使用Wss连接服务器
	char strprotocolName[32]; //协议名称
	char straddress[32];      //服务器域名
	unsigned int u32port;              //服务器端口 
	char strpath[64];           //服务器路径
	char strusername[64];       //用户名
	char strpassword[64];       //密码
} stCFG_OcppServer, *pstCFG_OcppServer;

typedef struct CFG_Network
{
    int u32dhcp;   // 是否使用DHCP
	unsigned char streth0_IP[16];   // IP地址
	unsigned char streth0_MASK[16]; // 地址掩码
	unsigned char streth0_GATE[16]; // 网关
	unsigned char streth0_MAC[18];  //
	unsigned char strDNS_ADDR0[10]; // DNS地址
} stCFG_Network, *pstCFG_Network;

typedef struct CFG_PROTOCOLINFO
{
    stCFG_Network    network;
    stCFG_OcppServer ocpp;

} stCFG_PROTOCOLINFO, *pstCFG_PROTOCOLINFO;


void CFG_DefaultProtocol(pstCFG_PROTOCOLINFO pConfigProtocol);
int CFG_ReadProtocol(const char *filePath, pstCFG_PROTOCOLINFO pConfigProtocol);
int CFG_SaveProtocol(const char *filePath, pstCFG_PROTOCOLINFO pConfigProtocol);


#ifdef __cplusplus
}
#endif

#endif