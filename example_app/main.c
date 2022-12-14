#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../rtlsp/rtlsp.h"

int main(int argc, char *argv[]) {
    rtlsp_init(MAX, HIGH, "./bin", "./bin", SIGUSR1, SIGUSR2);
    // rtlsp_log("Hello, World!");
    // rtlsp_logf("Hello, %s!", "World");
    // rtlsp_logl("Hello, World!", HIGH);
    // rtlsp_loglf(HIGH, "Hello, %s!", "World");
    rtlsp_destroy();
    return 0;
}