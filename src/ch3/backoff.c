#include <pthread.h>
#include "errors.h"

#define ITERATIONS 10

pthread_mutex_t mutex[3] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
};

int backoff = 1; // backoff or deadlock
int yield_flag = 0; // 0: no yield > 0: yield, < 0: sleep

void *lock_forward(void *arg)
{
    int backoffs;
    int status;
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        backoffs = 0;
        for (int i = 0; i < 3; ++i) {
            if (i == 0) {
                status = pthread_mutex_lock(&mutex[i]);
                err_abort_if(status != 0, status, "Frist lock");
                printf("forward locker got %d\n", i);
            } else {
                if (backoff) {
                    // 要回退的话，选择尝试锁住，无法获得锁的权利就干其他的时期
                    status = pthread_mutex_trylock(&mutex[i]);
                } else {
                    // 不回退，直接要锁，要不了的话就阻塞，自己也不释放自己已经锁了的mutex
                    // 造成死backward locker got锁
                    status = pthread_mutex_lock(&mutex[i]);
                }
                if (status == EBUSY) {
                    backoffs++;
                    printf("forward locker backing off at %d]\n", i);
                    // 尝试锁失败，把自己已经获得的锁给打开
                    for (; i >= 0; --i) {
                        status = pthread_mutex_unlock(&mutex[i]);
                        err_abort_if(status != 0, status, "Backoff");
                    }
                } else {
                    err_abort_if(status != 0, status, "Lock mutex");
                    printf("forward locker got %d\n", i);
                }
            }

            if (yield_flag) {
                if (yield_flag > 0) {
                    sched_yield();
                } else {
                    sleep(1);
                }
            }
        }
        // 报告回退了几次
        printf("lock forward got all locks, %d backoffs\n", backoffs);
        pthread_mutex_unlock(&mutex[0]);
        pthread_mutex_unlock(&mutex[1]);
        pthread_mutex_unlock(&mutex[2]);
        sched_yield();
    }

    return NULL;
}

void *lock_backward(void *arg)
{
    int backoffs;
    int status;
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        backoffs = 0;
        for (int i = 2; i >= 0; --i) {
            if (i == 2) {
                status = pthread_mutex_lock(&mutex[i]);
                err_abort_if(status != 0, status, "Frist lock");
                printf("backward locker got %d\n", i);
            } else {
                if (backoff) {
                    // 要回退的话，选择尝试锁住，无法获得锁的权利就干其他的时期
                    status = pthread_mutex_trylock(&mutex[i]);
                } else {
                    // 不回退，直接要锁，要不了的话就阻塞，自己也不释放自己已经锁了的mutex
                    // 造成死锁
                    status = pthread_mutex_lock(&mutex[i]);
                }
                if (status == EBUSY) {
                    backoffs++;
                    printf("forward locker backing off at %d]\n", i);
                    // 尝试锁失败，把自己已经获得的锁给打开
                    for (; i < 3; ++i) {
                        status = pthread_mutex_unlock(&mutex[i]);
                        err_abort_if(status != 0, status, "Backoff");
                    }
                } else {
                    err_abort_if(status != 0, status, "Lock mutex");
                    printf("backward locker got %d\n", i);
                }
            }

            if (yield_flag) {
                if (yield_flag > 0) {
                    sched_yield();
                } else {
                    sleep(1);
                }
            }
        }
        // 报告回退了几次
        printf("lock backward got all locks, %d backoffs\n", backoffs);
        pthread_mutex_unlock(&mutex[0]);
        pthread_mutex_unlock(&mutex[1]);
        pthread_mutex_unlock(&mutex[2]);
        sched_yield();
    }

    return NULL;
}


int main(int argc, char *argv[])
{
    pthread_t forward, backward;
    if (argc > 2) {
        yield_flag = atoi(argv[2]);
    }

    int status = pthread_create(&forward, NULL, lock_forward, NULL);
    err_abort_if(status != 0, status, "Create forward");

    status = pthread_create(&backward, NULL, lock_backward, NULL);
    err_abort_if(status != 0, status, "Create backward");

    pthread_exit(NULL);
    return 0;
}