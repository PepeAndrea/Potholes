#ifndef Database
#define Database

void getAllPotholes_DB(int client_sd, sqlite3* database);
void getNearPotholes_DB(int client_sd, sqlite3* database,double latitude,double lonitude, double radius);
void insertPotholes_DB(int client_sd, sqlite3* database, char* username,double latitude,double longitude, double variation);
void getThreshold_DB(int client_sd, sqlite3* database);

#endif 