#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<syslog.h>
#include<string.h>

#define FILE_NAME "/home/thomas/EmbSysFinal/music_commands"

int main()
{
    FILE * fifo;
    char buffer[200];
    const char * fifo_path;
    int rv = 0;
    
    while (1) {
        fifo = fopen(FILE_NAME, "r");
        rv = fscanf(fifo, "%s", buffer);
        if (rv == 1) {
            printf("%s\n", buffer);
        }
        fclose(fifo);
    }
    
    return 0;
}
