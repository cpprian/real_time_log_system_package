#include "rtlsp.h"

static struct rtlsp rtlsp;


void rtlsp_init(LOG_LEVEL llevel, IMPORTANCE_LEVEL ilevel, char *log_path, char *dump_path, int sig1, int sig2) {
    rtlsp.llevel = llevel;
    rtlsp.log_path = log_path;
    rtlsp.dump_path = dump_path;

    rtlsp.sem_log = sem_open("sem_log", O_CREAT, 0644, 1);
    rtlsp.sem_dump = sem_open("sem_dump", O_CREAT, 0644, 1);


    // TODO: połącz sygnały ze zdarzeniami
    signal(sig1, rtlsp_dump);
    signal(sig2, rtlsp_destroy);

    rtlsp_logl(MESSAGE_INFO, LOW, "rtlsp_init() called");
    rtlsp_loglf(MESSAGE_INFO, LOW, "llevel: %d, log_path: %s, dump_path: %s, sig1: %d, sig2: %d", llevel, log_path, dump_path, sig1, sig2);
    rtlsp_loglf(MESSAGE_INFO, LOW, "sem_log: %p, sem_dump: %p", rtlsp.sem_log, rtlsp.sem_dump);
}

void rtlsp_destroy() {
    sem_close(rtlsp.sem_log);
    sem_close(rtlsp.sem_dump);
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

    if (mtype == MESSAGE_ERROR) {
        rtlsp_dump();
    }
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

    if (mtype == MESSAGE_ERROR) {
        rtlsp_dump();
    }
}

void rtlsp_dump() {
    printf("rtlsp_dump() called\n");

    sem_wait(rtlsp.sem_dump);

    FILE *dump_file = fopen(rtlsp.dump_path, "a");
    if (dump_file == NULL) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FOPEN);
        return;
    }

    time_t t = time(NULL);

    fprintf(dump_file, "%s\n", ctime(&t));

    if (fclose(dump_file) != 0) {
        rtlsp_logl(MESSAGE_ERROR, LOW, ERR_FCLOSE);
    }

    sem_post(rtlsp.sem_dump);

    if (rtlsp.llevel == HIGH) {
        rtlsp_destroy();
    }
}
