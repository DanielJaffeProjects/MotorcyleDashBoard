#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "state.h"

// cross platform support by Claude
#ifdef _WIN32
#include <windows.h>
#include <conio.h> // _kbhit(), _getch()
#else
#include <termios.h>
#include <fcntl.h>
static struct termios original_termios;
static int original_flags;
#endif

#define ACCEL_RATE 0.3f   // higher = snappier acceleration
#define DECEL_RATE 15.0f  // lower than accel feel
#define DT 1.0f           // delta-time in seconds
#define REFUEL_SECONDS 10 // how long refueling takes

void enable_raw_mode()
{
#ifdef _WIN32
    // On Windows, _getch() already reads raw keypresses with no echo
    // Nothing extra needed
#else
    struct termios raw;
    tcgetattr(STDIN_FILENO, &original_termios);
    raw = original_termios;

    // No echo, no line buffering
    raw.c_lflag &= ~(ECHO | ICANON);

    // Non-blocking: return immediately even if no key pressed
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    original_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, original_flags | O_NONBLOCK);
#endif
}

void restore_terminal()
{
#ifdef _WIN32
#else
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    fcntl(STDIN_FILENO, F_SETFL, original_flags);
#endif
}

// Returns the character pressed, or 0 if no key was pressed
static char read_key()
{
#ifdef _WIN32
    if (_kbhit())
        return (char)_getch(); // returns immediately, no echo
    return 0;
#else
    char c;
    int n = read(STDIN_FILENO, &c, 1);
    if (n == 1)
        return c;
    return 0;
#endif
}

// W - accelerate
// speed += (SPEED_MAX - speed) * accel_rate * dt
static void handle_W()
{
    pthread_mutex_lock(&motion_lock);
    if (global_state.engine_on == 1)
    {
        float delta = (SPEED_MAX - global_state.speed) * ACCEL_RATE * DT;
        global_state.speed += delta;
        if (global_state.speed > SPEED_MAX)
            global_state.speed = (float)SPEED_MAX;
        global_state.accel_mode = 'A';
    }
    pthread_mutex_unlock(&motion_lock);
}

// S - decelerate
// speed -= decel_rate * dt
static void handle_S()
{
    pthread_mutex_lock(&motion_lock);
    if (global_state.engine_on == 1)
    {
        float delta = DECEL_RATE * DT;
        global_state.speed -= delta;
        if (global_state.speed < 0.0f)
            global_state.speed = 0.0f;
        global_state.accel_mode = 'D';
    }
    pthread_mutex_unlock(&motion_lock);
}

// C - cruise
static void handle_C()
{
    pthread_mutex_lock(&motion_lock);
    global_state.accel_mode = 'C';
    pthread_mutex_unlock(&motion_lock);
}

// A - toggle left turn signal
static void handle_A()
{
    pthread_mutex_lock(&signal_lock);
    global_state.left_signal = !global_state.left_signal;
    pthread_mutex_unlock(&signal_lock);
}

// D - toggle right turn signal
static void handle_D()
{
    pthread_mutex_lock(&signal_lock);
    global_state.right_signal = !global_state.right_signal;
    pthread_mutex_unlock(&signal_lock);
}

// Z - toggle hazard lights
static void handle_Z()
{
    pthread_mutex_lock(&signal_lock);
    global_state.hazard = !global_state.hazard;
    pthread_mutex_unlock(&signal_lock);
}

// H - toggle headlight
static void handle_H()
{
    pthread_mutex_lock(&signal_lock);
    global_state.headlight = !global_state.headlight;
    pthread_mutex_unlock(&signal_lock);
}

// F - refuel
// Only works when: engine OFF, speed 0, not already refueling
static void handle_F()
{
    pthread_mutex_lock(&engine_lock);
    int can_refuel = (global_state.engine_on == 0 &&
                      global_state.speed == 0.0f &&
                      global_state.refueling == 0);
    if (can_refuel)
        global_state.refueling = 1;
    pthread_mutex_unlock(&engine_lock);

    if (!can_refuel)
        return;

    enqueue_event("Refueling started");

    float fill_per_sec = FUEL_MAX / (float)REFUEL_SECONDS;

    for (int i = 0; i < REFUEL_SECONDS; i++)
    {
        sleep(1);
        pthread_mutex_lock(&fuel_lock);
        global_state.fuel_level += fill_per_sec;
        if (global_state.fuel_level > FUEL_MAX)
            global_state.fuel_level = FUEL_MAX;
        pthread_mutex_unlock(&fuel_lock);
    }

    pthread_mutex_lock(&engine_lock);
    global_state.refueling = 0;
    pthread_mutex_unlock(&engine_lock);

    enqueue_event("Refueling complete");
}

