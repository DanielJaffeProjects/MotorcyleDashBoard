#include <stdio.h>
#include <unistd.h>
#include "state.h"
#include <stdlib.h>
void get_speed() {
    // if accelerating, speed goes up twice as fast
    // also speed must be below 121
    if (global_state.accel_mode == 'A' && global_state.speed <120) {
        global_state.speed+=2;
        // add both trip distance and total distance
        global_state.trip_distance+=global_state.speed/3600;
        global_state.total_distance+=global_state.speed/3600;
    }
    // if decelerating speed goes down
    // also speed is higher than 0
    else if (global_state.accel_mode == 'D' && global_state.speed >0) {
        global_state.speed-=1;
        // add both trip distance and total distance
        global_state.trip_distance+=global_state.speed/3600;
        global_state.total_distance+=global_state.speed/3600;

    }
    //otherwise speed cant change
    else {
        // add both trip distance and total distance
        global_state.trip_distance+=global_state.speed/3600;
        global_state.total_distance+=global_state.speed/3600;
    }




}



void *motion_thread(void *arg) {
    while (1) {
        // wait for the engine to be on and not locked to start calculating speed
        pthread_mutex_lock(&engine_lock);
        while (global_state.engine_on==0) {
            pthread_cond_wait(&engine_on_cond, &engine_lock);
        }
        pthread_mutex_unlock(&engine_lock);

        pthread_mutex_lock(&motion_lock);
        get_speed();
        pthread_mutex_unlock(&motion_lock);
        sleep(1);
    }
    return NULL;
}