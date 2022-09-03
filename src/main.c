#include "Headers/server_header.h"


//? Static Server Socket descriptor 
static int server_sd;

int main(int argc, char const *argv[])
{
    //? Connected Client Socket
    int connected_sd;
    //? Client Socket Pointer for Multithread
    int* client_sd;
    //? Server address struct
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    //? Socket Buffer 
    char buffer[1024] = { 0 };





    return 0;
}
