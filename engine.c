#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *engine_thread(void *arg) {
    while (1) {
        printf("engine\n");
        sleep(1);
    }
    return NULL;
}