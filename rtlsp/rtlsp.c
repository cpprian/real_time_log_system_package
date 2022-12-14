#include "rtlsp.h"

static struct rtlsp rtlsp;


void rtlsp_init(LOG_LEVEL llevel, IMPORTANCE_LEVEL ilevel, char *log_path, char *dump_path, int sig1, int sig2) {
    rtlsp.llevel = llevel;
    rtlsp.ilevel = ilevel;
    rtlsp.log_path = log_path;
    rtlsp.dump_path = dump_path;

    printf("rtlsp_init() called\n");
    printf("llevel: %d\n", rtlsp.llevel);
    printf("ilevel: %d\n", rtlsp.ilevel);
    printf("log_path: %s\n", rtlsp.log_path);
    printf("dump_path: %s\n", rtlsp.dump_path);
}

void rtlsp_destroy() {
    printf("rtlsp_destroy() called\n");
}
