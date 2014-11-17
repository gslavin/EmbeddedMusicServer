#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<syslog.h>
#include<string.h>

#define FIFO_PATH "/home/thomas/EmbSysFinal/music_commands"

void post_to_log(char * msg)
{
    openlog("MUSIC DAEMON: ", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Just checking in: %s", msg);
    closelog();
}

int main(void)
{
    
    /* Process and session ID */
    pid_t pid, sid;
    FILE * fifo;
    char buffer[200];
    int rv = 0;

    /* Fork from parent process */
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }
    
    /* change file mode mask */
    umask(0);

    /* Open logs here */
    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    /* Change to directory that will always be present */
    if ((chdir("/")) < 0) {
        /* log failure here */
        exit(EXIT_FAILURE);
    }

    /* close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon initialization */
    while (1) {
        /* read fifo and post to log */
        fifo = fopen(FIFO_PATH, "r");
        if (fifo != NULL) {
            rv = fscanf(fifo, "%s", buffer);
            fclose(fifo);
            if (rv == 1) {
                post_to_log(buffer);
            }
        }
        sleep(3);
    }
}
