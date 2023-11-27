#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "websocket.h"

typedef struct
{
	ocpp_connect_t *connect;
	struct lws_client_connect_info *ccinfo;
	size_t sendDataLen;						 // 发送数据大小
	char sendbuff[OCPP_CONNECT_SEND_BUFFER]; // 发送存储区
	pthread_mutex_t buffLock;				 // 缓存区锁
} ocpp_connect_session_data_t;

static ocpp_connect_session_data_t session_data;
static struct lws_context *context;
// 定义一个定时器结构体
static lws_sorted_usec_list_t sul;
/**
 * @description: 发送ping
 * @param:
 * @return:
 */
int send_ping(struct lws *wsi)
{
	unsigned char buf[LWS_PRE + 2]; // 2是Ping消息的大小
	unsigned char *p = &buf[LWS_PRE];

	p[0] = 0x89; // 设置Ping帧的标志位
	p[1] = 0x00; // 设置Payload Length，通常为0，或者根据需要设置具体大小

	int ret = lws_write(wsi, p, 2, LWS_WRITE_PING);
	if (ret < 0)
	{
		lwsl_err("send ping fail, ret = %d\n", ret);
		return -1;
	}
	else
	{
		lwsl_notice("send ping success, ret = %d\n", ret);
		return 0;
	}
}
// 定义一个定时器回调函数
static void sul_cb(lws_sorted_usec_list_t *sul)
{
	// 使用lws_client_connect_via_info函数来重新创建一个客户端连接，并且保存返回的wsi变量
	struct lws *wsi;
	wsi = lws_client_connect_via_info(session_data.ccinfo);
	if (wsi == NULL)
	{
		lwsl_err("Failed to reconnect to server\n");
	}
	else
	{
		lwsl_info("Reconnected to server\n");
	}
}
/**
 * @description:
 * @param:
 * @return:
 */
static int ocpp_connect_websocket_send_back(struct lws *wsi_in, char *str, int str_size_in)
{
	int n = 0;
	int len = 0;
	char *out = NULL;

	if (str_size_in < 1)
		len = strlen(str);
	else
		len = str_size_in;

	// 为加密的字符串保留内存
	out = (char *)calloc(1, sizeof(char) * (LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));

	memcpy(out + LWS_SEND_BUFFER_PRE_PADDING, str, len);
	n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

	if (out)
		free(out);
	return n;
}

static int ocpp_connect_service_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	switch (reason)
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
		lwsl_notice("PROTOCOL_INIT\n");
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_err("CONNECTION ERROR: %s\n", (char *)in);
		pthread_mutex_lock(&session_data.buffLock);
		session_data.connect->isConnect = false;
		pthread_mutex_unlock(&session_data.buffLock);
		lws_sul_schedule(context, 0, &sul, sul_cb, SERVER_RECONNECT_INTERVAL * 1000 * LWS_US_PER_MS); // 安排一个定时器，在1
		break;

	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_notice("CONNECTION ESTABLISHED\n");
		pthread_mutex_lock(&session_data.buffLock);
		session_data.connect->isConnect = true;
		pthread_mutex_unlock(&session_data.buffLock);
		lws_set_timer_usecs(wsi, 5000000);
		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		lwsl_err("CONNECTION CLOSED: %s\n", (char *)in);
		pthread_mutex_lock(&session_data.buffLock);
		session_data.connect->isConnect = false;
		pthread_mutex_unlock(&session_data.buffLock);
		lws_sul_schedule(context, 0, &sul, sul_cb, SERVER_RECONNECT_INTERVAL * 1000 * LWS_US_PER_MS); // 安排一个定时器，在1
		break;

	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
		lwsl_notice("CONNECTION Authorization START\n");
		char *username = session_data.connect->username;
		char *password = session_data.connect->password;
		char *authorization = (char *)calloc(1, 256);
		char *user_pwd = (char *)calloc(1, 256);

		sprintf(user_pwd, "%s:%s", username, password);
		lws_b64_encode_string(user_pwd, strlen(user_pwd), authorization, 256);
		char *basic = (char *)calloc(1, 256);
		sprintf(basic, "Basic %s", authorization);
		int value_len = strlen(basic);
		unsigned char **p, *end;
		p = (unsigned char **)in;
		end = (*p) + len;
		int ret = lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_AUTHORIZATION, basic, value_len, p, end);

		free(authorization);
		free(user_pwd);
		free(basic);
		lwsl_notice("CONNECTION Authorization END\n");

		break;

	case LWS_CALLBACK_TIMER:
		if (send_ping(wsi) == -1)
		{
			lwsl_notice("检测到与服务器断开\n");
			pthread_mutex_lock(&session_data.buffLock);
			session_data.connect->isConnect = false;
			pthread_mutex_unlock(&session_data.buffLock);
		}
		if (session_data.connect->isConnect == false)
		{
			lws_close_reason(wsi, LWS_CLOSE_STATUS_GOINGAWAY, NULL, 0);
			return -1;
		}
		lws_set_timer_usecs(wsi, 5000000);
		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		pthread_mutex_lock(&session_data.buffLock);
		if (session_data.sendDataLen > 0)
		{
			ocpp_connect_websocket_send_back(wsi, session_data.sendbuff, session_data.sendDataLen);
			session_data.sendDataLen = 0;
			memset(session_data.sendbuff,0,OCPP_CONNECT_SEND_BUFFER);
		}
		pthread_mutex_unlock(&session_data.buffLock);
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE:
		session_data.connect->receive(in, len);
		break;

	case LWS_CALLBACK_RECEIVE_PONG:
		lwsl_notice("Received PONG frame\n"); // 在接收到pong帧时打印消息
		break;

	default:
		break;
	}
	// DEBUG_MSG("out");
	return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static void ocpp_connect_initialize_websocket_context(struct lws_context_creation_info *const info, struct lws_protocols *const protocol, ocpp_connect_t *const connect)
{

	memset(info, 0, sizeof(struct lws_context_creation_info));
	memset(protocol, 0, sizeof(struct lws_protocols));
	protocol->name = connect->protocolName;
	protocol->callback = &ocpp_connect_service_callback;
	protocol->per_session_data_size = 0;
	protocol->rx_buffer_size = 0;
	protocol->id = 0;

	info->port = CONTEXT_PORT_NO_LISTEN; // 创建客户端,不监听端口
	info->iface = NULL;
	info->protocols = protocol;
	info->gid = -1;
	info->uid = -1;
	info->options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;					   // 初始化SSL库
	info->client_ssl_private_key_password = NULL;							   // 私钥
	info->client_ssl_cert_filepath = NULL;									   // 客户端的证书
	info->client_ssl_cert_mem = NULL;										   // 从内存而不是文件加载客户端证书
	info->client_ssl_cert_mem_len = 0;										   // 长度字节
	info->client_ssl_private_key_filepath = connect->ssl_private_key_filepath; // 私钥路径
	info->client_ssl_key_mem = NULL;										   // 加载客户端密钥从内存而不是文件
	info->client_ssl_key_mem_len = 0;										   //
	info->client_ssl_ca_filepath = connect->ssl_ca_filepath;				   // CA证书文件路径或NULL
	info->client_ssl_ca_mem = NULL;											   // 从内存加载CA证书，而不是文件
	info->client_ssl_ca_mem_len = 0;										   //
	info->client_ssl_cipher_list = "AES256-SHA:RSA";						   // NULL;                               //支持的加密套件,用于在会话中加密
}

