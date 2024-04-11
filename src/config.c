#include "config.h"


int saveJsonToFile(const char *jsonStr, const char *filePath)
{
    cJSON *json = cJSON_Parse(jsonStr);
    if (json == NULL)
    {
        printf("Error: Invalid JSON data.\n");
        return -1;
    }

    int file = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (file == -1)
    {
        printf("Error: Failed to open file '%s' for writing.\n", filePath);
        cJSON_Delete(json);
        return -1;
    }

    if (flock(file, LOCK_EX) == -1)
    {
        printf("Error: Failed to acquire file lock.\n");
        close(file);
        cJSON_Delete(json);
        return -1;
    }

    lseek(file, 0, SEEK_SET);

    char *jsonFormattedStr = cJSON_Print(json);
    ssize_t bytesWritten = write(file, jsonFormattedStr, strlen(jsonFormattedStr));
    free(jsonFormattedStr);

    if (bytesWritten == -1)
    {
        printf("Error: Failed to write JSON data to the file.\n");
    }

    flock(file, LOCK_UN); // 释放文件锁
    close(file);
    cJSON_Delete(json);

    return bytesWritten == -1 ? -1 : 0;
}

char *readJsonFromFile(const char *filePath)
{
    int file = open(filePath, O_RDONLY);

    if (file == -1)
    {
        printf("Error: Failed to open file '%s' for reading.\n", filePath);
        return NULL;
    }

    if (flock(file, LOCK_SH) == -1)
    {
        printf("Error: Failed to acquire file lock for reading.\n");
        close(file);
        return NULL;
    }

    // 获取文件大小
    off_t fileSize = lseek(file, 0, SEEK_END);
    lseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(fileSize + 1);

    if (buffer == NULL)
    {
        printf("Error: Failed to allocate memory.\n");
        flock(file, LOCK_UN); // 释放文件锁
        close(file);
        return NULL;
    }

    ssize_t bytesRead = read(file, buffer, fileSize);

    if (bytesRead != fileSize)
    {
        printf("Error: Failed to read file '%s'.\n", filePath);
        free(buffer);
        flock(file, LOCK_UN); // 释放文件锁
        close(file);
        return NULL;
    }

    buffer[fileSize] = '\0';
    flock(file, LOCK_UN); // 释放文件锁
    close(file);

    return buffer;
}
