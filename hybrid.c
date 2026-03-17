#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *hybrid_thread(void *arg) {
    while (1) {
        printf("hybrid\n");
        sleep(1);
    }
    return NULL;
}