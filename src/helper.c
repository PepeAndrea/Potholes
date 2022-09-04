#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_LOG 1024

char *logfile = "Log/log.txt";

void log_m(char *tag, char *message){

    char toWrite[MAX_LOG];
    int file = open(logfile,O_RDWR|O_CREAT|O_APPEND,S_IRWXU);
    if (file != -1){
        sprintf(toWrite,"%s: %s\n",tag,message);
        write(file,toWrite,strlen(tag)+strlen(message)+3);
        close(file);
    }
}

void log_e(char *tag, char *message){

    char toWrite[MAX_LOG];
    int file = open(logfile,O_RDWR|O_CREAT|O_APPEND,S_IRWXU);
    if (file != -1){
        sprintf(toWrite,"ERRORE - %s: %s\n",tag,message);
        write(file,toWrite,strlen(tag)+strlen(message)+12);
        close(file);
    }
}