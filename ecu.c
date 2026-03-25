#include <stdio.h>
#include <unistd.h>
#include "state.h"
#include <string.h>

void *ecu_thread(void *arg) {
    while (1) {
    
        //If the engine is off, force the rpm zone into IDLE
       if (global_state.engine_on == 0 || global_state.rpm == 0) {
            strcpy(global_state.rpm_zone, "IDLE");
       }
       else if (global_state.rpm >= 1100 && global_state.rpm < 1300) {
            strcpy(global_state.rpm_zone, "IDLE");
       }
       //Zones for when the engine is turned on
       else if (global_state.rpm >= 1300 && global_state.rpm < 8000) {
            strcpy(global_state.rpm_zone, "NORMAL");
       }
       else if (global_state.rpm >= 8000 && global_state.rpm < 14500) {
            strcpy(global_state.rpm_zone, "HIGH");
       }
       else {
            strcpy(global_state.rpm_zone, "REDLINE");
       }

       //Temperature zone logic
       if (global_state.engine_temp < 60.0f) {
            strcpy(global_state.temp_zone, "COLD");
       }
       else if (global_state.engine_temp < 95.0f) {
            strcpy(global_state.temp_zone, "NORMAL");
       }
       else if (global_state.engine_temp < 105.0f) {
            strcpy(global_state.temp_zone, "HOT");
       }
       else {
            strcpy(global_state.temp_zone, "OVERHEAT");
       }


       //The fuel status logic
       if (global_state.fuel_level <= FUEL_LOW) {
            strcpy(global_state.fuel_status, "LOW FUEL");
       }
       else {
        strcpy(global_state.fuel_status, "OK");
       }

       sleep(1);

    }
    return NULL;
}