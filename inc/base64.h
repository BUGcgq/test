#ifndef __BASE_64_H__
#define __BASE_64_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>


// 函数原型
char* base64_encode(const unsigned char* input, int length);
char* base64_decode(const char* input);


#ifdef __cplusplus
}
#endif

#endif