#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../rtlsp/message.h"
#include "../rtlsp/rtlsp.h"

int main(void) {
    rtlsp_init(LOW, "./example_app/bin/", "./example_app/bin/", SIGRTMIN , SIGRTMIN+1);

    srand(time(NULL));
    int r;
    printf("pid = %d\n", getpid());

    for (int i = 0; i < 1000; i++) {
        r = rand() % 100;
        rtlsp_loglf(MESSAGE_INFO, LOW, "rtlsp_logl() called %d with value: %d", i, r);
        printf("rtlsp_logl() called %d with value: %d \n", i, r);
        sleep(1);
    }

    rtlsp_destroy();
    return 0;
}