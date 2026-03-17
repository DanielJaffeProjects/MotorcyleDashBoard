#ifndef STATE_H
#define STATE_H

#define MAX_EVENTS 50
#define FUEL_MAX   4.7f
#define FUEL_LOW   0.7f
#define RPM_MAX    16500
#define SPEED_MAX  200

typedef struct {
    char description[128];
    int  elapsed_seconds;
} LogEntry;

typedef struct {

    // Engine
    int   rpm;
    float engine_temp;      // Celsius
    int   engine_on;        // 1 = ON, 0 = OFF

    // Motion
    float speed;            // MPH
    float total_distance;   // includes random starting offset
    float trip_distance;    // always starts at 0.0

    // Fuel
    float fuel_level;       // 0.0 - 4.7 gallons

    // ECU derived
    char  rpm_zone[16];     // "IDLE" "NORMAL" "HIGH" "REDLINE"
    char  temp_zone[16];    // "COLD" "NORMAL" "HOT" "OVERHEAT"
    char  fuel_status[16];  // "OK" or "LOW FUEL"

    // Hybrid Assist
    float battery_level;    // 0.0 - 100.0%
    int   assist_active;    // 1 = ON, 0 = OFF
    int   charging_active;  // 1 = ON, 0 = OFF
    char  hybrid_mode[32];  // "ELECTRIC" "ASSIST" "CHARGING" "IDLE"

    // Signals & Lights
    int   left_signal;
    int   right_signal;
    int   hazard;
    int   headlight;

    // Timers
    int   total_elapsed_sec;    // random offset to simulate random history + running count
    int   current_elapsed_sec;  // counts from 0

    // Event Log
    LogEntry event_log[MAX_EVENTS];
    int      event_count;
} SystemState;

extern SystemState global_state;

#endif
