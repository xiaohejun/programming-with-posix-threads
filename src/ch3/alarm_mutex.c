#include <pthread.h>
#include <time.h>
#include "errors.h"

typedef struct alarm_tag {
    struct alarm_tag *link;
    int seconds;
    time_t time;
    char message[64];
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
alarm_t *alarm_list;

void* alarm_thread(void *arg)
{
    while (1) {
        // 要使用alarm_list了，先锁住
        int status = pthread_mutex_lock(&alarm_mutex);
        if (status != 0) {
            err_abort(status, "Lock mutext");
        }

        // 当前alarm为指向链表中的第一个元素
        alarm_t* alarm = alarm_list;
        
        int sleep_time = 0;
        if (alarm == NULL) {
            // alarm 是空，当前没有闹钟请求要处理
            // 阻塞自己1s
            sleep_time = 1;
        } else {
            // 有元素，main函数里面alarm请求是按照时间请求加入的
            // 链表头节点指向下一个，因为当前的已经准备要从链表中删除了
            alarm_list = alarm->link;

            // 取得当前的时间
            time_t now = time(NULL);
            if (alarm->time <= now) {
                sleep_time = 0;
            } else {
                sleep_time = alarm->time - now;
            }
            printf("\n\"%s\": timeout:%ld, timenow:%ld, sleep_time:%d\n", alarm->message, alarm->time, now, sleep_time);
        }
        // 解锁
        status = pthread_mutex_unlock(&alarm_mutex);
        if (status != 0) {
            err_abort(status, "Unlock mutex");
        }

        if (sleep_time > 0) {
            // 当前线程需要sleep
            sleep(sleep_time);
        } else {
            // 当前线程不需要sleep，交出自己的调度权
            sched_yield();
        }

        // 当前的闹钟已经处理完了，可以把内存释放了
        if (alarm != NULL) {
            printf("\n\"%s\":%ds, current timeout", alarm->message, alarm->seconds);
            free(alarm);
        }
    }

    return NULL;
}

void insert_alarm_to_sorted_list(alarm_t **alarm_list, alarm_t* alarm)
{
    alarm_t** last = alarm_list;
    alarm_t* cur = *alarm_list;
    while (cur != NULL) {
        if (cur->time >= alarm->time) {
            // alarm插入到当前遍历到的元素的前面
            alarm->link = cur;
            *last = alarm;
            break;
        }
        last = &cur->link;
        cur = cur->link;
    }

    if (cur == NULL) {
        *last = alarm;
        alarm->link = NULL;
    }
}

void print_list(const alarm_t* alarm_list)
{
    printf("[list:");
    for (alarm_t* cur = alarm_list; cur != NULL; cur = cur->link) {
        time_t now = time(NULL);
        printf("{time:%d now:%d diff:%d\"%s\"},", cur->time, now, cur->time - now, cur->message);
    }
    printf("]\n");
}

int main()
{
    pthread_t thread;
    int status = pthread_create(&thread, NULL, alarm_thread, NULL);
    if (status != 0) {
        err_abort(status, "Pthread create");
    }

    char line[128];
    while (1) {
        printf("alarm>");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            exit(0);
        }
        if (strlen(line) == 0) {
            continue;
        }
        alarm_t* alarm = (alarm_t *)malloc(sizeof(alarm_t));
        if (alarm == NULL) {
            errno_abort("Allocated alarm");
        }
        if (sscanf(line, "%d %63[^\n]", &alarm->seconds, alarm->message) < 2) {
            fprintf(stderr, "Bad command");
            free(alarm);
        } else {
            // 要操作alarm_list了，先锁住
            int status = pthread_mutex_lock(&alarm_mutex);
            if (status != 0) {
                err_abort(status, "Lock mutex");
            }
            alarm->time = time(NULL) + alarm->seconds;
            insert_alarm_to_sorted_list(&alarm_list, alarm);
            print_list(alarm_list);
            // 解锁
            status = pthread_mutex_unlock(&alarm_mutex);
            if (status != 0) {
                err_abort(status, "Unlock mutex");
            }
        }
    }
    return 0;
}