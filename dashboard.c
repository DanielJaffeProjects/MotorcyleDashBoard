#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *dashboard_thread(void *arg) {
    while (1) {
        printf("dashboard\n");
        sleep(1);
    }
    return NULL;
}