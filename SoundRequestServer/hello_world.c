// Copyright (c) 2014 Cesanta Software
// All rights reserved
//
// This example demostrates basic use of Mongoose embedded web server.
// $Date: 2014-09-09 22:20:23 UTC $
//
// Modified by George Slavin
// Sun Nov 16 21:53:11 EST 2014

#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "mongoose.h"

#define FIFO_PATH "/home/thomas/EmbSysFinal/music_commands"
#define FIFO_ENABLE 1

typedef struct state_t {
   char control[8];
   char chord[20];
} state_t;

typedef struct conn_list_t {
   char ip[48];
   unsigned short port;
   state_t state;
   struct conn_list_t * next;
} conn_list_t;

static int write_to_fifo(state_t * state) {
    FILE * fifo;
    int rv = 0;
    fifo = fopen(FIFO_PATH, "w");
    if (fifo != NULL) {
        fputs(state->control, fifo);
        fputs(",", fifo);
        fputs(state->chord, fifo);
        fclose(fifo);
    }
    else {
        rv = 1;
    }
    return rv;
}

/* Content Rules:
 * control={start,stop,stop_all}
 * chord={A,B,C,D,E,F,G}
 */ 

static void parse_content(char * mesg, size_t length, state_t * state) {
    /* only the first line of the content is needed for now */
    /* chord=G&control=1 */
    /* parse the key value pairs */
    char * key, * value;
    char * args[2];
    int i;
    mesg[length] = '\0';
    printf("State Update request: %s\n", mesg);
    args[0] = strtok(mesg, "&");
    args[1] = strtok(NULL, "&");
    for(i = 0; i < 2; i++) {
        if (args[i] == NULL) {
            continue;
        }
        key = strtok(args[i], "=");
        value = strtok(NULL, "=");
        if (strcmp("control",key) == 0) {
            strcpy(state->control, value);
        }
        else if (strcmp("chord",key) == 0) {
            strcpy(state->chord, value);
        }
    }
}

state_t * get_state(char ip[48], unsigned short port, conn_list_t ** conns)
{   
    conn_list_t * p;
    conn_list_t * new_node;
    int i;
    for(p = *conns; p != NULL; p = p->next) {
        printf("cmp yields%d\n",memcmp(ip,p->ip, sizeof(p->ip)));
        if ((memcmp(ip,p->ip, sizeof(p->ip)) == 0)) {
            printf("Connection found\n");
            return &(p->state);
        }
    }
    new_node = malloc(sizeof(conn_list_t));
    memcpy(new_node->ip, ip, 48);
    new_node->port = port;
    strcpy(new_node->state.control, "stop");
    strcpy(new_node->state.chord, "g");
    new_node->next = NULL;
    if (*conns == NULL) {
        *conns = new_node;
    }
    else {
        p->next = new_node;
    }

    return &(new_node->state);
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
  static int fifo_error = 0;
  static conn_list_t * conns = NULL;
  state_t * state;
  /* state POSTs should be only be 3-4 chars */
  switch (ev) {
    case MG_AUTH: return MG_TRUE;
    case MG_REQUEST:
        if (strcmp("/state", conn->uri) == 0) {
            state = get_state(conn->remote_ip, conn->remote_port, &conns);
            if(strcmp("POST", conn->request_method) == 0) {
                parse_content(conn->content, conn->content_len, state);
                printf("State Updated: %s, %s\n", state->control, state->chord);
                mg_printf_data(conn, "%s,%s", state->control, state->chord);
            }
            else { 
                printf("State Requested\n");
                mg_printf_data(conn, "%s,%s", state->control, state->chord);
            }
            #if FIFO_ENABLE
            if(write_to_fifo(state)) {
                fifo_error = 1;
            }
            #endif
        }
        else {
            mg_printf_data(conn, "Hello! Requested URI is [%s]", conn->uri);
        }
      return MG_TRUE;
    default: return MG_FALSE;
  }
}

int main(void) {
  struct mg_server *server;

  // Create and configure the server
  server = mg_create_server(NULL, ev_handler);
  mg_set_option(server, "listening_port", "8082");

  // Serve request. Hit Ctrl-C to terminate the program
  printf("Starting on port %s\n", mg_get_option(server, "listening_port"));
  for (;;) {
    // Polls every second
    mg_poll_server(server, 1000);
  }

  // Cleanup, and free server instance
  mg_destroy_server(&server);

  return 0;
}
