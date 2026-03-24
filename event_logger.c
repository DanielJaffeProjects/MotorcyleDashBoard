#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "state.h"

void log_event(const char *desc)
{
    int index = global_state.event_count % MAX_EVENTS;

    // Replacing the oldest event with the new one
    LogEntry *e = &global_state.event_log[index];
    strncpy(e->description, desc, 127);
    e->description[127] = '\0';
    e->elapsed_seconds = global_state.current_elapsed_sec;

    global_state.event_count++;
}

void *event_logger_thread(void *arg)
{

    // Keeping track of the previous events
    char prev_rpm_zone[16] = "";
    char prev_temp_zone[16] = "";
    char prev_hybrid_mode[32] = "";
    int prev_engine_on = -1;
    int fuel_warned = 0;
    int battery_warned = 0;
    int overheat_warned = 0;
    char msg[128];

    while (1)
    {

        // Engine On or Off
        if (prev_engine_on != global_state.engine_on)
        {
            if (global_state.engine_on)
                log_event("Engine turned ON");
            else
                log_event("Engine turned OFF");
            prev_engine_on = global_state.engine_on;
        }

        // RPM entering a new operating zone
        if (strcmp(global_state.rpm_zone, prev_rpm_zone) != 0)
        {
            snprintf(msg, sizeof(msg), "RPM zone -> %s", global_state.rpm_zone);
            log_event(msg);
            strcpy(prev_rpm_zone, global_state.rpm_zone);
        }

        // Fuel dropping below a threshold
        if (!fuel_warned && global_state.fuel_level < FUEL_LOW)
        {
            log_event("WARNING: Fuel critical (<0.7 gal)");
            fuel_warned = 1;
        }
        if (fuel_warned && global_state.fuel_level >= FUEL_LOW)
        {
            fuel_warned = 0;
        }

        // Battery level reaching a critical level
        if (!battery_warned && global_state.battery_level < 15.0f)
        {
            log_event("WARNING: Battery critical (<15%)");
            battery_warned = 1;
        }
        if (battery_warned && global_state.battery_level >= 15.0f)
        {
            battery_warned = 0;
        }

        // Engine temperature exceeding a defined limit
        if (strcmp(global_state.temp_zone, prev_temp_zone) != 0)
        {
            snprintf(msg, sizeof(msg), "Temp zone -> %s", global_state.temp_zone);
            log_event(msg);
            strcpy(prev_temp_zone, global_state.temp_zone);
        }

        // Hybrid assist activation or deactivation
        if (strcmp(global_state.hybrid_mode, prev_hybrid_mode) != 0)
        {
            snprintf(msg, sizeof(msg), "Hybrid mode -> %s", global_state.hybrid_mode);
            log_event(msg);
            strcpy(prev_hybrid_mode, global_state.hybrid_mode);
        }

        // Overheat Warning
        if (!overheat_warned && strcmp(global_state.temp_zone, "OVERHEAT") == 0)
        {
            log_event("CRITICAL: Engine overheating!");
            overheat_warned = 1;
        }
        if (overheat_warned && strcmp(global_state.temp_zone, "OVERHEAT") != 0)
        {
            overheat_warned = 0;
        }

        usleep(100000);
    }
    return NULL;
}