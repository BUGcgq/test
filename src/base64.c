#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "base64.h"


/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：base64_encode
 * 描    述: 使用base64编码
 * 
 * 参    数: input - [输入要编码的数据] 
 * 参    数: length - [长度] 
 * 返回类型：成功返回char*，失败返回NULL 
 * 特殊说明：返回值要free
 * 修改记录: 无
 * *****************************************************************************
 */
char *base64_encode(const unsigned char *input, int length)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    char *output = (char *)malloc(bufferPtr->length + 1);
    if (output == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        BIO_free_all(bio);
        return NULL;
    }
    memcpy(output, bufferPtr->data, bufferPtr->length);
    output[bufferPtr->length] = '\0';
    BIO_free_all(bio);
    return output;
}

/**
 * *****************************************************************************
 * 作    者: 陈贵全
 * 创建日期: 2024-03-11
 * 函 数 名：base64_decode
 * 描    述: 使用base64解码
 * 
 * 参    数: input - [输入要解码的数据] 
 * 返回类型：成功返回char*，失败返回NULL 
 * 特殊说明：返回值要free
 * 修改记录: 无
 * *****************************************************************************
 */
char *base64_decode(const char *input)
{
    BIO *bio, *b64;
    int decodeLen = 0;
    char *output = (char *)malloc(strlen(input) + 1);
    if (output == NULL) {
        fprintf(stderr, "Failed to allocate memory.\n");
        return NULL;
    }
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf((void *)input, -1);
    bio = BIO_push(b64, bio);
    decodeLen = BIO_read(bio, output, strlen(input));
    output[decodeLen] = '\0';
    BIO_free_all(bio);
    return output;
}

// int main()
// {
//     const char* original_url = "ftp://inc02:za%40g0Pg6J@14.29.244.45:21/EVCM-SD10.tar.gz";
//     // Base64编码URL
//     char* encoded_url = base64_encode((const unsigned char*)original_url, strlen(original_url));
//     if (encoded_url) {
//         printf("Base64 Encoded URL: %s\n", encoded_url);
//         free(encoded_url);
//     }
//     // Base64解码URL
//     const char* base64_encoded_url = "ZnRwOi8vaW5jMDI6emElNDBnMFBnNkpAMTQuMjkuMjQ0LjQ1OjIxL0VWQ00tU0QxMC50YXIuZ3o=";
//     char* decoded_url = base64_decode(base64_encoded_url);
//     if (decoded_url) {
//         printf("Base64 Decoded URL: %s\n", decoded_url);
//         free(decoded_url);
//     }

//     return 0;
// }