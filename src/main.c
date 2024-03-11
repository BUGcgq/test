#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "log.h"
#include "ftp.h"
#include "base64.h"


int main()
{
    const char* original_url = "ftp://inc02:za%40g0Pg6J@14.29.244.45:21/EVCM-SD10.tar.gz";
    // Base64编码URL
    char* encoded_url = base64_encode((const unsigned char*)original_url, strlen(original_url));
    if (encoded_url) {
        printf("Base64 Encoded URL: %s\n", encoded_url);
        free(encoded_url);
    }
    // Base64解码URL
    const char* base64_encoded_url = "ZnRwOi8vaW5jMDI6emElNDBnMFBnNkpAMTQuMjkuMjQ0LjQ1OjIxL0VWQ00tU0QxMC50YXIuZ3o=";
    char* decoded_url = base64_decode(base64_encoded_url);
    if (decoded_url) {
        printf("Base64 Decoded URL: %s\n", decoded_url);
        free(decoded_url);
    }

    return 0;
}