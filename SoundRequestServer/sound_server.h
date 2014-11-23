#define FIFO_PATH "/home/thomas/EmbSysFinal/music_commands"
#define FIFO_ENABLE 1

/* Content Rules:
 * control={start,stop,stop_all}
 * chord={A,B,C,D,E,F,G}
 */ 

typedef struct state_t {
   int id;
   char control[8];
   char chord[8];
} state_t;

typedef struct conn_list_t {
   char ip[48];
   unsigned short port;
   state_t state;
   struct conn_list_t * next;
} conn_list_t;
