#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#endif

#include "state.h"

#define MIN_WIDTH 60
#define MAX_WIDTH 140

// Getting the width of the terminal. This part is created with the help of AI
int get_terminal_width()
{
#ifdef _WIN32
       CONSOLE_SCREEN_BUFFER_INFO csbi;
       int columns = 80;

       if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
       {
              columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
       }
       return columns;
#else
       struct winsize w;

       if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0)
              return 80;

       return w.ws_col;
#endif
}

void format_time(int total_seconds, char *buf)
{
       int h = total_seconds / 3600;
       int m = (total_seconds % 3600) / 60;
       int s = total_seconds % 60;
       sprintf(buf, "%02d:%02d:%02d", h, m, s);
}

void format_bar(float value, float max, int width, char *buf)
{
       int filled = (int)((value / max) * width);

       if (filled > width)
              filled = width;
       if (filled < 0)
              filled = 0;

       buf[0] = '[';

       for (int i = 0; i < width; i++)
              buf[i + 1] = (i < filled) ? '=' : '-';

       buf[width + 1] = ']';
       buf[width + 2] = '\0';
}

void print_top(int width)
{
       printf("+");
       for (int i = 0; i < width - 2; i++)
              printf("-");
       printf("+\n");
}

void print_mid(int width)
{
       printf("+");
       for (int i = 0; i < width - 2; i++)
              printf("-");
       printf("+\n");
}

void print_bottom(int width)
{
       printf("+");
       for (int i = 0; i < width - 2; i++)
              printf("-");
       printf("+\n");
}

void print_row(int width, const char *text)
{
       int content_width = width - 4;
       printf("| %-*.*s |\n", content_width, content_width, text);
}

// Printing the Actual Dashboard
void print_dashboard(SystemState snap)
{
       int width = get_terminal_width();

       if (width < MIN_WIDTH)
       {
              width = MIN_WIDTH;
       }
       if (width > MAX_WIDTH)
       {
              width = MAX_WIDTH;
       }

       int content_width = width - 4;

       printf("\033[H");

       char total_time[9], current_time[9];
       format_time(snap.total_elapsed_sec, total_time);
       format_time(snap.current_elapsed_sec, current_time);

       int bar_width = content_width / 3;

       char rpm_bar[256], speed_bar[256], fuel_bar[256], battery_bar[256];

       format_bar(snap.rpm, RPM_MAX, bar_width, rpm_bar);
       format_bar(snap.speed, SPEED_MAX, bar_width, speed_bar);
       format_bar(snap.fuel_level, FUEL_MAX, bar_width, fuel_bar);
       format_bar(snap.battery_level, 100.0f, bar_width, battery_bar);

       print_top(width);

       printf("\033[H");

       // Title
       char title[512];
       snprintf(title, sizeof(title), "MOTORCYCLE DASHBOARD OS");
       print_row(width, title);

       print_mid(width);

       // ENGINE
       char engine[512];
       snprintf(engine, sizeof(engine),
                "ENGINE: %s   RPM: %d %s",
                snap.engine_on ? "ON" : "OFF",
                snap.rpm,
                rpm_bar);
       print_row(width, engine);

       char temp[512];
       snprintf(temp, sizeof(temp),
                "TEMP: %.1f C   ZONE: %s",
                snap.engine_temp,
                snap.temp_zone);
       print_row(width, temp);

       print_mid(width);

       // MOTION
       char speed[512];
       snprintf(speed, sizeof(speed),
                "SPEED: %.1f MPH %s",
                snap.speed,
                speed_bar);
       print_row(width, speed);

       char distance[512];
       snprintf(distance, sizeof(distance),
                "TOTAL: %.1f mi   TRIP: %.1f mi",
                snap.total_distance,
                snap.trip_distance);
       print_row(width, distance);

       print_mid(width);

       // FUEL
       char fuel[512];
       snprintf(fuel, sizeof(fuel),
                "FUEL: %.2f gal %s   STATUS: %s",
                snap.fuel_level,
                fuel_bar,
                snap.fuel_status);
       print_row(width, fuel);

       print_mid(width);

       // BATTERY
       char battery[512];
       snprintf(battery, sizeof(battery),
                "BATTERY: %.1f%% %s   MODE: %s",
                snap.battery_level,
                battery_bar,
                snap.hybrid_mode);
       print_row(width, battery);

       char hybrid[512];
       snprintf(hybrid, sizeof(hybrid),
                "ASSIST: %s   CHARGING: %s",
                snap.assist_active ? "ON" : "OFF",
                snap.charging_active ? "ON" : "OFF");
       print_row(width, hybrid);

       print_mid(width);

       // SIGNALS
       char signals[512];
       snprintf(signals, sizeof(signals),
                "LEFT: %s   RIGHT: %s   HAZARD: %s   HEADLIGHT: %s",
                snap.left_signal ? "<<" : "--",
                snap.right_signal ? ">>" : "--",
                snap.hazard ? "ON" : "OFF",
                snap.headlight ? "ON" : "OFF");
       print_row(width, signals);

       print_mid(width);

       // TIME
       char timebuf[512];
       snprintf(timebuf, sizeof(timebuf),
                "TOTAL TIME: %s   CURRENT TIME: %s",
                total_time, current_time);
       print_row(width, timebuf);

       print_mid(width);

       // EVENTS
       print_row(width, "RECENT EVENTS:");

       int total = snap.event_count;
       int count = total < 4 ? total : 4;

       for (int i = count - 1; i >= 0; i--)
       {
              int index = (total - 1 - i) % MAX_EVENTS;

              char event[512];
              snprintf(event, sizeof(event),
                       "[%05d] %s",
                       snap.event_log[index].elapsed_seconds,
                       snap.event_log[index].description);

              print_row(width, event);
       }

       for (int i = count; i < 4; i++)
              print_row(width, "[-----]");

       print_bottom(width);
}

void *dashboard_thread(void *arg){
#ifdef _WIN32
       system("cls");
#else
       system("clear");
#endif
       while (1){
#ifdef _WIN32
              system("cls");
#else
              system("clear");
#endif
              // lock everything so that dashboard can get everything without things changing
              pthread_mutex_lock(&engine_lock);
              pthread_mutex_lock(&motion_lock);
              pthread_mutex_lock(&fuel_lock);
              pthread_mutex_lock(&hybrid_lock);
              pthread_mutex_lock(&ecu_lock);
              pthread_mutex_lock(&signal_lock);
              pthread_mutex_lock(&log_lock);

              // take a copy of what global state is at this point
              SystemState snap = global_state;
              // Increment the time counters
              global_state.total_elapsed_sec++;
              global_state.current_elapsed_sec++;

              //unlocks all the parts to continue
              pthread_mutex_unlock(&log_lock);
              pthread_mutex_unlock(&signal_lock);
              pthread_mutex_unlock(&ecu_lock);
              pthread_mutex_unlock(&hybrid_lock);
              pthread_mutex_unlock(&fuel_lock);
              pthread_mutex_unlock(&motion_lock);
              pthread_mutex_unlock(&engine_lock);

              // print the output of the dashboard
              print_dashboard(snap);
              fflush(stdout);
              sleep(1);
       }

       return NULL;
}