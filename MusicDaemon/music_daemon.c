#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<syslog.h>
#include<string.h>
#include<pthread.h>
#include<math.h>
#include "sound_server.h"

#define DEBUG 0
#define MAX_THREADS 5
#define SLEEP_TIME 0.5

typedef enum {
    START,
    STOP,
    STOP_ALL
} control_t;

typedef enum {
    /* piano key number of middle A */
    A_MAJ = 49,
    B_FLAT_MAJ,
    B_MAJ,
    C_MAJ,
    C_SHARP_MAJ,
    D_MAJ,
    D_SHARP_MAJ,
    E_MAJ,
    F_MAJ,
    F_SHARP_MAJ,
    G_MAJ
} chord_t;

typedef struct cmd_t {
    control_t control;
    chord_t chord;
} cmd_t;

typedef struct user_info {
    int id;
    pthread_t thread;
    int valid_thread;
    cmd_t cmd;
} user_info;

typedef struct user_list {
    user_info user;
    struct user_list * next;
} user_list;

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

void parse_state(state_t * state, cmd_t * cmd)
{
    /* set control field */
    if (strcmp(state->control, "start") == 0) {
        cmd->control = START;
    }
    else if (strcmp(state->control, "stop") == 0) {
        cmd->control = STOP;
    }
    else if (strcmp(state->control, "stop_all") == 0) {
        cmd->control = STOP_ALL;
    }
    /* Set command field */
    if (strcmp(state->chord, "a") == 0) {
        cmd->chord = A_MAJ;
    }
    else if (strcmp(state->chord, "b") == 0) {
        cmd->chord = B_MAJ;
    }
    else if (strcmp(state->chord, "g") == 0) {
        cmd->chord = G_MAJ;
    }
}

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

user_info * find_user(int id, user_list ** users)
{
    user_list * p;
    user_list * q;
    user_list * new_node;
    for(p = *users; p != NULL; p = p->next) {
        if (p->user.id == id) {
            return &(p->user);
        }
        q = p;
    }
    /* create new user */
    new_node = malloc(sizeof(user_list));
    new_node->user.id = id;
    new_node->user.thread = 0;
    new_node->user.valid_thread = 0;
    new_node->user.cmd.control = STOP;
    new_node->user.cmd.chord = G_MAJ;
    if (*users == NULL) {
        *users = new_node;
    }
    else {
        q->next = new_node;
    }

    return &(new_node->user);
}

static int get_freq(int note)
{
   return pow(2.0, (note-40)/12.0)*440;
}

void play_tone(chord_t chord) {
    int root, fifth;
    double root_freq, fifth_freq;
    root = (chord_t)chord;
    syslog(LOG_INFO, "Root is %d", root);
    fifth = root + 7;
    /* Piano Note to frequency */
    root_freq = get_freq(root);
    fifth_freq= get_freq(fifth);
    char buffer[200];
    sprintf(buffer, "play -q -n synth 5 sine %f sine %f gain -10.0",
            root_freq, fifth_freq);
    system(buffer);
    //pthread_exit(NULL);
}

void run_user_cmd(user_info * user, cmd_t * cmd)
{
    /* if smae command do nothing */
    if (memcmp(&(user->cmd), cmd, sizeof(cmd_t)) == 0) {
        syslog(LOG_INFO, "COMMAND the same");
        return;
    }
    else {
        /* stop thread if running*/
        if (user->valid_thread) {
            /* stop thread */
            user->valid_thread = 0;
        }
        /* run new command */
        memcpy(&(user->cmd), cmd, sizeof(cmd_t));
        /* start new thread */
        play_tone(user->cmd.chord);

        /* set thread as valid */
        user->valid_thread = 1;
        syslog(LOG_INFO, "COMMAND changed");
    }

}
int main(void)
{
    
    /* Process and session ID */
    pid_t pid, sid;
    state_t * state = NULL;
    if ((state = malloc(sizeof(state_t))) == NULL) {
        exit(EXIT_FAILURE);
    }
    cmd_t * cmd = NULL;
    if ((cmd = malloc(sizeof(cmd_t))) == NULL) {
        exit(EXIT_FAILURE);
    }
    user_list * users = NULL;
    user_info * user = NULL;

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
            #if DEBUG
            post_state_to_log(state);
            #endif
            parse_state(state, cmd);
            user = find_user(state->id, &users);
            run_user_cmd(user, cmd);
        }
        //parse_message();
        sleep(SLEEP_TIME);
    }

    free(state);
    free(cmd);

    return 0;
}
