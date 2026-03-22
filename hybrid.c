#include <stdio.h>
#include <unistd.h>
#include "state.h"

void *hybrid_thread(void *arg) {
    float previous_speed = 0.0f;

    if (global_state.battery_level <= 0.0f) {
        global_state.battery_level = 100.0f;
    }

    while (1) {
        global_state.assist_active = 0;
        global_state.charging_active = 0;
        strcpy(global_state.hybrid_mode, "IDLE");

        if (global_state.engine_on == 0) {
            previous_speed = global_state.speed;
            usleep(100000);
            continue;
        }

        if (global_state.speed < previous_speed && global_state.speed > 0.0f) {
            global_state.charging_active = 1;
            strcpy(global_state.hybrid_mode, "CHARGING");
            global_state.battery_level += 0.3f;
        }

        else if (global_state.speed > 0.0f &&
                 global_state.speed <= 20.0f &&
                 global_state.battery_level > 20.0f) {
                    global_state.assist_active = 1;
                    strcpy(global_state.hybrid_mode, "ELECTRIC");
                    global_state.battery_level -= 0.2f;
                }
        else if ((global_state.speed > 20.0f || global_state.rpm > 5000) &&
                  global_state.battery_level > 20.0f) {
                    global_state.assist_active = 1;
                    strcpy(global_state.hybrid_mode, "ASSIST");
                    global_state.battery_level -= 0.4f;
                }
        if (global_state.battery_level > 100.0f) {
            global_state.battery_level = 100.0f;
        }
        else if (global_state.battery_level < 0.0f) {
            global_state.battery_level = 0.0f;
        }

        previous_speed = global_state.speed;
        usleep(100000);
    }
    return NULL;
}