#ifndef Server
#define Server

void *requestHandler(void *thread_client_sd);
void getAllPotholes(int client_sd, sqlite3* database);
void getNearPotholes(int client_sd, sqlite3* database);
void insertPotholes(int client_sd, sqlite3* database);
void getThreshold(int client_sd, sqlite3* database);

#endif 