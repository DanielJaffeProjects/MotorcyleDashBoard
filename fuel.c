#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *fuel_thread(void *arg) {
    while (1) {
        printf("fuel\n");
        sleep(1);
    }
    return NULL;
}