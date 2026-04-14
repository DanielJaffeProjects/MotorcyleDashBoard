#include <stdio.h>
#include <string.h>
#include "state.h"

void *ecu_thread(void *arg)
{

     pthread_mutex_lock(&ecu_lock);

     while (1)
     {

          // Waiting for the signal
          pthread_cond_wait(&ecu_cond, &ecu_lock);

          // Locking
          pthread_mutex_lock(&engine_lock);
          pthread_mutex_lock(&motion_lock);
          pthread_mutex_lock(&fuel_lock);


          // RPM
          if (global_state.engine_on == 0 || global_state.rpm == 0)
          {
               strcpy(global_state.rpm_zone, "IDLE");
          }
          else if (global_state.rpm < (int)(RPM_MAX * 0.08))
          {
               strcpy(global_state.rpm_zone, "IDLE");
          }
          else if (global_state.rpm < (int)(RPM_MAX * 0.5))
          {
               strcpy(global_state.rpm_zone, "NORMAL");
          }
          else if (global_state.rpm < (int)(RPM_MAX * 0.85))
          {
               strcpy(global_state.rpm_zone, "HIGH");
          }
          else
          {
               strcpy(global_state.rpm_zone, "REDLINE");
          }

          // TEMP
          if (global_state.engine_temp < 60.0f)
          {
               strcpy(global_state.temp_zone, "COLD");
          }
          else if (global_state.engine_temp < 95.0f)
          {
               strcpy(global_state.temp_zone, "NORMAL");
          }
          else if (global_state.engine_temp < 105.0f)
          {
               strcpy(global_state.temp_zone, "HOT");
          }
          else
          {
               strcpy(global_state.temp_zone, "OVERHEAT");
          }

          // FUEL
          if (global_state.fuel_level <= FUEL_LOW)
          {
               strcpy(global_state.fuel_status, "LOW FUEL");
          }
          else
          {
               strcpy(global_state.fuel_status, "OK");
          }

          // Engine Off Behavior
          if (global_state.engine_on == 0)
          {

               global_state.rpm = 0;

               // Slowly reduce speed
               if (global_state.speed > 0)
               {
                    global_state.speed -= 2.0f;
                    if (global_state.speed < 0)
                         global_state.speed = 0;
               }

               // No more distance
               if (global_state.speed == 0)
               {
                    // Do nothing
               }
          }

          // Overheat Protection
          int overheat_rpm_cap   = (int)(RPM_MAX * 0.5);
          int overheat_speed_cap = (int)(SPEED_MAX * 0.4);
          if (strcmp(global_state.temp_zone, "OVERHEAT") == 0)
          {

               // Cap RPM
               if (global_state.rpm > overheat_rpm_cap)
                    global_state.rpm = overheat_rpm_cap;

               // Cap speed
               if (global_state.speed > overheat_speed_cap)
                    global_state.speed = overheat_speed_cap;
          }

          // Low Fuel Constraint
          if (strcmp(global_state.fuel_status, "LOW FUEL") == 0)
          {

               if (global_state.speed > 60)
                    global_state.speed = 60;
          }

          // Idle State Enforcement
          if (global_state.engine_on == 1 && global_state.speed == 0)
          {
               int idle_min = (int)(RPM_MAX * 0.07);
               int idle_max = (int)(RPM_MAX * 0.09);
               if (global_state.rpm < idle_min)
                    global_state.rpm = idle_min;
               if (global_state.rpm > idle_max)
                    global_state.rpm = idle_max;
          }

          // System State Update
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
          else if (global_state.rpm > 10000)
          {
               ecu_state = HIGH_LOAD;
          }
          else
          {
               ecu_state = NORMAL;
          }

          // Release when done
          pthread_mutex_unlock(&fuel_lock);
          pthread_mutex_unlock(&motion_lock);
          pthread_mutex_unlock(&engine_lock);
     }

     pthread_mutex_unlock(&ecu_lock);
     return NULL;
}