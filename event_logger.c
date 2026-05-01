#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "state.h"


// Producer function
void enqueue_event(const char *desc)
{
    LogEntry entry;

    strncpy(entry.description, desc, sizeof(entry.description) - 1);
    entry.description[sizeof(entry.description) - 1] = '\0';

    entry.elapsed_seconds = global_state.current_elapsed_sec;

    pthread_mutex_lock(&queue_lock);

    while (event_queue.count == MAX_EVENTS) {
        pthread_cond_wait(&queue_not_full, &queue_lock);
    }

    event_queue.buffer[event_queue.tail] = entry;
    event_queue.tail = (event_queue.tail + 1) % MAX_EVENTS;
    event_queue.count++;

    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_lock);
}

// Consumer thread
void *event_logger_thread(void *arg)
{
    (void)arg;

    while (global_state.shutdown_flag == 0) {
        LogEntry entry;

        pthread_mutex_lock(&queue_lock);

        while (event_queue.count == 0 && global_state.shutdown_flag == 0) {
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }

        if (event_queue.count == 0 && global_state.shutdown_flag == 1) {
            pthread_mutex_unlock(&queue_lock);
            break;
        }

        entry = event_queue.buffer[event_queue.head];
        event_queue.head = (event_queue.head + 1) % MAX_EVENTS;
        event_queue.count--;

        pthread_cond_signal(&queue_not_full);
        pthread_mutex_unlock(&queue_lock);

        // Store in dashboard buffer
        pthread_mutex_lock(&log_lock);

        int index = global_state.event_count % MAX_EVENTS;
        global_state.event_log[index] = entry;
        global_state.event_count++;

        pthread_mutex_unlock(&log_lock);
    }

    return NULL;
}