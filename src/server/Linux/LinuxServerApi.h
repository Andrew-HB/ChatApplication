#include <arpa/inet.h>  //inet_pton
#include <cstring>      //memset
#include <errno.h>	    //strerror
#include <stdio.h>	    //printf
#include <sys/socket.h>
#include <unistd.h>     //close
#include <stdlib.h>	    //exit
#include <fcntl.h>
#include <string>
#include <algorithm>

#define MAX_CONNECTIONS 10

class Server {
public:

private:
    int servSocket;
    
    int SOCK_PORT;
    const char* SOCK_IP;

    int allConnections[MAX_CONNECTIONS];

public:
    Server();
    Server(const char* sock_ip, int sock_port);
    ~Server();
    
    void Run();
private:
    void Init();
    int CreateSocket();
    void CloseAllConnections();
    int HandleMessage(char* msg, int fd_i);
};