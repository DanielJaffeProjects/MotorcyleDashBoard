#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "state.h"

// Formating Time
void format_time(int total_seconds, char *buf)
{
    int h = total_seconds / 3600;
    int m = (total_seconds % 3600) / 60;
    int s = total_seconds % 60;
    sprintf(buf, "%02d:%02d:%02d", h, m, s);
}

// Formating Bars
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

// Printing the Actual Dashboard
void print_dashboard(void)
{
    char total_time[9], current_time[9];
    char fuel_bar[23], battery_bar[23], rpm_bar[23], speed_bar[23];

    format_time(global_state.total_elapsed_sec, total_time);
    format_time(global_state.current_elapsed_sec, current_time);
    format_bar((float)global_state.rpm, (float)RPM_MAX, 20, rpm_bar);
    format_bar(global_state.speed, (float)SPEED_MAX, 20, speed_bar);
    format_bar(global_state.fuel_level, FUEL_MAX, 20, fuel_bar);
    format_bar(global_state.battery_level, 100.0f, 20, battery_bar);

    printf("+----------------------------------------------+\n");
    printf("|         MOTORCYCLE  DASHBOARD  OS            |\n");
    printf("+----------------------------------------------+\n");

    // Engine
    printf("|  ENGINE  : %-3s                               |\n",
           global_state.engine_on ? "ON " : "OFF");
    printf("|  RPM     : %-6d %s          |\n",
           global_state.rpm, rpm_bar);
    printf("|  ZONE    : %-8s                           |\n",
           global_state.rpm_zone);
    printf("|  TEMP    : %5.1f C   STATE : %-8s         |\n",
           global_state.engine_temp, global_state.temp_zone);

    printf("+----------------------------------------------+\n");

    // Motion
    printf("|  SPEED   : %5.1f MPH %s     |\n",
           global_state.speed, speed_bar);
    printf("|  TOTAL   : %010.1f mi                    |\n",
           global_state.total_distance);
    printf("|  TRIP    : %010.1f mi                    |\n",
           global_state.trip_distance);

    printf("+----------------------------------------------+\n");

    // Fuel
    printf("|  FUEL    : %4.2f gal %s       |\n",
           global_state.fuel_level, fuel_bar);
    printf("|  STATUS  : %-10s                        |\n",
           global_state.fuel_status);

    printf("+----------------------------------------------+\n");

    // Hybrid
    printf("|  BATTERY : %5.1f%% %s        |\n",
           global_state.battery_level, battery_bar);
    printf("|  MODE    : %-10s                        |\n",
           global_state.hybrid_mode);
    printf("|  ASSIST  : %-3s      CHARGING : %-3s          |\n",
           global_state.assist_active ? "ON " : "OFF",
           global_state.charging_active ? "ON " : "OFF");

    printf("+----------------------------------------------+\n");

    // Signals & Lights
    printf("|  LEFT : %-2s   RIGHT : %-2s   HAZARD : %-3s     |\n",
           global_state.left_signal ? "<<" : "  ",
           global_state.right_signal ? ">>" : "  ",
           global_state.hazard ? "ON " : "OFF");
    printf("|  HEADLIGHT : %-3s                             |\n",
           global_state.headlight ? "ON " : "OFF");

    printf("+----------------------------------------------+\n");

    // Timers
    printf("|  TOTAL TIME   : %-8s                     |\n", total_time);
    printf("|  CURRENT TIME : %-8s                     |\n", current_time);

    printf("+----------------------------------------------+\n");

    // Last 4 events from circular buffer
    printf("|  RECENT EVENTS:                              |\n");
    int total = global_state.event_count;
    int count = total < 4 ? total : 4;
    for (int i = count - 1; i >= 0; i--)
    {
        int index = (total - 1 - i) % MAX_EVENTS;
        printf("|  [%05d] %-36s|\n",
               global_state.event_log[index].elapsed_seconds,
               global_state.event_log[index].description);
    }
    for (int i = count; i < 4; i++)
    {
        printf("|  [-----] %-36s|\n", "");
    }

    printf("+----------------------------------------------+\n");
}

void *dashboard_thread(void *arg)
{
    while (1)
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        print_dashboard();
        fflush(stdout);

        global_state.total_elapsed_sec++;
        global_state.current_elapsed_sec++;

        sleep(1);
    }
    return NULL;
}