#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../rtlsp/message.h"
#include "../rtlsp/rtlsp.h"

int main(void) {
    rtlsp_init(LOW, "./bin/", "./bin/", SIGRTMIN , SIGRTMIN+1);

    srand(time(NULL));
    int r;

    for (int i = 0; i < 100; i++) {
        rtlsp_loglf(MESSAGE_INFO, LOW, "rtlsp_logl() called %d", i);
        r = rand() % 100;
        if (r > 60) {
            r = rand() % 4;
            rtlsp_sig(getpid(), SIGRTMIN, r);
        }

        r = rand() % 100;
        if (r > 80) {
            rtlsp_sig(getpid(), SIGRTMIN+1, r);
        }
        sleep(1);
    }

    rtlsp_destroy();
    return 0;
}