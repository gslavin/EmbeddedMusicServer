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
#include "sound_server.h"

/* returns postive value on error */
static int write_to_fifo(state_t * state) {
    FILE * fifo;
    int rv = 0;
    fifo = fopen(FIFO_PATH, "wb");
    if (fifo != NULL) {
        fwrite(state, sizeof(state_t), 1, fifo);
    }
    else {
        rv = 1;
    }
    fclose(fifo);
    return rv;
}

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
    conn_list_t * q;
    conn_list_t * new_node;
    static int current_id = 0;
    for(p = *conns; p != NULL; p = p->next) {
        if ((memcmp(ip,p->ip, sizeof(p->ip)) == 0)) {
            printf("Connection found\n");
            return &(p->state);
        }
        q = p;
    }
    new_node = malloc(sizeof(conn_list_t));
    memcpy(new_node->ip, ip, 48);
    new_node->port = port;
    new_node->state.id = current_id++;
    strcpy(new_node->state.control, "stop");
    strcpy(new_node->state.chord, "g");
    new_node->next = NULL;
    if (*conns == NULL) {
        *conns = new_node;
    }
    else {
        q->next = new_node;
    }

    return &(new_node->state);
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
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
                /* write state to fifo, fifo blocks until read from*/
                #if FIFO_ENABLE
                if(write_to_fifo(state)) {
                    mg_printf_data(conn, "%s", "FIFO ERROR");
                } 
                else
                #endif
                {
                    printf("State Updated: %s, %s\n", state->control, state->chord);
                    mg_printf_data(conn, "%s,%s", state->control, state->chord);
                }
            }
            else { 
                printf("State Requested\n");
                mg_printf_data(conn, "%s,%s", state->control, state->chord);
            }
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
