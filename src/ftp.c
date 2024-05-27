#include "ftp.h"

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-05-22
 * 函 数 名：ET_FTP_downloadFile
 * 描    述: ftp下载文件
 *
 * 参    数: url - [远程服务器url]
 * 参    数: localPath - [本地路径]
 * 参    数: mode - 0 - 主动，1 - 被动
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */
int ET_FTP_downloadFile(const char *url, const char *localPath, int mode)
{
    if (url == NULL || localPath == NULL)
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
        fp = fopen(localPath, "wb");
        if (fp == NULL)
        {
            printf("Failed to open file: %s\n", localPath);
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

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-05-22
 * 函 数 名：ET_FTP_uploadFile
 * 描    述: ftp上传文件
 *
 * 参    数: url - [远程服务器url]
 * 参    数: localPath - [本地路径]
 * 参    数: mode - 0 - 主动，1 - 被动
 * 返回类型：int
 * 特殊说明：无
 * 修改记录: 无
 * *****************************************************************************
 */

int ET_FTP_uploadFile(const char *url, const char *localPath, int mode)
{
    if (url == NULL || localPath == NULL)
    {
        fprintf(stderr, "错误: URL 和本地文件路径不能为空。\n");
        return -1; // 返回错误代码，表示参数无效
    }

    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        fp = fopen(localPath, "rb");
        if (fp == NULL)
        {
            fprintf(stderr, "Failed to open file: %s\n", localPath);
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_READDATA, fp);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        if (strncmp(url, "sftp://", 7) == 0)
        {
            // 使用 SFTP 协议
            curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_SFTP);
            curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
        }
        else if (strncmp(url, "ftp://", 6) == 0)
        {
            // 使用 FTP 协议
            if (mode == 0)
            {
                // 主动
                curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
            }
            else
            {
                // 被动
                curl_easy_setopt(curl, CURLOPT_FTPPORT, NULL);
            }
        }
        else
        {
            fprintf(stderr, "Unsupported protocol: %s\n", url);
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }

        fclose(fp);
        curl_easy_cleanup(curl);
    }
    else
    {
        fprintf(stderr, "Failed to initialize curl.\n");
        return -1;
    }

    return 0;
}

void main(void)
{

    if (ET_FTP_uploadFile("sftp://inc01:kth7-S9vS@14.29.244.45:21/inc/tmp/test.txt", "/app/core/test.txt", 1) == 0)
    {
        printf("上传成功\n");
    }
    else
    {
        printf("上传失败\n");
    }
}