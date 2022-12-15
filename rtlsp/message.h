#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MESSAGE_SIZE 1024

typedef enum {
    MESSAGE_INFO, 
    MESSAGE_ERROR,
    MESSAGE_WARNING,
    MESSAGE_CRITICAL
} MESSAGE_TYPE;

#define MSG_INFO        "INFO:"
#define MSG_ERROR       "ERROR:"
#define MSG_WARNING     "WARNING:"
#define MSG_CRITICAL    "CRITICAL:"
#define MSG_UNKNOWN     "UNKNOWN:"

#define ERR_NULL        "Null pointer"
#define ERR_ALLOC       "Failed to allocate memory"
#define ERR_SEM         "Failed to create semaphore"
#define ERR_SIG         "Failed to register signal handler"
#define ERR_FOPEN       "Failed to open file"
#define ERR_FCLOSE      "Failed to close file"
#define ERR_FWRITE      "Failed to write to file"
#define ERR_FREAD       "Failed to read from file"

#endif // MESSAGE_H