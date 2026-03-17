#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *event_logger_thread(void *arg) {
    while (1) {
        printf("event_logger\n");
        sleep(1);
    }
    return NULL;
}