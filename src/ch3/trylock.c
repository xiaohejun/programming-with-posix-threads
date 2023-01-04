#include <pthread.h>
#include "errors.h"

#define SPIN 1000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long counter;
time_t end_time;

void* counter_thread(void *arg)
{
    while (time(NULL) < end_time) {
        int status = pthread_mutex_lock(&mutex);
        err_abort_if(status != 0, status, "Lock mutex");

        for (int spin = 0; spin < SPIN; ++spin) {
            ++counter;
        }

        status = pthread_mutex_unlock(&mutex);
        err_abort_if(status != 0, status, "Unlock mutex");

        sleep(1);
    }

    printf("Counter is %#lx\n", counter);
    return NULL;
}


void* monitor_thread(void *arg) 
{
    int misses = 0;
    while (time(NULL) < end_time) {
        int status = pthread_mutex_trylock(&mutex);
        if (status != EBUSY) {
            err_abort_if(status != 0, status, "Trylock mutex");

            printf("Counter is %ld\n", counter / SPIN);
            status = pthread_mutex_unlock(&mutex);
            err_abort_if(status != 0, status, "Unlick mutex");
        } else {
            misses++;
        }
        sleep(3);
    }

    printf("Monito thread missed update %d times.\n", misses);
    return NULL;
}

int main()
{
    pthread_t counter_thread_id;
    pthread_t monitor_thread_id;

    // 1分钟之后停止
    end_time = time(NULL) + 60;
    int status = pthread_create(&counter_thread_id, NULL, counter_thread, NULL);
    err_abort_if(status != 0, status, "Create counter thread");

    status = pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);
    err_abort_if(status != 0, status, "Create monitor thread");

    status = pthread_join(counter_thread_id, NULL);
    err_abort_if(status != 0, status, "Join counter thread");

    status = pthread_join(monitor_thread_id, NULL);
    err_abort_if(status != 0, status, "Join monitor thread");
    return 0;
}

