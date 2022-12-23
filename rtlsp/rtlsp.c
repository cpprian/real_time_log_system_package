#include "rtlsp.h"

static struct rtlsp rtlsp;

char* generate_log_filename(char* log_path) {
    char *log_filename = (char*)calloc(100, sizeof(char));
    if (log_filename == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        return NULL;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(log_filename, "log_%d-%d-%d_%d:%d:%d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return log_filename;
}

void rtlsp_init(LOG_LEVEL llevel, char *log_path, char *dump_path, int sig1, int sig2) {
    rtlsp.llevel = llevel;
    rtlsp.is_on = 1;
    rtlsp.dump_path = dump_path;

    rtlsp.sem_log = sem_open("sem_log", O_CREAT, 0644, 1);
    rtlsp.sem_dump = sem_open("sem_dump", O_CREAT, 0644, 1);
    rtlsp.sem_is_on = sem_open("sem_is_on", O_CREAT, 0644, 1);

    rtlsp.log_path = generate_log_filename(log_path);
    if (rtlsp.log_path == NULL) {
        return;
    }

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig1);
    rtlsp.sa_log.sa_sigaction = rtlsp_log_config;
    rtlsp.sa_log.sa_mask = set;
    rtlsp.sa_log.sa_flags = SA_SIGINFO;

    if (sigaction(sig1, &rtlsp.sa_log, NULL) == -1) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
        return;
    }

    sigset_t set2;
    sigemptyset(&set2);
    sigaddset(&set2, sig2);
    rtlsp.sa_log.sa_sigaction = rtlsp_dump;
    rtlsp.sa_log.sa_mask = set2;
    rtlsp.sa_log.sa_flags = SA_SIGINFO;

    if (sigaction(sig2, &rtlsp.sa_log, NULL) == -1) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
        return;
    }

    sem_post(rtlsp.sem_log);
    sem_post(rtlsp.sem_dump);
    sem_post(rtlsp.sem_is_on);

    rtlsp_logl(MESSAGE_INFO, LOW, "rtlsp_init() called");
}

void rtlsp_destroy() {
    sem_close(rtlsp.sem_log);
    sem_close(rtlsp.sem_dump);

    sem_unlink("sem_log");
    sem_unlink("sem_dump");
    sem_unlink("sem_is_on");

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

    rtlsp_logl(MESSAGE_INFO, rtlsp.llevel, msg);

    free(msg);
}

void rtlsp_logl(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *msg) {
    if (!rtlsp.is_on) {
        return;
    }
    if (ilevel < rtlsp.llevel) {
        return;
    }
    if (msg == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_NULL);
        return;
    }

    sem_wait(rtlsp.sem_log);

    FILE *log_file = fopen(rtlsp.log_path, "a+");
    if (log_file == NULL) {
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

    char* date_now = (char*)calloc(100, sizeof(char));
    if (date_now == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(date_now, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fprintf(log_file, "[%s] %s %s\n", date_now, mtype_str, msg);
    if (fclose(log_file) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FCLOSE);
    }

    free(date_now);
    sem_post(rtlsp.sem_log);
}

void rtlsp_loglf(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *fmt, ...) {
    if (!rtlsp.is_on) {
        return;
    }
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

void rtlsp_dump(int signo, siginfo_t* info, void* other) {
    if (!rtlsp.is_on) {
        return;
    }
    sem_wait(rtlsp.sem_dump);

    printf("Signal %d caught\n", signo);
    char *dump_file_name = (char*)calloc(MAX_MESSAGE_SIZE, sizeof(char));
    if (dump_file_name == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    sprintf(dump_file_name, "dump_%d-%d-%d_%d:%d:%d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    FILE *dump_file = fopen(dump_file_name, "a");
    if (dump_file == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FOPEN);
        free(dump_file_name);
        return;
    }

    FILE* proc_maps = fopen("/proc/self/root/proc/self/stack", "rb");
    if (proc_maps == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FOPEN);
        free(dump_file_name);
        return;
    }

    char *line = (char*)calloc(16, sizeof(char));
    if (line == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_ALLOC);
        free(dump_file_name);
        return;
    }

    int npos, c = 0;
    while (fread(line, 1, sizeof(line), proc_maps) > 0) {
        fprintf(dump_file, "%04x", npos);
        npos += 16;

        for (int i = 0; i < 16; i++) {
            fprintf(dump_file, " %02x", line[i]);
        }

        fprintf(dump_file, "  ");

        for (int i = 0; i < 16; i++) {
            c = line[i];
            fprintf(dump_file, "%c", (c < 32 || c > 126) ? '.' : c);
        }

        fprintf(dump_file, "\n");
    }

    if (fclose(dump_file) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FCLOSE);
    }

    if (fclose(proc_maps) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FCLOSE);
    }

    rtlsp_logl(MESSAGE_WARNING, LOW, "Dump created");
    free(dump_file_name);
    free(line);
    sem_post(rtlsp.sem_dump);
}

void rtlsp_log_config(int signo, siginfo_t* info, void* other) {
    sem_wait(rtlsp.sem_is_on);
    switch (info->si_value.sival_int) {
        case 0:
            rtlsp.llevel = LOW;
            printf("Log level set to LOW\n");
            break;
        case 1:
            rtlsp.llevel = MEDIUM;
            printf("Log level set to MEDIUM\n");
            break;
        case 2:
            rtlsp.llevel = HIGH;
            printf("Log level set to HIGH\n");
            break;
        case 3:
            if (rtlsp.is_on) {
                rtlsp.is_on = 0;
            } else {
                rtlsp.is_on = 1;
            }
            printf("Log is %s\n", rtlsp.is_on ? "on" : "off");
            break;
        default:
            rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
            break;
    }
    sem_post(rtlsp.sem_is_on);
}

void rtlsp_sig(int pid, int signo, int value) {
    union sigval val;
    val.sival_int = value;
    if (sigqueue(pid, signo, val) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_SIG);
    }
}