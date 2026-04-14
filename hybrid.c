// #include <stdio.h>
// #include <unistd.h>
// #include <string.h>
// #include "state.h"

// // Hybrid sync
// pthread_mutex_t hybrid_lock = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t speed_change_cond = PTHREAD_COND_INITIALIZER;

// void *hybrid_thread(void *arg)
// {
//     (void)arg;

//     float previous_speed = -1.0f;
//     char prev_mode[32] = "IDLE";

//     pthread_mutex_lock(&hybrid_lock);
//     if (global_state.battery_level <= 0.0f) {
//         global_state.battery_level = 100.0f;
//     }
//     pthread_mutex_unlock(&hybrid_lock);

//     while (1) {

//         pthread_mutex_lock(&motion_lock);

//         while (global_state.speed == previous_speed) {
//             pthread_cond_wait(&speed_change_cond, &motion_lock);
//         }

//         float speed = global_state.speed;
//         pthread_mutex_unlock(&motion_lock);

//         pthread_mutex_lock(&engine_lock);
//         int engine_on = global_state.engine_on;
//         int rpm = global_state.rpm;
//         pthread_mutex_unlock(&engine_lock);

//         pthread_mutex_lock(&hybrid_lock);

//         float battery = global_state.battery_level;

//         global_state.assist_active = 0;
//         global_state.charging_active = 0;
//         strcpy(global_state.hybrid_mode, "IDLE");

//         if (!engine_on) {
//             strcpy(global_state.hybrid_mode, "IDLE");
//         }
//         else if (battery <= BATTERY_LOW_THRESHOLD) {
//             strcpy(global_state.hybrid_mode, "INACTIVE");
//         }
//         else if (speed < previous_speed && speed > 0) {
//             global_state.charging_active = 1;
//             strcpy(global_state.hybrid_mode, "CHARGING");
//             global_state.battery_level += 1.0f;
//         }
//         else if (speed > 0 && speed <= 20) {
//             global_state.assist_active = 1;
//             strcpy(global_state.hybrid_mode, "ELECTRIC");
//             global_state.battery_level -= 0.5f;
//         }
//         else if (speed > previous_speed || rpm > 5000) {
//             global_state.assist_active = 1;
//             strcpy(global_state.hybrid_mode, "ASSIST");
//             global_state.battery_level -= 1.0f;
//         }

//         if (global_state.battery_level > 100.0f)
//             global_state.battery_level = 100.0f;

//         if (global_state.battery_level < 0.0f)
//             global_state.battery_level = 0.0f;

//         char current_mode[32];
//         strcpy(current_mode, global_state.hybrid_mode);

//         pthread_mutex_unlock(&hybrid_lock);

//         // Log hybrid mode change
//         if (strcmp(prev_mode, current_mode) != 0) {
//             char msg[128];
//             snprintf(msg, sizeof(msg), "Hybrid mode -> %s", current_mode);
//             enqueue_event(msg);
//             strcpy(prev_mode, current_mode);
//         }

//         previous_speed = speed;
//     }

//     return NULL;
// }



#include <stdio.h>
#include <unistd.h>
#include "state.h"
#include "string.h"

//This thread is looking at the speed and RPM and 
//deciding if the motocycle should use electric power 
//or assist the engine or charge the battery.
void *hybrid_thread(void *arg) {

    //Stored the previous speed to detect acceleration vs. deceleration
    float previous_speed = 0.0f;

    //Initializing the battery if not set already 
    if (global_state.battery_level <= 0.0f) {
        global_state.battery_level = 100.0f;
    }

    while (1) {
        //Reseting the hybrid system values each loop
        global_state.assist_active = 0;
        global_state.charging_active = 0;
        strcpy(global_state.hybrid_mode, "IDLE");

        //If the engine is off, the hybrid system does nothing 
        if (global_state.engine_on == 0) {
            previous_speed = global_state.speed;
            sleep(1);
            continue;
        }

        //If the speed is decreasing, the motorcycle is slowing down
        //This energy is converted into battery charge 
        if (global_state.speed < previous_speed && global_state.speed > 0.0f) {
            global_state.charging_active = 1;
            strcpy(global_state.hybrid_mode, "CHARGING");
            global_state.battery_level += 1.0f;
        }

        //At low speeds, the motorcycle will use electric power
        else if (global_state.speed > 0.0f &&
                 global_state.speed <= 20.0f &&
                 global_state.battery_level > 20.0f) {
                    global_state.assist_active = 1;
                    strcpy(global_state.hybrid_mode, "ELECTRIC");
                    global_state.battery_level -= 0.5f;
                }

        //When accelerating or RPM is high, electric will assist the engine 
        else if ((global_state.speed > previous_speed || global_state.rpm > 5000) &&
                  global_state.battery_level > 20.0f) {
                    global_state.assist_active = 1;
                    strcpy(global_state.hybrid_mode, "ASSIST");

                    //Draining the battery faster 
                    global_state.battery_level -= 1.0f;
                }

        //Having the battery level stay between 0% and 100%
        if (global_state.battery_level > 100.0f) {
            global_state.battery_level = 100.0f;
        }
        if (global_state.battery_level < 0.0f) {
            global_state.battery_level = 0.0f;
        }
        //Saving the current speed for next loop comparison
        previous_speed = global_state.speed;

        //Running at the same pase as the motion thread
        sleep(1);
    }
    return NULL;
}