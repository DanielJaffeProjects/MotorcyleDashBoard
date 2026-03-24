#include <stdio.h>
#include <unistd.h>
#include "state.h"
#include <stdlib.h>
void get_speed() {
    // go up in speed till 80
    for (int i = 0; i < 30; i++) {
        global_state.speed+=1;
        // add both trip distance and total distance
        global_state.trip_distance+=global_state.speed/3600;
        global_state.total_distance+=global_state.speed/3600;
        sleep(1);
    }
    // go back down in speed till 50
    for (int i = 0; i < 30; i++) {
        global_state.speed-=1;
        // add both trip distance and total distance
        global_state.trip_distance+=global_state.speed/3600;
        global_state.total_distance+=global_state.speed/3600;
        sleep(1);
    }
}



void *motion_thread(void *arg) {
    while (1) {
        get_speed();
        printf("Speed %f", global_state.speed);
        printf("Trip Distance %f",global_state.trip_distance);
        printf("Total Distance %f",global_state.total_distance);
        sleep(1);
    }
    return NULL;
}