#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string>
#include <algorithm>

#define MAX_CONNECTIONS 10

class Server {
public:

private:
    SOCKET servSocket;
    
    int SOCK_PORT;
    const char* SOCK_IP;

    int allConnections[MAX_CONNECTIONS];

public:
    Server();
    Server(const char* sock_ip, int sock_port);
    ~Server();
    
    void Run();
private:
    int MaxFD();
    void Init();
    SOCKET CreateSocket();
    void CloseAllConnections();
    int HandleMessage(char* msg, int fd_i);
};