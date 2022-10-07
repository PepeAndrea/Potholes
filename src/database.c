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
#include <math.h>
#include "../Headers/helper_header.h"
#include "../Headers/database_header.h"

#define MAX_DB_ROW 500


void getAllPotholes_DB(int client_sd, sqlite3* database){

    char *query;
    int query_status;
    char database_row_data[MAX_DB_ROW];
    sqlite3_stmt *stmt;

    query = sqlite3_mprintf("SELECT * FROM Potholes;");
    query_status = sqlite3_prepare_v2(database, query, strlen(query), &stmt, NULL);

    if (query_status != SQLITE_OK){
        log_e("sqlite3_prepare_v2","Impossibile eseguire query");
        return;
    }
    
    //? Iterate db response
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        //! Clean buffer  
        bzero((char*) &database_row_data, sizeof(database_row_data));

        //! Format string with ';' delimiter
        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 0));
        strcat(database_row_data, ";");

        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 1));
        strcat(database_row_data, ";");

        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 2));
        strcat(database_row_data, ";");

        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 3));
        strcat(database_row_data, ";\n");

        send(client_sd,database_row_data,strlen(database_row_data),0);
    }

    sqlite3_free(query);
    sqlite3_finalize(stmt);
}



void getNearPotholes_DB(int client_sd, sqlite3* database,double latitude,double longitude, double radius){
    char *query;
    int query_status;
    char database_row_data[MAX_DB_ROW];
    sqlite3_stmt *stmt;

    query = sqlite3_mprintf("SELECT * FROM Potholes WHERE latitude >= ? AND latitude <= ? AND longitude >= ? AND longitude <= ?;");
    query_status = sqlite3_prepare_v2(database, query, strlen(query), &stmt, NULL);

    if (query_status != SQLITE_OK){
        log_e("sqlite3_prepare_v2","Impossibile eseguire query");
        return;
    }

    //? Latitude-Longitude Bounding box
    //1 deg latitude ~ 111km
    //1 deg longitude ~ 111*cos(latitude*pi/180) km    
    double latBox = radius/111;
    double cosArg = latitude*M_PI/180;
    double longBox =(radius * cos(cosArg))/(111*cos(cosArg));



    //! Binding data in prepared statement
    sqlite3_bind_double(stmt,1,latitude-latBox);
    sqlite3_bind_double(stmt,2,latitude+latBox);
    sqlite3_bind_double(stmt,3,longitude-longBox);
    sqlite3_bind_double(stmt,4,longitude+longBox);

    log_m("Query preparata",sqlite3_expanded_sql(stmt));
    
    //? Iterate db response
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        //! Clean buffer  
        bzero((char*) &database_row_data, sizeof(database_row_data));

        //! Format string with ';' delimiter
        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 0));
        strcat(database_row_data, ";");

        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 1));
        strcat(database_row_data, ";");

        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 2));
        strcat(database_row_data, ";");

        strcat(database_row_data, (char *)sqlite3_column_text(stmt, 3));
        strcat(database_row_data, ";\n");

        send(client_sd,database_row_data,strlen(database_row_data),0);
    }

    sqlite3_free(query);
    sqlite3_finalize(stmt);
}


void insertPotholes_DB(int client_sd, sqlite3* database, char* username,double latitude,double longitude, double variation){
    char *query;
    int query_status;
    char database_row_data[MAX_DB_ROW];
    sqlite3_stmt *stmt;

    query = sqlite3_mprintf("INSERT INTO Potholes VALUES (?,?,?,?);");
    query_status = sqlite3_prepare_v2(database, query, strlen(query), &stmt, NULL);

    if (query_status != SQLITE_OK){
        log_e("sqlite3_prepare_v2","Impossibile eseguire query");
        return;
    }

    //! Binding data in prepared statement
    sqlite3_bind_text(stmt,1,username,strlen(username),0);
    sqlite3_bind_double(stmt,2,latitude);
    sqlite3_bind_double(stmt,3,longitude);
    sqlite3_bind_double(stmt,4,variation);

    query = sqlite3_expanded_sql(stmt);
    log_m("Query preparata",query);
    
    //! Execute statement
    query_status = sqlite3_exec(database, query, NULL, NULL, NULL);
    if (query_status != SQLITE_OK){
        log_e("sqlite3_exec","Errore durante l'inserimento dei dati nel database");
        send(client_sd,"ERROR\n",strlen("ERROR\n"),0);
    }else{
        send(client_sd,"SUCCESS\n",strlen("SUCCESS\n"),0);
    }
    
    sqlite3_free(query);
    sqlite3_finalize(stmt);
}
void getThreshold_DB(int client_sd, sqlite3* database);