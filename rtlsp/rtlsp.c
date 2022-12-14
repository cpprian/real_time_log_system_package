#include "rtlsp.h"

static struct rtlsp rtlsp;


void rtlsp_init(LOG_LEVEL llevel, IMPORTANCE_LEVEL ilevel, char *log_path, char *dump_path, int sig1, int sig2) {
    rtlsp.llevel = llevel;
    rtlsp.ilevel = ilevel;
    rtlsp.log_path = log_path;
    rtlsp.dump_path = dump_path;

    printf("rtlsp_init() called\n");
}

void rtlsp_destroy() {
    printf("rtlsp_destroy() called\n");
}

void rtlsp_log(const char *msg) {
    printf("rtlsp_log() called\n");
}

void rtlsp_logf(const char *fmt, ...) {
    printf("rtlsp_logf() called\n");
}

void rtlsp_logl(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *msg) {
    printf("rtlsp_logl() called\n");
}

void rtlsp_loglf(MESSAGE_TYPE mtype, IMPORTANCE_LEVEL ilevel, const char *fmt, ...) {
    printf("rtlsp_loglf() called\n");
}

void rtlsp_dump() {
    printf("rtlsp_dump() called\n");
}
