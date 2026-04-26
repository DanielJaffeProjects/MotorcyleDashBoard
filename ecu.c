#include <stdio.h>
#include <string.h>
#include "state.h"

void *ecu_thread(void *arg)
{
     while (1)
     {
          // engine -> motion -> fuel -> ecu

          // Waiting for the signal
          pthread_mutex_lock(&ecu_lock);
          pthread_cond_wait(&ecu_cond, &ecu_lock);
          pthread_mutex_unlock(&ecu_lock);

          // Locking
          pthread_mutex_lock(&engine_lock);
          pthread_mutex_lock(&motion_lock);
          pthread_mutex_lock(&fuel_lock);

          // RPM
          if (global_state.engine_on == 0 || global_state.rpm == 0)
          {
               strcpy(global_state.rpm_zone, "IDLE");
          }
          else if (global_state.rpm < RPM_IDLE_MAX)
          {
               strcpy(global_state.rpm_zone, "IDLE");
          }
          else if (global_state.rpm < RPM_NORMAL_MAX)
          {
               strcpy(global_state.rpm_zone, "NORMAL");
          }
          else if (global_state.rpm < RPM_HIGH_MAX)
          {
               strcpy(global_state.rpm_zone, "HIGH");
          }
          else
          {
               strcpy(global_state.rpm_zone, "REDLINE");
          }
          // TEMP
          char prev_temp_zone[16];
          strcpy(prev_temp_zone, global_state.temp_zone);

          if (global_state.engine_temp < TEMP_COLD)
          {
               strcpy(global_state.temp_zone, "COLD");
          }
          else if (global_state.engine_temp < TEMP_NORMAL)
          {
               strcpy(global_state.temp_zone, "NORMAL");
          }
          else if (global_state.engine_temp < TEMP_HOT)
          {
               strcpy(global_state.temp_zone, "HOT");
          }
          else
          {
               strcpy(global_state.temp_zone, "OVERHEAT");
          }

          // Log if temperature zone changed to HOT or OVERHEAT
          if (strcmp(global_state.temp_zone, "HOT") == 0 &&
              strcmp(prev_temp_zone, "HOT") != 0)
          {
               enqueue_event("Engine temperature entered HOT zone");
          }
          if (strcmp(global_state.temp_zone, "OVERHEAT") == 0 &&
              strcmp(prev_temp_zone, "OVERHEAT") != 0)
          {
               enqueue_event("Engine temperature entered OVERHEAT zone");
          }

          // FUEL
          char prev_fuel_status[16];
          strcpy(prev_fuel_status, global_state.fuel_status);

          if (global_state.fuel_level <= FUEL_LOW)
          {
               strcpy(global_state.fuel_status, "LOW FUEL");
          }
          else
          {
               strcpy(global_state.fuel_status, "OK");
          }

          // Log if fuel dropped below threshold
          if (strcmp(global_state.fuel_status, "LOW FUEL") == 0 &&
              strcmp(prev_fuel_status, "LOW FUEL") != 0)
          {
               enqueue_event("Fuel dropped below threshold - LOW FUEL");
          }

          // Engine Off Behavior
          if (global_state.engine_on == 0)
          {
               if (global_state.rpm != 0)
               {
                    global_state.rpm = 0;
                    enqueue_event("ECU: Engine OFF - RPM set to 0");
               }


               // Signal motion thread to begin gradual speed reduction
               pthread_cond_signal(&speed_change_cond);
          }

          // Overheat Protection
          if (strcmp(global_state.temp_zone, "OVERHEAT") == 0)
          {
               int capped = 0;

               if (global_state.rpm > OVERHEAT_RPM_CAP)
               {
                    global_state.rpm = OVERHEAT_RPM_CAP;
                    capped = 1;
               }
               if (global_state.speed > OVERHEAT_SPEED_CAP)
               {
                    global_state.speed = (float)OVERHEAT_SPEED_CAP;
                    capped = 1;
               }
               if (capped)
               {
                    enqueue_event("ECU: OVERHEAT protection - RPM and speed capped");
               }
          }

          // Low Fuel Constraint
          if (strcmp(global_state.fuel_status, "LOW FUEL") == 0)
          {
               if (global_state.speed > LOW_FUEL_SPEED_CAP)
               {
                    global_state.speed = (float)LOW_FUEL_SPEED_CAP;
                    enqueue_event("ECU: LOW FUEL - speed limited to 60 MPH");
               }
          }

          // Idle State Enforcement
          if (global_state.engine_on == 1 && global_state.speed == 0)
          {
               if (global_state.rpm < RPM_IDLE_MIN)
               {
                    global_state.rpm = RPM_IDLE_MIN;
               }
               if (global_state.rpm > RPM_IDLE_MAX)
               {
                    global_state.rpm = RPM_IDLE_MAX;
               }
          }

          // System State Update
          ECUState prev_ecu_state = ecu_state;

          if (global_state.engine_on == 0)
          {
               ecu_state = ENGINE_OFF;
          }
          else if (global_state.speed == 0)
          {
               ecu_state = IDLE;
          }
          else if (strcmp(global_state.temp_zone, "OVERHEAT") == 0)
          {
               ecu_state = CRITICAL;
          }
          else if (global_state.rpm > RPM_HIGH_MAX)
          {
               ecu_state = HIGH_LOAD;
          }
          else
          {
               ecu_state = NORMAL;
          }

          // Log ECU state transitions
          if (ecu_state != prev_ecu_state)
          {
               if (ecu_state == CRITICAL)
                    enqueue_event("ECU state changed to CRITICAL");
               else if (ecu_state == HIGH_LOAD)
                    enqueue_event("ECU state changed to HIGH LOAD");
               else if (ecu_state == ENGINE_OFF)
                    enqueue_event("ECU state changed to ENGINE OFF");
          }

          // Release when done
          pthread_mutex_unlock(&fuel_lock);
          pthread_mutex_unlock(&motion_lock);
          pthread_mutex_unlock(&engine_lock);
     }

     return NULL;
}