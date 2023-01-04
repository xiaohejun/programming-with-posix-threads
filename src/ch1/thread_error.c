#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    pthread_t thread;
    // 使用未创建的thread变量
    int status = pthread_join(thread, NULL);
    if (status != 0) {
        fprintf(stderr, "error %d: %s\n", status, strerror(status));
    }
    return 0;
}