// K - kill switch (engine ON -> OFF)
static void handle_K()
{
    int did_kill = 0;

    pthread_mutex_lock(&engine_lock);
    pthread_mutex_lock(&motion_lock);

    if (global_state.engine_on == 1)
    {
        global_state.engine_on = 0;
        global_state.rpm = 0;
        global_state.speed = 0.0f;
        global_state.accel_mode = 'C';
        global_state.current_elapsed_sec = 0;
        global_state.trip_distance = 0.0f;

        did_kill = 1;
    }

    pthread_mutex_unlock(&motion_lock);
    pthread_mutex_unlock(&engine_lock);

    if (did_kill)
    {
        enqueue_event("Kill switch - Engine OFF");
        pthread_cond_broadcast(&engine_on_cond);
    }
}

// I - ignition (engine OFF -> ON)
static void handle_I()
{
    int did_start = 0;
    int no_fuel = 0;
    int refueling_now = 0;

    pthread_mutex_lock(&engine_lock);
    pthread_mutex_lock(&fuel_lock);

    if (global_state.engine_on == 0)
    {
        if (global_state.refueling == 1)
        {
            refueling_now = 1;
        }
        else if (global_state.fuel_level <= 0.0f)
        {
            no_fuel = 1;
        }
        else
        {
            global_state.engine_on = 1;
            global_state.accel_mode = 'C';
            did_start = 1;
        }
    }

    pthread_mutex_unlock(&fuel_lock);
    pthread_mutex_unlock(&engine_lock);

    if (did_start)
    {
        enqueue_event("Ignition - Engine ON");
        pthread_cond_broadcast(&engine_on_cond);
    }
    else if (no_fuel)
    {
        enqueue_event("Ignition failed - no fuel");
    }
    else if (refueling_now)
    {
        enqueue_event("Ignition failed - refueling");
    }
    else
    {
        enqueue_event("Ignition ignored - already ON");
    }
}

// B - battery mode toggle
// ON: battery drains, fuel near zero, RPM drops to 0
// OFF (or auto-off when battery hits 0)
static void handle_B()
{
    pthread_mutex_lock(&hybrid_lock);
    global_state.battery_mode = !global_state.battery_mode;

    if (global_state.battery_mode)
    {
        enqueue_event("Battery mode ON - running on electric");
        enqueue_event("DR.K do you see this top secret message");
    }

    else
    {
        enqueue_event("Battery mode OFF");
    }
    pthread_mutex_unlock(&hybrid_lock);
}

// Q - graceful shutdown
static void handle_Q()
{
    enqueue_event("Shutdown requested");

    pthread_mutex_lock(&engine_lock);
    global_state.shutdown_flag = 1;
    pthread_mutex_unlock(&engine_lock);

    // Wake up all waiting threads so they can check shutdown_flag and exit
    pthread_cond_broadcast(&engine_on_cond);
    pthread_cond_broadcast(&speed_change_cond);
    pthread_cond_broadcast(&ecu_cond);
    pthread_cond_broadcast(&queue_not_empty);
    pthread_cond_broadcast(&queue_not_full);

    restore_terminal();
}

void *input_thread(void *arg)
{
    (void)arg;

    enable_raw_mode();

    while (global_state.shutdown_flag == 0)
    {
        char c = read_key();

        if (c == 0)
        {
            // No key pressed — sleep briefly
            usleep(50000); // 50 ms
            continue;
        }

        switch (c)
        {
        case 'w':
        case 'W':
            handle_W();
            break;
        case 's':
        case 'S':
            handle_S();
            break;
        case 'c':
        case 'C':
            handle_C();
            break;
        case 'a':
        case 'A':
            handle_A();
            break;
        case 'd':
        case 'D':
            handle_D();
            break;
        case 'z':
        case 'Z':
            handle_Z();
            break;
        case 'h':
        case 'H':
            handle_H();
            break;
        case 'f':
        case 'F':
            handle_F();
            break;
        case 'k':
        case 'K':
            handle_K();
            break;
        case 'i':
        case 'I':
            handle_I();
            break;
        case 'b':
        case 'B':
            handle_B();
            break;
        case 'q':
        case 'Q':
            handle_Q();
            break;
        default:
            break;
        }
    }

    restore_terminal();
    return NULL;
}