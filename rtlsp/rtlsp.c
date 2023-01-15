#include "rtlsp.h"

static struct rtlsp rtlsp;
volatile sig_atomic_t sig_config;

char* generate_log_filename(char* log_path) {
    char *log_filename = (char*)calloc(100, sizeof(char));
    if (log_filename == NULL) {
        printf("generate_log_filename: %s\n", ERR_ALLOC);
        return NULL;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(log_filename, "%s/log_%d-%d-%d_%d:%d:%d.txt", log_path, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return log_filename;
}

void rtlsp_init(LOG_LEVEL llevel, char *log_path, char *dump_path, int sig1, int sig2) {
    rtlsp.llevel = llevel;
    rtlsp.is_on = 1;
    rtlsp.dump_path = dump_path;
    rtlsp.sig_config = sig1;
    rtlsp.sig_dump = sig2;

    sem_init(&rtlsp.sem_log, 0, 0);
    sem_init(&rtlsp.sem_dump, 0, 0);
    sem_init(&rtlsp.sem_write, 0, 1);

    rtlsp.log_path = generate_log_filename(log_path);
    if (rtlsp.log_path == NULL) {
        return;
    }

    // SET CONFIG LOG SIGNAL
    sigset_t set;
    sigemptyset(&set);
    sigfillset(&set);
    rtlsp.sa_log.sa_sigaction = rtlsp_signal_log_config;
    rtlsp.sa_log.sa_mask = set;
    rtlsp.sa_log.sa_flags = SA_SIGINFO;
    if (sigaction(sig1, &rtlsp.sa_log, NULL) == -1) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
        return;
    }

    // SET DUMP LOG SIGNAL
    sigset_t set2;
    sigemptyset(&set2);
    sigfillset(&set2);
    rtlsp.sa_log.sa_sigaction = rtlsp_signal_dump;
    rtlsp.sa_log.sa_mask = set2;
    rtlsp.sa_log.sa_flags = SA_SIGINFO;
    if (sigaction(sig2, &rtlsp.sa_log, NULL) == -1) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
        return;
    }

    // CREATE LOG THREAD
    if (pthread_create(&rtlsp.thread_log, NULL, rtlsp_log_config, NULL) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_THREAD);
        return;
    }

    // CREATE DUMP THREAD
    if (pthread_create(&rtlsp.thread_dump, NULL, rtlsp_dump, NULL) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_THREAD);
        return;
    }

    if (pthread_detach(rtlsp.thread_log) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_THREAD);
        return;
    }

    if (pthread_detach(rtlsp.thread_dump) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_THREAD);
        return;
    }

    rtlsp_logl(MESSAGE_INFO, LOW, "rtlsp_init() called");
}

void rtlsp_destroy() {
    rtlsp.is_on = 0;

    sem_destroy(&rtlsp.sem_log);
    sem_destroy(&rtlsp.sem_dump);
    sem_destroy(&rtlsp.sem_write);

    pthread_cancel(rtlsp.thread_log);
    pthread_cancel(rtlsp.thread_dump);

    free(rtlsp.log_path);
}

void rtlsp_log(const char *msg) {
    if (!rtlsp.is_on) {
        return;
    }
    if (msg == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_NULL);
        return;
    }

    rtlsp_logl(MESSAGE_INFO, MEDIUM, msg);
}

void rtlsp_logf(const char *fmt, ...) {
    if (!rtlsp.is_on) {
        return;
    }
    if (fmt == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_NULL);
        return;
    }

    va_list args;
    va_start(args, fmt);
    char *msg = malloc(MAX_MESSAGE_SIZE);
    if (msg == NULL) {
        va_end(args);
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        return;
    }

    vsnprintf(msg, MAX_MESSAGE_SIZE, fmt, args);
    va_end(args);

    rtlsp_logl(MESSAGE_INFO, MEDIUM, msg);

    free(msg);
}

