#include <pthread.h>
#include "errors.h"

typedef struct alarm_tag {
    int seconds;
    char message[64];
} alarm_t;

void* alarm_thread(void *arg)
{
    alarm_t* alarm = (alarm_t *)arg;
    int status = pthread_detach(pthread_self());
    if (status != 0) {
        err_abort(status, "Detach thread.");
    }
    sleep(alarm->seconds);
    printf("(%d) %s\n", alarm->seconds, alarm->message);
    free(alarm);
    return NULL;
}

int main(void) {
	char line[128];

	while(1) {
		printf("Alarm> ");
		if (fgets(line, sizeof(line), stdin) == NULL)
			exit(0);

		if (strlen(line) == 0)
			continue;

        alarm_t* alarm = (alarm_t *)malloc(sizeof(alarm_t));
        if (alarm == NULL) {
            errno_abort("Allocate alarm");
        }

		if (sscanf(line, "%d %63[^\n]", &alarm->seconds, alarm->message) < 2) {
			fprintf(stderr, "Bad command\n");
            free(alarm);
		} else {
            pthread_t thread;
            int status = pthread_create(&thread, NULL, alarm_thread, alarm); 
            if (status != 0) {
                err_abort(status, "Create alarm thread");
            }
		}
	}
}