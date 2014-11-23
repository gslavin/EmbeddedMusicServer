#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<syslog.h>
#include<string.h>
#include "sound_server.h"

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

void post_state_to_log(state_t * state)
{
    openlog("MUSIC DAEMON: ", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "ID: %d Control: %s, Chord: %s"
        ,state->id, state->control, state->chord);
    closelog();
}

void post_to_log(char * msg)
{
    openlog("MUSIC DAEMON: ", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "%s", msg);
    closelog();
}

#if 0
void parse_message(char * buffer, cmd_t * cmd)
{
     
}
#endif

int read_fifo(state_t * state)
{
    FILE * fifo = fopen(FIFO_PATH, "rb");
    int rv = 0;
    int items_read = 0;
    if (fifo != NULL) {
        items_read = 
            fread(state, sizeof(state_t), 1, fifo);
        rv = (items_read == 0) ? 1 : 0;
    }
    else {
        rv = 1;
    }
    fclose(fifo);

    return rv;
}

int main(void)
{
    
    /* Process and session ID */
    pid_t pid, sid;
    state_t * state = NULL;
    if ((state = malloc(sizeof(state_t))) == NULL) {
        exit(EXIT_FAILURE);
    }

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
        if (read_fifo(state)) {
            post_to_log("error reading fifo");
        }
        else {
            post_state_to_log(state);
        }
        //parse_message();
        sleep(SLEEP_TIME);
    }

    return 0;
}
