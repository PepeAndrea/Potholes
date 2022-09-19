#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sqlite3.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include "../Headers/server_header.h"
#include "../Headers/helper_header.h"
#include "../Headers/database_header.h"

#define MAX_BUFFER 150
#define PORT 8585
#define THRESHOLD 15 

//? Static Server Socket descriptor 
static int server_sd;

int main(int argc, char const *argv[])
{
    //? Connected Client Socket
    int client_sd;
    //? Server address struct
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    //? Client Socket Pointer for Multithread
    int* thread_client_sd;
    //?Client data
    struct sockaddr_in client_address;
    int client_addrlen = sizeof(client_address);

    //? Thread data
    int thread_attr_status;
    pthread_attr_t thread_attr;
    int create_request_thread_status;
    pthread_t requets_thread;


    //! Setup signal handler for server shutdown
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    signal(SIGINT, signalHandler);


    //! Create socket
    if ((server_sd = socket(AF_INET, SOCK_STREAM,0)) == 0) {
        log_e("socket","Errore durante la creazione del socket");
        exit(EXIT_FAILURE);
    }else{
        log_m("Socket Success", "Socket creata con successo");
    }
    

    //! Set address
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    //! Binding socket
    if (bind(server_sd, (struct sockaddr*)&address,addrlen)< 0) {
        log_e("bind","Errore durante il binding");
        exit(EXIT_FAILURE);
    }else{
        log_m("Bind Success", "Bind eseguito con successo");

    }

    //!Set socket on listening
    if (listen(server_sd,15) < 0) {
        log_e("listen","Errore durante l'ascolto");
        exit(EXIT_FAILURE);
    }else{
        log_m("Listen Success", "Listen eseguito con successo");
    }

    //! Accept connetion by client.
    while ((client_sd = accept(server_sd,(struct sockaddr*)&client_address, (socklen_t*)&client_addrlen)) != -1) {

        //! Get client ip
        char client_ip[25];
        if(inet_ntop(AF_INET, &client_address.sin_addr, client_ip, sizeof(client_ip))) {
            log_m("Indirizzo ip Client connesso", client_ip);
        }

        //! Setup Thread
        thread_attr_status = pthread_attr_init(&thread_attr);
        if(thread_attr_status!=0) 
            log_e("pthread_attr_init", "Errore durante l'iniziallizzazione degli attributi");
        else
            log_m("pthread_attr_init", "Attributi inizializzati con successo");

        //?Set Thread Detach State
        thread_attr_status = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
        if(thread_attr_status!=0)
            log_e("pthread_attr_setdetachstate", "Errore durante l'impostazione dello stato detach");
        else
            log_m("pthread_attr_setdetachstate", "Stato detach impostato");

        //? Memory allocation for socket descriptor    
        thread_client_sd = malloc(sizeof(int));
        *(thread_client_sd) = client_sd;

        //! Start request thread
        create_request_thread_status = pthread_create(&requets_thread, &thread_attr, requestHandler, (void *) thread_client_sd);

        if(create_request_thread_status < 0)
            log_e("pthread_create", "Errore durante la creazione di un nuovo thread");
        else    
            log_m("pthread_create", "Thread creato con successo");

        pthread_attr_destroy(&thread_attr);

    }

    log_e("accept", "Errore durante l'accept");
    close(server_sd);

    return 0;
}

