#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sqlite3.h>
#include <netdb.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    sqlite3* database;
    char* query;
    int db_conn_status,query_status;

    //! Creo il database
    db_conn_status = sqlite3_open_v2("Database/database.db", &database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, NULL);

    if (db_conn_status != SQLITE_OK)
    {
        perror("Errore apertura db");
        exit(EXIT_FAILURE);
    }

    //! Riprisino le tabelle
    query = sqlite3_mprintf("DROP TABLE IF EXISTS Potholes");
    query_status = sqlite3_exec(database, query, NULL, NULL, NULL);
    //? Ad ogni query possiamo eseguire la free per liberare memoria
    sqlite3_free(query);

    query = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS Potholes (username VARCHAR(20), latitude DOUBLE, longitude DOUBLE, variation DOUBLE);");
    query_status = sqlite3_exec(database, query, NULL, NULL, NULL);
    sqlite3_free(query);

    //! Valori iniziali per popolare il db
    //? TOGLIERE COMMENRI SOLO PER SCOPI DI DEBUG
    /*
    for (int i = 0; i < 10; i++)
    {
        query = sqlite3_mprintf("INSERT INTO Potholes VALUES ('%s', %d, %d, %d);", "Andrea", i, i, rand()%9);
        query_status = sqlite3_exec(database, query, NULL, NULL, NULL);
        if (query_status != SQLITE_OK)
        {
            perror("Errore inserimento");
        }
        sqlite3_free(query);
    }
    */

    return 0;
}
