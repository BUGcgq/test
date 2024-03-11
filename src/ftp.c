#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ftp.h"

int ocpp_download_file(const char *url, const char *local_file_path, int mode)
{
    if (url == NULL || local_file_path == NULL)
    {
        fprintf(stderr, "错误:URL和本地文件路径不能为空。\n");
        return -1; // 返回错误代码，表示参数无效
    }

    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(local_file_path, "wb");
        if (fp == NULL)
        {
            printf("Failed to open file: %s\n", local_file_path);
            return -1;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        if (mode == 0)
        {
            // active mode
            curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
        }
        else
        {
            // passive mode
            curl_easy_setopt(curl, CURLOPT_FTPPORT, NULL);
        }
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return -1;
        }
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    return 0;
}
int ocpp_upload_file(const char *url, const char *local_file_path, int mode)
{
    if (url == NULL || local_file_path == NULL)
    {
        fprintf(stderr, "错误:URL和本地文件路径不能为空。\n");
        return -1; // 返回错误代码，表示参数无效
    }

    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(local_file_path, "rb");
        if (fp == NULL)
        {
            printf("Failed to open file: %s\n", local_file_path);
            return -1;
        }
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_READDATA, fp);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        if (mode == 0)
        {
            // active mode
            curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
        }
        else
        {
            // passive mode
            curl_easy_setopt(curl, CURLOPT_FTPPORT, NULL);
        }
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return -1;
        }
        curl_easy_cleanup(curl);
        fclose(fp);
    }
    return 0;
}

