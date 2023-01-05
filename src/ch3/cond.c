#include <pthread.h>
#include <time.h>
#include "errors.h"

typedef struct my_struct_tag {
    pthread_mutex_t mutex; // 互斥量，保护共享数据
    pthread_cond_t cond; // 改变value的信号
    int value; // 被互斥量保护的数据
} my_struct_t;

my_struct_t data = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER,
    0
};

int hibernation = 1;

void *wait_thread(void *arg)
{
    sleep(hibernation);
    int status = pthread_mutex_lock(&data.mutex);
    err_abort_if(status != 0, status, "Lock mutex");

    // 设置谓词，条件取决于该值
    data.value = 1;

    status = pthread_cond_signal(&data.cond);
    err_abort_if(status != 0, status, "Signal condition");

    status = pthread_mutex_unlock(&data.mutex);
    err_abort_if(status != 0, status, "Unlock mutex");

    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        hibernation = atoi(argv[1]);
    }

    printf("arg: hibernation: %d\n", hibernation);

    pthread_t wait_thread_id;
    int status = pthread_create(&wait_thread_id, NULL, wait_thread, NULL);
    err_abort_if(status != 0, status, "Create wait thread");

    // 等待2s之后超时
    struct timespec timeout;
    timeout.tv_sec = time(NULL) + 2;
    timeout.tv_nsec = 0;

    status = pthread_mutex_lock(&data.mutex);
    err_abort_if(status != 0, status, "Lock mutex");

    while (data.value == 0) {
        status = pthread_cond_timedwait(&data.cond, &data.mutex, &timeout);
        if (status == ETIMEDOUT) {
            printf("Condition wait timed out.\n");
            break;
        } else {
            err_abort_if(status != 0, status, "Wait on condition");
        }
    }
    
    if (data.value != 0) {
        printf("Condition was signaled, data.value = %d\n", data.value);
    }
    status = pthread_mutex_unlock(&data.mutex);
    err_abort_if(status != 0, status, "unlock mutex");
    return 0;
}


