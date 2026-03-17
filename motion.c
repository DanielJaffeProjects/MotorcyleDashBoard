#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *motion_thread(void *arg) {
    while (1) {
        printf("motion\n");
        sleep(1);
    }
    return NULL;
}