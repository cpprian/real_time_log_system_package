#include "rtlsp.h"

static struct rtlsp rtlsp;

void rtlsp_init(LOG_LEVEL llevel, IMPORTANCE_LEVEL ilevel, char *log_path, char *dump_path, int sig1, int sig2) {
    rtlsp.llevel = llevel;
    rtlsp.is_on = 1;

    char *log_filename = (char*)calloc(100, sizeof(char));
    if (log_filename == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        return;
    }

    time_t t = time(NULL);
    strcat(log_filename, "log_");
    strcat(log_filename, ctime(&t));
    strcat(log_filename, ".txt");

    rtlsp.log_path = (char*)calloc(strlen(log_path) + strlen(log_filename), sizeof(char));
    if (rtlsp.log_path == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        free(log_filename);
        return;
    }

    rtlsp.dump_path = dump_path;

    rtlsp.sem_log = sem_open("sem_log", O_CREAT, 0644, 1);
    rtlsp.sem_dump = sem_open("sem_dump", O_CREAT, 0644, 1);
    rtlsp.sem_is_on = sem_open("sem_is_on", O_CREAT, 0644, 1);

    if (rtlsp.sem_log == SEM_FAILED || rtlsp.sem_dump == SEM_FAILED || rtlsp.sem_is_on == SEM_FAILED) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SEM);
        free(log_filename);
        return;
    }

    pthread_create(&rtlsp.thread_dump, NULL, rtlsp_dump, NULL);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig1);
    rtlsp.sa_log.sa_sigaction = rtlsp_log_thread;
    rtlsp.sa_log.sa_mask = set;
    rtlsp.sa_log.sa_flags = SA_SIGINFO;

    if (sigaction(sig1, &rtlsp.sa_log, NULL) == -1) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
        free(log_filename);
        return;
    }

    rtlsp_logl(MESSAGE_INFO, LOW, "rtlsp_init() called");
    rtlsp_loglf(MESSAGE_INFO, LOW, "llevel: %d, log_path: %s, dump_path: %s, sig1: %d, sig2: %d", llevel, log_path, dump_path, sig1, sig2);
    rtlsp_loglf(MESSAGE_INFO, LOW, "sem_log: %p, sem_dump: %p", rtlsp.sem_log, rtlsp.sem_dump);

    free(log_filename);
}

void rtlsp_destroy() {
    sem_close(rtlsp.sem_log);
    sem_close(rtlsp.sem_dump);

    free(rtlsp.log_path);

    pthread_exit(&rtlsp.thread_log);
    pthread_exit(&rtlsp.thread_dump);
}

void rtlsp_log(const char *msg) {
    if (msg == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_NULL);
        return;
    }
    
    rtlsp_logl(MESSAGE_INFO, MEDIUM, msg);
}

void rtlsp_logf(const char *fmt, ...) {
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

    rtlsp_logl(MESSAGE_INFO, rtlsp.llevel, msg);

    free(msg);
}

void rtlsp_logl(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *msg) {
    if (ilevel < rtlsp.llevel) {
        return;
    }
    if (msg == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_NULL);
        return;
    }

    sem_wait(rtlsp.sem_log);

    FILE *log_file = fopen(rtlsp.log_path, "a");
    if (log_file == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FOPEN);
        return;
    }

    time_t t = time(NULL);

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

    fprintf(log_file, "[%s] %s %s\n", ctime(&t), mtype_str, msg);
    if (fclose(log_file) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FCLOSE);
    }

    sem_post(rtlsp.sem_log);
}

void rtlsp_loglf(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *fmt, ...) {
    if (ilevel < rtlsp.llevel) {
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
    sem_wait(rtlsp.sem_dump);

    char *dump_file_name = malloc(MAX_MESSAGE_SIZE);
    if (dump_file_name == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        return NULL;
    }

    time_t t = time(NULL);

    strcat(dump_file_name, rtlsp.dump_path);
    strcat(dump_file_name, "dump_");
    strcat(dump_file_name, ctime(&t));
    strcat(dump_file_name, ".txt");

    FILE *dump_file = fopen(rtlsp.dump_path, "a");
    if (dump_file == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FOPEN);
        free(dump_file_name);
        return NULL;
    }

    execv("hexdump", (char *[]){"hexdump", "-v", "-e" "'\\' 'x' 1/1 '%02X'", "\n", rtlsp.dump_path, NULL});

    if (fclose(dump_file) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FCLOSE);
    }

    free(dump_file_name);
    sem_post(rtlsp.sem_dump);
    return NULL;
}

void rtlsp_log_thread(int signo, siginfo_t* info, void* other) {
    sem_wait(rtlsp.sem_is_on);
    while (1) {
        switch (info->si_value.sival_int) {
            case 0:
                rtlsp.llevel = LOW; 
                break;
            case 1:
                rtlsp.llevel = MEDIUM;
                break;
            case 2:
                rtlsp.llevel = HIGH;
                break;
            case 3:
                if (rtlsp.is_on) {
                    rtlsp.is_on = 0;
                } else {
                    rtlsp.is_on = 1;
                }
                break;
            default:
                rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
                break;
        }
    }
    sem_post(rtlsp.sem_is_on);
}