#include "ftp.h"

struct FtpFile
{
    const char *filename;
    FILE *stream;
};

// 写回调函数，将接收到的数据写入本地文件
static size_t writeCallBack(void *buffer, size_t size, size_t nmemb, void *stream)
{
    struct FtpFile *out = (struct FtpFile *)stream;
    if (!out->stream)
    {
        out->stream = fopen(out->filename, "wb");
        if (!out->stream)
        {
            return -1; // 打开文件失败
        }
    }
    return fwrite(buffer, size, nmemb, out->stream);
}

// 下载文件函数，成功返回0，失败返回-1
int ET_FTP_downloadFile(const char *url, const char *localPath)
{
    CURL *curl;
    CURLcode res;
    struct FtpFile ftpfile = {
        localPath, // 本地文件路径
        NULL};

    // 初始化libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // 设置写回调函数
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallBack);
        // 设置写数据的结构体
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

        // 启用SFTP协议
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_SFTP);

        // 执行文件下载
        res = curl_easy_perform(curl);

        // 检查错误
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() 失败: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1;
        }

        // 清理
        curl_easy_cleanup(curl);
    }
    else
    {
        // 如果初始化失败
        curl_global_cleanup();
        return -1;
    }

    // 关闭本地文件
    if (ftpfile.stream)
    {
        fclose(ftpfile.stream);
    }

    // 清理libcurl全局环境
    curl_global_cleanup();

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

int ET_FTP_uploadFile(const char *url, const char *local_file_path, int mode)
{
    if (url == NULL || local_file_path == NULL)
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
        fp = fopen(local_file_path, "rb");
        if (fp == NULL)
        {
            fprintf(stderr, "Failed to open file: %s\n", local_file_path);
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_READDATA, fp);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        if (strstr(url, "sftp://") != NULL)
        {
            // 使用 SFTP 协议
            curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_SFTP);
            curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
        }
        else
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

// void main(void)
// {

//     if (ET_FTP_uploadFile("sftp://inc01:kth7-S9vS@14.29.244.45:21/inc/tmp/test.txt", "/app/core/test.txt", 1) == 0)
//     {
//         printf("上传成功\n");
//     }
//     else
//     {
//         printf("上传失败\n");
//     }
// }