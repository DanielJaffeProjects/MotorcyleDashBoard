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

void *engine_thread(void *arg);
void *motion_thread(void *arg);
void *fuel_thread(void *arg);
void *ecu_thread(void *arg);
void *hybrid_thread(void *arg);
void *event_logger_thread(void *arg);
void *dashboard_thread(void *arg);

void init_state()
{
    srand(time(NULL));

    // Engine
    global_state.rpm = 0;
    global_state.engine_temp = 20.0f;
    global_state.engine_on = 1;

    // Motion
    global_state.speed = 50.0f;
    global_state.total_distance = 1000.0f + (rand() % 50000);
    global_state.trip_distance = 0.0f;

    // Fuel
    global_state.fuel_level = FUEL_MAX;

    // ECU derived
    strcpy(global_state.rpm_zone, "IDLE");
    strcpy(global_state.temp_zone, "COLD");
    strcpy(global_state.fuel_status, "OK");

    // Hybrid
    global_state.battery_level = 80.0f;
    global_state.assist_active = 0;
    global_state.charging_active = 0;
    strcpy(global_state.hybrid_mode, "IDLE");

    // Signals & Lights
    global_state.left_signal = 0;
    global_state.right_signal = 0;
    global_state.hazard = 0;
    global_state.headlight = 1;

    // Timers
    global_state.total_elapsed_sec = rand() % 100000;
    global_state.current_elapsed_sec = 0;

    // Event log
    global_state.event_count = 0;
    memset(global_state.event_log, 0, sizeof(global_state.event_log));
}

int main(void)
{

    init_state();

    pthread_t t_engine;
    pthread_t t_motion;
    pthread_t t_fuel;
    pthread_t t_ecu;
    pthread_t t_hybrid;
    pthread_t t_event_logger;
    pthread_t t_dashboard;

    pthread_create(&t_engine, NULL, engine_thread, NULL);
    pthread_create(&t_motion, NULL, motion_thread, NULL);
    pthread_create(&t_fuel, NULL, fuel_thread, NULL);
    pthread_create(&t_ecu, NULL, ecu_thread, NULL);
    pthread_create(&t_hybrid, NULL, hybrid_thread, NULL);
    pthread_create(&t_event_logger, NULL, event_logger_thread, NULL);
    pthread_create(&t_dashboard, NULL, dashboard_thread, NULL);

    pthread_join(t_engine, NULL);
    pthread_join(t_motion, NULL);
    pthread_join(t_fuel, NULL);
    pthread_join(t_ecu, NULL);
    pthread_join(t_hybrid, NULL);
    pthread_join(t_event_logger, NULL);
    pthread_join(t_dashboard, NULL);

    printf("Hahaha");
    return 0;
}