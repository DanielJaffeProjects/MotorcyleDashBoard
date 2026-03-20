#include <stdio.h>
#include <unistd.h>
#include "state.h"

// initialize the rpm direction to go up
static int rpm_direction = 1;

void update_RPM() {
    if (global_state.engine_on == 0) {
        global_state.rpm = 0;
        return;
    }
    // add or subtract 100 to the rpm
    global_state.rpm += rpm_direction * 100;
    // if the rpm is greater than 16500 than start lowering it
    if (global_state.rpm >= 16500) {
        rpm_direction = -1;
        //else it it is lower than 1100 start adding to it
    } else if (global_state.rpm <= 1100) {
        rpm_direction =  1;
    }
}

void update_Temp() {

    // if engine is on then it can go up to 120
    if (global_state.engine_on == 1) {
        //heat up if rpm is above 1300
        if (global_state.rpm > 8000) {
            global_state.engine_temp += 0.8f;
        } else if (global_state.rpm > 1300) {
            global_state.engine_temp += 0.2f;
        } else {
            // if motorcycle rpm is idle slowly lower the temp
            if (global_state.engine_temp > 80.0f)
                global_state.engine_temp -= 0.1f;
            else
                global_state.engine_temp += 0.05f;
        }

        // Cap at 120°C
        if (global_state.engine_temp > 120.0f)
            global_state.engine_temp = 120.0f;
    }
    // otherwise it is off and should go down slowly
    else {
        if (global_state.engine_temp>40) {
            global_state.engine_temp -=1.2;
        };
    }

}

void *engine_thread(void *arg) {
    global_state.engine_on = 1;
    global_state.rpm = 1100;
    global_state.engine_temp = 40;

    while (1) {
        // if engine is 1 then it is on else it is off
        if (global_state.engine_on == 1) {
            printf("ENGINE: ON \n");
        }
        else {
            printf("ENGINE: OFF \n");
        }
        update_RPM();
        printf("RPM: %d\n", global_state.rpm);
        update_Temp();
        printf("Temp: %f\n", global_state.engine_temp);

        sleep(1);

    }
    return NULL;
}