/**
 * @description: 建立websock连接
 * @param:
 * @return:
 */
static void ocpp_connect_send(const char *const message, size_t len)
{

	pthread_mutex_lock(&session_data.buffLock);
	strncpy(session_data.sendbuff, message, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = len;
	pthread_mutex_unlock(&session_data.buffLock);

	return;
}

/**
 * @description: 建立websock连接
 * @param:
 * @return:
 */
static void *ocpp_connect_websocket(void *arg)
{
	ocpp_connect_t *connect = (ocpp_connect_t *)arg;
	struct lws_protocols protocol;
	struct lws_context_creation_info info;
	struct lws_client_connect_info ccinfo;
	struct lws *wsi = NULL;
	ocpp_connect_initialize_websocket_context(&info, &protocol, connect);
	context = lws_create_context(&info);
	if (context == NULL)
	{
		printf("conntext NULL,connect fail\n");
		return NULL;
	}

	session_data.connect = connect;
	session_data.ccinfo = &ccinfo;

	ccinfo.context = context;
	ccinfo.port = connect->port;
	ccinfo.address = connect->address;
	ccinfo.path = connect->path;
	ccinfo.ssl_connection = connect->isWss ? (LCCSCF_USE_SSL | LCCSCF_ALLOW_EXPIRED) : 0;
	ccinfo.host = connect->address;
	ccinfo.origin = connect->address;
	ccinfo.ietf_version_or_minus_one = -1;
	ccinfo.client_exts = NULL;
	ccinfo.protocol = protocol.name;
	lwsl_notice("%s ocpp connect address = %s  port = %d\n", __func__, ccinfo.address, ccinfo.port);
	lwsl_notice("%s ocpp connect path = %s isWss = %d protocolName = %s\n", __func__, ccinfo.path, connect->isWss, ccinfo.protocol);

	while (1)
	{
		wsi = lws_client_connect_via_info(&ccinfo);
		if (wsi != NULL)
		{
			break;
		}
		sleep(5);
	}
	int n = 0;
	while (1)
	{
		n = lws_service(context, 0);
	}

	lws_context_destroy(context);

	return NULL;
}

/**
 * @description: 连接初始化
 * @param:
 * @return:
 */
void ocpp_connect_init(ocpp_connect_t *connect)
{
	connect->send = ocpp_connect_send;
	connect->isConnect = false; // default no connect server
	memset(session_data.sendbuff, 0, OCPP_CONNECT_SEND_BUFFER);
	session_data.sendDataLen = 0;
	session_data.connect = connect;
	pthread_mutex_init(&session_data.buffLock, NULL);
	pthread_t ptid_connect;
	if (pthread_create(&ptid_connect, NULL, ocpp_connect_websocket, connect) != 0)
	{
		printf("cann't create connect thread %s\n", strerror(errno));
	}

	printf("connect init end\n");
}
