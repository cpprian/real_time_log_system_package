#ifndef RTLSP_H
#define RTLSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>

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
    int is_on;
    int sig_config;
    int sig_dump;
    LOG_LEVEL llevel;
    char *log_path;
    char *dump_path;
    // semaphore for changing the log level and turning on/off the logger
    sem_t sem_log;
    // semaphore for saving a new dump
    sem_t sem_dump;
    // semaphore for writing to a file
    sem_t sem_write;
    pthread_t thread_log;
    pthread_t thread_dump;
    struct sigaction sa_log;
};

void rtlsp_init(LOG_LEVEL llevel, char *log_path, char *dump_path, int sig1, int sig2);
void rtlsp_destroy();

void rtlsp_log(const char *msg);
void rtlsp_logf(const char *fmt, ...);
void rtlsp_logl(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *msg);
void rtlsp_loglf(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *fmt, ...);

void* rtlsp_dump(void* arg);
void* rtlsp_log_config(void* arg);

void rtlsp_signal_log_config(int signo, siginfo_t *info, void *other);
void rtlsp_signal_dump(int signo, siginfo_t *info, void *other);
void rtlsp_sig(int pid, int signo, int value);

#endif // RTLSP_H