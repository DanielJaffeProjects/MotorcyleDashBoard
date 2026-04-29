/////////////////////////////////////////////////
// Group Members: Khalil Smith, Daniel Jaffe, Hien Tran
// Date: 03/24/2026
// Course: CSC 220 Operating Systems and Systems Programming
// Project: Motorcycle System Simulation - Phase I
// Description: This project simulates a motorcycle system using
// multiple threads, where each subsystem (engine, motion, fuel, ECU, and hybrid assist)
// runs independently but shares a common system state. The simulation models real-time
// behavior like RPM changes, speed, temperature, fuel usage, and battery activity, while
// the ECU and hybrid threads interpret and manage the system to produce meaningful outputs
// displayed on a dashboard.
/////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

SystemState global_state;
EventQueue event_queue;
ECUState ecu_state;

pthread_mutex_t engine_lock;
pthread_mutex_t motion_lock;
pthread_mutex_t fuel_lock;
pthread_mutex_t hybrid_lock;
pthread_mutex_t ecu_lock;
pthread_mutex_t signal_lock;
pthread_mutex_t log_lock;
pthread_mutex_t queue_lock;

pthread_cond_t ecu_cond;
pthread_cond_t speed_change_cond;
pthread_cond_t fuel_cond;
pthread_cond_t hybrid_cond;
pthread_cond_t engine_on_cond;
pthread_cond_t queue_not_empty;
pthread_cond_t queue_not_full;

void *engine_thread(void *arg);
void *motion_thread(void *arg);
void *fuel_thread(void *arg);
void *ecu_thread(void *arg);
void *hybrid_thread(void *arg);
void *event_logger_thread(void *arg);
void *dashboard_thread(void *arg);
void *input_thread(void *arg);

// Initialize mutexes & condition variables
void init_sync()
{
    pthread_mutex_init(&engine_lock, NULL);
    pthread_mutex_init(&motion_lock, NULL);
    pthread_mutex_init(&fuel_lock, NULL);
    pthread_mutex_init(&hybrid_lock, NULL);
    pthread_mutex_init(&ecu_lock, NULL);
    pthread_mutex_init(&signal_lock, NULL);
    pthread_mutex_init(&log_lock, NULL);
    pthread_mutex_init(&queue_lock, NULL);

    pthread_cond_init(&engine_on_cond, NULL);
    pthread_cond_init(&speed_change_cond, NULL);
    pthread_cond_init(&fuel_cond, NULL);
    pthread_cond_init(&hybrid_cond, NULL);
    pthread_cond_init(&ecu_cond, NULL);

    pthread_cond_init(&queue_not_empty, NULL);
    pthread_cond_init(&queue_not_full, NULL);
}

void destroy_sync()
{
    pthread_mutex_destroy(&engine_lock);
    pthread_mutex_destroy(&motion_lock);
    pthread_mutex_destroy(&fuel_lock);
    pthread_mutex_destroy(&hybrid_lock);
    pthread_mutex_destroy(&ecu_lock);
    pthread_mutex_destroy(&signal_lock);
    pthread_mutex_destroy(&log_lock);
    pthread_mutex_destroy(&queue_lock);

    pthread_cond_destroy(&engine_on_cond);
    pthread_cond_destroy(&speed_change_cond);
    pthread_cond_destroy(&fuel_cond);
    pthread_cond_destroy(&hybrid_cond);
    pthread_cond_destroy(&ecu_cond);
    pthread_cond_destroy(&queue_not_empty);
    pthread_cond_destroy(&queue_not_full);
}

// Initialize event queue
void init_queue()
{
    event_queue.head = 0;
    event_queue.tail = 0;
    event_queue.count = 0;
}

void init_state(int argc, char *argv[])
{
    // ./a.out <RPM> <ENGINE_STATE> <SPEED> <FUEL_LEVEL> <A/D> <BATTERY_LEVEL>

    global_state.rpm = atoi(argv[1]);
    global_state.engine_on = atoi(argv[2]);
    global_state.speed = atof(argv[3]);
    global_state.fuel_level = atof(argv[4]);

    // Store accel mode in global state so ECU/dashboard can access it
    global_state.accel_mode = argv[5][0];

    // Hybrid battery
    global_state.battery_level = (argc > 6) ? atof(argv[6]) : 80.0f;

    // Defaults for remaining values
    global_state.engine_temp = 20.0f;
    global_state.total_distance = 0.0f;
    global_state.trip_distance = 0.0f;

    strcpy(global_state.rpm_zone, "IDLE");
    strcpy(global_state.temp_zone, "COLD");
    strcpy(global_state.fuel_status, "OK");

    global_state.assist_active = 0;
    global_state.charging_active = 0;
    strcpy(global_state.hybrid_mode, "IDLE");

    global_state.left_signal = 0;
    global_state.right_signal = 0;
    global_state.hazard = 0;
    global_state.headlight = 1;

    global_state.total_elapsed_sec = 0;
    global_state.current_elapsed_sec = 0;

    global_state.event_count = 0;

    global_state.refueling = 0;
    global_state.battery_mode = 0;
    global_state.shutdown_flag = 0;
    memset(global_state.event_log, 0, sizeof(global_state.event_log));
}

int main(int argc, char *argv[])
{
    if (argc < 6)
    {
        printf("Usage: ./main.exe <RPM> <ENGINE_STATE> <SPEED> <FUEL_LEVEL> <A/D> [BATTERY]\n");
        return 1;
    }
    init_sync();
    init_queue();
    init_state(argc, argv);

    pthread_t t_engine;
    pthread_t t_motion;
    pthread_t t_fuel;
    pthread_t t_ecu;
    pthread_t t_hybrid;
    pthread_t t_event_logger;
    pthread_t t_dashboard;
    pthread_t t_input;

    pthread_create(&t_engine, NULL, engine_thread, NULL);
    pthread_create(&t_motion, NULL, motion_thread, argv[5]);
    pthread_create(&t_fuel, NULL, fuel_thread, NULL);
    pthread_create(&t_ecu, NULL, ecu_thread, NULL);
    pthread_create(&t_hybrid, NULL, hybrid_thread, NULL);
    pthread_create(&t_event_logger, NULL, event_logger_thread, NULL);
    pthread_create(&t_dashboard, NULL, dashboard_thread, NULL);
    pthread_create(&t_input, NULL, input_thread, NULL);

    pthread_join(t_engine, NULL);
    pthread_join(t_motion, NULL);
    pthread_join(t_fuel, NULL);
    pthread_join(t_ecu, NULL);
    pthread_join(t_hybrid, NULL);
    pthread_join(t_event_logger, NULL);
    pthread_join(t_dashboard, NULL);
    pthread_join(t_input, NULL);

    destroy_sync();
    printf("\nSystem shutdown complete.\n");
    return 0;
}