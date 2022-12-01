#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <WinBase.h>
#include <stdio.h>
#include <string>
#include <algorithm>

class Client {
public:
    
private:
    unsigned int clientSocket;

    int SOCK_PORT;
    const char* SOCK_IP;

public:
    Client();
    ~Client();

    void Run();

private:
    void Init();
    unsigned int CreateSocket();
    int HandleMessage(char* msg);
};