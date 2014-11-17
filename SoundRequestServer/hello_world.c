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

typedef struct state_t {
   int is_playing;
   char chord[20];
} state_t;

static int write_to_fifo(const char * cmd) {
    FILE * fifo;
    int rv = 0;
    fifo = fopen(FIFO_PATH, "w");
    if (fifo != NULL) {
        fputs(cmd, fifo);
        fclose(fifo);
    }
    else {
        rv = 1;
    }
    return rv;
}

static void parse_content(char * mesg, state_t * state) {
    /* only the first line of the content is needed for now */
    /* chord=G&is_playing=1 */
    /* parse the key value pairs */
    char * end;
    char * key, * value;
    char * args[2];
    int i;

    end = strchr(mesg, '\n');
    end[0] = '\0';
    printf("State Update request: %s\n", mesg);
    args[0] = strtok(mesg, "&");
    args[1] = strtok(NULL, "&");
    for(i = 0; i < 2; i++) {
        key = strtok(args[i], "=");
        value = strtok(NULL, "=");
        if (strcmp("chord",key) == 0) {
            strcpy(state->chord, value);
        }
        else if (strcmp("is_playing",key) == 0) {
            state->is_playing = atoi(value);
        }
    }
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
  static int fifo_error = 0;
  static state_t state = {0,"none"};
  /* state POSTs should be only be 3-4 chars */
  switch (ev) {
    case MG_AUTH: return MG_TRUE;
    case MG_REQUEST:
        if (strcmp("/state", conn->uri) == 0) {
            if(strcmp("POST", conn->request_method) == 0) {
                parse_content(conn->content, &state);
                printf("State Updated: %d, %s\n", state.is_playing, state.chord);
                mg_printf_data(conn, "%d,%s", state.is_playing, state.chord);
            }
            else { 
                printf("State Requested\n");
                mg_printf_data(conn, "%d,%s", state.is_playing, state.chord);
            }
        }
        else {
            mg_printf_data(conn, "Hello! Requested URI is [%s]", conn->uri);
        }
        #if 0
        if(write_to_fifo(conn->uri)) {
            fifo_error = 1;
        }
        #endif
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