void rtlsp_logl(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *msg) {
    if (!rtlsp.is_on) {
        return;
    }
    if (ilevel < (IMPORTANCE_LEVEL)rtlsp.llevel) {
        return;
    }
    if (msg == NULL) {
        return;
    }

    char *mtype_str;
    switch (mtype) {
        case MESSAGE_INFO:
            mtype_str = MSG_INFO;
            break;
        case MESSAGE_WARNING:
            mtype_str = MSG_WARNING;
            break;
        case MESSAGE_ERROR:
            mtype_str = MSG_ERROR;
            break;
        default:
            mtype_str = MSG_UNKNOWN;
            break;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char* date_now = (char*)calloc(100, sizeof(char));
    if (date_now == NULL) {
        return;
    }
    sprintf(date_now, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    
    sem_wait(&rtlsp.sem_write);
    FILE *log_file = fopen(rtlsp.log_path, "a+");
    if (log_file == NULL) {
        printf("Error opening file!\n");
        return;
    }
    fprintf(log_file, "[%s] %s %s\n", date_now, mtype_str, msg);

    fclose(log_file);
    sem_post(&rtlsp.sem_write);

    free(date_now);
}

void rtlsp_loglf(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *fmt, ...) {
    if (!rtlsp.is_on) {
        return;
    }
    if (ilevel < (IMPORTANCE_LEVEL)rtlsp.llevel) {
        return;
    }
    if (fmt == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_NULL);
        return;
    }

    va_list args;
    va_start(args, fmt);
    char *msg = malloc(MAX_MESSAGE_SIZE);
    if (msg == NULL) {
        va_end(args);
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        return;
    }

    vsnprintf(msg, MAX_MESSAGE_SIZE, fmt, args);
    va_end(args);

    rtlsp_logl(mtype, ilevel, msg);

    free(msg);
}

void* rtlsp_dump(void* arg) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, rtlsp.sig_dump);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    while (1) {
        sem_wait(&rtlsp.sem_dump);

        char *dump_file_name = (char*)calloc(MAX_MESSAGE_SIZE, sizeof(char));
        if (dump_file_name == NULL) {
            rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
            return NULL;
        }

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        sprintf(dump_file_name, "%s/dump_%d-%d-%d_%d:%d:%d.txt", rtlsp.dump_path, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

        FILE *dump_file = fopen(dump_file_name, "a");
        if (dump_file == NULL) {
            rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FOPEN);
            free(dump_file_name);
            return NULL;
        }

        fprintf(dump_file, "Dump file created at %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fprintf(dump_file, "LOG FILE: %s\n", rtlsp.log_path);
        fprintf(dump_file, "DUMP FILE: %s\n", dump_file_name);
        fprintf(dump_file, "LOG IS %s\n", rtlsp.is_on ? "ON" : "OFF");
        fprintf(dump_file, "LOG LEVEL: %d\n", rtlsp.llevel);

        if (fclose(dump_file) != 0) {
            rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FCLOSE);
        }

        free(dump_file_name);
        rtlsp_logl(MESSAGE_WARNING, LOW, "Dump created");
    }

    return NULL;
}

void* rtlsp_log_config(void* arg) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, rtlsp.sig_config);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    while(1) {

        sem_wait(&rtlsp.sem_log);
        switch (sig_config) {
            case 0:
                rtlsp.llevel = MIN;
                rtlsp_logl(MESSAGE_INFO, LOW, "Log level set to MIN");
                break;
            case 1:
                rtlsp.llevel = STANDARD;
                rtlsp_logl(MESSAGE_INFO, LOW, "Log level set to STANDARD");
                break;
            case 2:
                rtlsp.llevel = MAX;
                rtlsp_logl(MESSAGE_INFO, LOW, "Log level set to MAX");
                break;
            case 3:
                if (rtlsp.is_on) {
                    rtlsp_logl(MESSAGE_INFO, LOW, "Log turned off");
                    rtlsp.is_on = 0;
                } else {
                    rtlsp.is_on = 1;
                    rtlsp_logl(MESSAGE_INFO, LOW, "Log turned on");
                }
                break;
            default:
                break;
        }
    }

    return NULL;
}

// signal handler for rtlsp_config
void rtlsp_signal_log_config(int signo, siginfo_t *info, void *other) {
    sem_post(&rtlsp.sem_log);
    sig_config = info->si_value.sival_int;
}

// signal handler for rtlsp_dump
void rtlsp_signal_dump(int signo, siginfo_t *info, void *other) {
    sem_post(&rtlsp.sem_dump);
}

void rtlsp_sig(int pid, int signo, int value) {
    union sigval val = {.sival_int = value};
    if (sigqueue(pid, signo, val) != 0) {
        printf("rtlsp_sig: Error sending signal\n");
    }
}