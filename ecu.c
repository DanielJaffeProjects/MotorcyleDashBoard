#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *ecu_thread(void *arg) {
    while (1) {
        printf("ecu\n");
        sleep(1);
    }
    return NULL;
}