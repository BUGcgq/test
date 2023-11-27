#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "http.h"
// CURLOPT_WRITEFUNCTION / CURLOPT_WRITEDATA： 用于处理响应体信息，将服务器响应的数据写入用户提供的缓冲区。

// CURLOPT_HEADERFUNCTION / CURLOPT_HEADERDATA： 用于处理响应头信息，将服务器响应的头部数据写入用户提供的缓冲区。

// CURLOPT_PROGRESSFUNCTION / CURLOPT_PROGRESSDATA： 用于处理进度信息，比如上传或下载的进度。

// CURLOPT_DEBUGFUNCTION / CURLOPT_DEBUGDATA： 用于处理调试信息，可以用来捕获libcurl的调试信息。
// 结构体用于存储服务器响应信息
// 结构体用于存储服务器响应信息
struct ResponseData
{
    long http_code;
    char *content_type;
    char *response_data;
};

// 发送HTTP请求的函数
void send_request(const char *url, const char *data, int is_get, struct ResponseData *resp_data)
{
    CURL *curl;
    CURLcode res;

    // 初始化curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl)
    {
        // 设置请求的URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // 如果是POST请求，设置POST数据
        if (!is_get)
        {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        }

        // 设置CURLOPT_WRITEFUNCTION为NULL，将响应的数据直接写入到结构体中
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp_data);

        // 发送请求
        res = curl_easy_perform(curl);

        // 获取状态码
        if (res == CURLE_OK)
        {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(resp_data->http_code));
            // 获取响应类型
            curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &(resp_data->content_type));
        }
        else
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // 清理curl
        curl_easy_cleanup(curl);

        // 全局清理
        curl_global_cleanup();
    }
}

int main()
{
    // 设置请求的URL和数据
    const char *url = "https://example.com/api";
    const char *data = "key1=value1&key2=value2";

    // 是否是GET请求，如果是GET请求，将第三个参数设为1，否则设为0
    int is_get = 0;

    // 初始化结构体
    struct ResponseData resp_data = {0, NULL, NULL};

    // 发送请求并获取状态码、响应类型和响应内容
    send_request(url, data, is_get, &resp_data);

    // 根据状态码处理不同的数据
    if (resp_data.http_code == 200)
    {
        printf("Request successful (HTTP Status Code 200)\n");
        // 在这里处理成功的情况
    }
    else
    {
        printf("Request failed with HTTP Status Code: %ld\n", resp_data.http_code);
        // 在这里处理其他状态码的情况
    }

    // 打印响应类型和响应内容
    printf("Content Type: %s\n", resp_data.content_type);
    printf("Response Data: %s\n", resp_data.response_data);

    // 注意：在这里释放响应内容的内存
    free(resp_data.response_data);

    return 0;
}