#ifndef RTLSP_H
#define RTLSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>

#include "message.h"

typedef enum {
    MIN,
    STANDARD,
    MAX
} LOG_LEVEL;

typedef enum {
    LOW,
    MEDIUM,
    HIGH
} IMPORTANCE_LEVEL;

struct rtlsp {
    LOG_LEVEL llevel;
    char *log_path;
    char *dump_path;
    sem_t *sem_log;
    sem_t *sem_dump;
};

void rtlsp_init(LOG_LEVEL llevel, IMPORTANCE_LEVEL ilevel, char *log_path, char *dump_path, int sig1, int sig2);
void rtlsp_destroy();

void rtlsp_log(const char *msg);
void rtlsp_logf(const char *fmt, ...);
void rtlsp_logl(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *msg);
void rtlsp_loglf(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *fmt, ...);

void rtlsp_dump();

#endif // RTLSP_H