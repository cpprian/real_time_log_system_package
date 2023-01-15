#include <stdio.h>
#include <stdlib.h>
#include <signal.h>


int main(int argc, char *argv[]) {
    union sigval val = {.sival_int = 2};

    if (sigqueue(atoi(argv[1]), atoi(argv[2]), val) != 0) {
        printf("error sending signal\n");
    }
    return 0;
}