#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include "state.h"


void getFuel() {
    // if off then don't do anything
    if (global_state.engine_on==0) {
        return;
    }
    // start lowering fuel by levels
    else {
        if (global_state.rpm>8000) {
            global_state.fuel_level-=0.02;
        }
        else if (global_state.rpm>1300) {
            global_state.fuel_level-=0.01;
        }
        else {
            global_state.fuel_level-=0.005;
        }
    }
}
void *fuel_thread(void *arg) {
    global_state.fuel_level= 4.7;

    while (1) {
        getFuel();
        printf("FUEL %f",global_state.fuel_level);
        sleep(1);
    }
    return NULL;
}