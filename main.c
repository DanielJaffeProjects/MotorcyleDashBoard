#include <stdio.h>
#include "state.h"
#include <pthread.h>

SystemState global_state;

void *engine_thread(void *arg);
void *motion_thread(void *arg);
void *fuel_thread(void *arg);
void *ecu_thread(void *arg);
void *hybrid_thread(void *arg);
void *event_logger_thread(void *arg);
void *dashboard_thread(void *arg);

int main(void) {
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