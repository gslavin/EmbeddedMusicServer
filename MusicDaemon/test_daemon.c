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
#define DEBUG 1
#define MAX_THREADS 2
#define SLEEP_TIME 0.5

typedef enum {
    START,
    STOP,
    STOP_ALL
} control_t;

typedef enum {
    A_MAJ,
    B_MAJ,
    C_MAJ,
    D_MAJ,
    E_MAJ,
    F_MAJ,
    G_MAJ
} chord_t;

typedef struct thread_list {
    long thread_ids[MAX_THREADS];
    int count;
} thread_list;

typedef struct cmd_t {
    control_t control;
    chord_t chord;
} cmd_t;

void post_to_log(char * msg)
{
    openlog("MUSIC DAEMON: ", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Just checking in: %s", msg);
    closelog();
}

void parse_message(char * buffer, cmd_t * cmd)
{
         
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
                #if DEBUG
                post_to_log(buffer);
                #endif
                //parse_message();
            }
        }
        sleep(SLEEP_TIME);
    }
}
