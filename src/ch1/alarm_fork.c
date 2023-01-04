#include <sys/types.h>
#include <sys/wait.h>
#include "errors.h"

int main(int argc, char *argv[])
{
    int status;
    char line[128];
    int seconds;
    pid_t pid;
    char message[64];

    while (1) {
		printf("Alarm> ");
		if (fgets(line, sizeof(line), stdin) == NULL)
			exit(0);

		if (strlen(line) == 0)
			continue;

		if (sscanf(line, "%d %63[^\n]", &seconds, message) < 2) {
			fprintf(stderr, "Bad command\n");
		} else {
            pid = fork();
            if (pid == (pid_t)-1) {
                errno_abort("Fork");
            } else if (pid == (pid_t)0) {
                /*
                In the child, wait and then print a message 
                */
                sleep(seconds);
                printf("(%d) %s\n", seconds, message);
            } else {
                /*
                In the parent, call waitpid() to collect children that
                have already terminated. 
                */
                do {
                    pid = waitpid((pid_t)-1, NULL, WNOHANG);
                    if (pid == (pid_t)-1) {
                        errno_abort("Wait for child");
                    }
                } while (pid != (pid_t)0);
            }
		}
    }
    return 0;
}