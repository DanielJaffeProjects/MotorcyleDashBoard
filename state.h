#ifndef STATE_H
#define STATE_H

#include <pthread.h>

#define MAX_EVENTS 50
#define FUEL_MAX   4.7f
#define FUEL_LOW   0.7f
#define RPM_MAX    16500
#define SPEED_MAX  200

typedef struct {
    char description[128];
    int  elapsed_seconds;
} LogEntry;

// Main System State
typedef struct {

    // Engine 
    int   rpm;
    float engine_temp;      // Celsius
    int   engine_on;        // 1 = ON, 0 = OFF

    // Motion
    float speed;            // MPH
    float total_distance;
    float trip_distance;

    // Fuel
    float fuel_level;       // gallons

    // ECU Derived
    char  rpm_zone[16];     // "IDLE" "NORMAL" "HIGH" "REDLINE"
    char  temp_zone[16];    // "COLD" "NORMAL" "HOT" "OVERHEAT"
    char  fuel_status[16];  // "OK" or "LOW FUEL"

    // Hybrid
    float battery_level;    // 0–100%
    int   assist_active;
    int   charging_active;
    char  hybrid_mode[32];  // "ELECTRIC" "ASSIST" "CHARGING" "IDLE"

    // Signals
    int   left_signal;
    int   right_signal;
    int   hazard;
    int   headlight;

    // Timers
    int total_elapsed_sec;
    int current_elapsed_sec;

} SystemState;

extern SystemState global_state;

// Mutex Locks
extern pthread_mutex_t engine_lock;
extern pthread_mutex_t motion_lock;
extern pthread_mutex_t fuel_lock;
extern pthread_mutex_t hybrid_lock;
extern pthread_mutex_t ecu_lock;
extern pthread_mutex_t signal_lock;
extern pthread_mutex_t log_lock;

// Condition Variables for Coordination
extern pthread_cond_t engine_on_cond;     // Motion/Fuel wait for engine ON
extern pthread_cond_t speed_change_cond;  // Hybrid waits for speed changes
extern pthread_cond_t fuel_cond;          // Fuel coordination
extern pthread_cond_t hybrid_cond;        // Hybrid coordination
extern pthread_cond_t ecu_cond;           // ECU reacts to system changes

// ECU System State
typedef enum {
    ENGINE_OFF,
    IDLE,
    NORMAL,
    HIGH_LOAD,
    CRITICAL
} ECUState;

extern ECUState ecu_state;

// Event Queue
typedef struct {
    LogEntry buffer[MAX_EVENTS];
    int head;
    int tail;
    int count;
} EventQueue;

extern EventQueue event_queue;

extern pthread_cond_t queue_not_empty;
extern pthread_cond_t queue_not_full;

#endif