//? Callback di partenza del thread
void *requestHandler(void *thread_client_sd) {
    
    //? Set client socket descriptor
    int client_sd = *(int *) thread_client_sd;
    //? Socket Buffer
    char buffer[MAX_BUFFER] = { 0 };
    bzero(buffer,sizeof(buffer));
    //? SQLite DB
    sqlite3 *database;
    int sqlite3_status;

    //! Open db
    sqlite3_status = sqlite3_open_v2("Database/database.db", &database, SQLITE_OPEN_READWRITE |SQLITE_OPEN_NOMUTEX, NULL);

    if(sqlite3_status != SQLITE_OK){
        send(client_sd, "Impossibile collegarsi al Database\n", strlen("Impossibile collegarsi al Database\n"), 0);
        log_e("sqlite3_open_v2","Impossibile collegarsi al database");
        return NULL;
    }
    log_m("sqlite3_open_v2","Connessione al database avvenuta con successo");

    log_m("Thread start", "Request Thread avviato");

    //! Read Action Request
    if(recv(client_sd, buffer, sizeof(buffer),0)==-1)
        log_e("recv error","Errore durante la riczione della richiesta");
    else{
        log_m("Richiesta ricevuta dal client",buffer);

        //! Dispatch Action
        if(strcmp(buffer, "getAllPotholes") == 0){
            getAllPotholes(client_sd, database);
        }else if(strcmp(buffer, "getNearPotholes") == 0){
            getNearPotholes(client_sd, database);
        }else if(strcmp(buffer, "insertPotholes") == 0){
            insertPotholes(client_sd, database);
        }else if(strcmp(buffer, "getThreshold") == 0){
            getThreshold(client_sd, database);
        }else{
            log_e("404", "Action non gestibile dal server");
            send(client_sd, "ERROR 404 - Action non gestibile\n", strlen("ERROR 404 - Action non gestibile\n"), 0);
            sleep(2);
        }
    }
    
    //! Free Socket memory allocation
    free(thread_client_sd);
    //! Close client connection socket
    close(client_sd);
    return NULL;
}


//? Handle getAllPotholes request
void getAllPotholes(int client_sd, sqlite3* database){
    getAllPotholes_DB(client_sd,database);
}

//? Handle getNearPotholes request
void getNearPotholes(int client_sd, sqlite3* database){

    char username[20];
    double latitude = 0, longitude = 0, raggio = 0;

    //? Socket Buffer
    char buffer[MAX_BUFFER] = { 0 };
    bzero(buffer,sizeof(buffer));

    //! Check if data sent by client are valid
    recv(client_sd, buffer, sizeof(buffer), 0);
    log_m("Dati inviati dal client",buffer);

    //? Get first token
    char *paramData = strtok(buffer, ";");
    if (paramData == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }

    //! Extract other params and fill variables
    strcpy(username, paramData);

    if ((paramData = strtok(NULL, ";")) == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }
    latitude = atof(paramData);
    
    if ((paramData = strtok(NULL, ";")) == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }
    longitude = atof(paramData);
    
    if ((paramData = strtok(NULL, ";")) == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }
    raggio = atof(paramData);

    //! Retrieve data from DB
    getNearPotholes_DB(client_sd,database,latitude,longitude,raggio);
}

//? Handle insertPotholes request
void insertPotholes(int client_sd, sqlite3* database){
    char username[20];
    double latitude = 0, longitude = 0, variation = 0;

    //? Socket Buffer
    char buffer[MAX_BUFFER] = { 0 };
    bzero(buffer,sizeof(buffer));

    //! Check if data sent by client are valid
    recv(client_sd, buffer, sizeof(buffer), 0);
    log_m("Dati inviati dal client",buffer);

    //? Get first token
    char *paramData = strtok(buffer, ";");
    if (paramData == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }

    //! Extract other params and fill variables
    strcpy(username, paramData);

    if ((paramData = strtok(NULL, ";")) == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }
    latitude = atof(paramData);
    
    if ((paramData = strtok(NULL, ";")) == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }
    longitude = atof(paramData);
    
    if ((paramData = strtok(NULL, ";")) == NULL){
        log_e("paramData","Parametri inviati non validi");
        return;
    }
    variation = atof(paramData);

    //! Insert data in DB
    insertPotholes_DB(client_sd,database,username,latitude,longitude,variation);
}

//? Handle getThreshold request
void getThreshold(int client_sd, sqlite3* database){
    char threshold[5];
    sprintf(threshold,"%d\n",THRESHOLD);
    send(client_sd, threshold, strlen(threshold), 0);
}

//? Handle signal from the OS
void signalHandler(int signal){
  char sig[10];
  sprintf(sig,"%d",signal);
  log_m("Signal ricevuto",sig);
  close(server_sd);
  log_m("Signal processato", "Server disattivato");
  exit(EXIT_SUCCESS);
}