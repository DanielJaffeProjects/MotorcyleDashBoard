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
        if (global_state.fuel_level>0) {
            if (global_state.rpm>8000) {
                global_state.fuel_level-=0.02f;
            }
            else if (global_state.rpm>1300) {
                global_state.fuel_level-=0.01f;
            }
            else {
                global_state.fuel_level-=0.005f;
            }
        }

    }
    if (global_state.fuel_level<= 0.0f) {
        // stops fuel level from going negative
        global_state.fuel_level=0.0f;

        // lock down engine when fuel is 0 and turn it off
        pthread_mutex_lock(&engine_lock);
        global_state.engine_on = 0;
        pthread_mutex_unlock(&engine_lock);
    }

}
void *fuel_thread(void *arg) {

    while (global_state.shutdown_flag == 0) {
        pthread_mutex_lock(&engine_lock);
        // if engine si off or battery is on don't use fuel
        while ((global_state.engine_on == 0 || global_state.battery_mode) &&
            global_state.shutdown_flag == 0) {
            pthread_cond_wait(&engine_on_cond, &engine_lock);
        }
        pthread_mutex_unlock(&engine_lock);

        if (global_state.shutdown_flag) {
            break;
        }

        pthread_mutex_lock(&fuel_lock);
        getFuel();
        pthread_mutex_unlock(&fuel_lock);

        sleep(1);
    }
    return NULL;
}