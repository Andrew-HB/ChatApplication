#include <arpa/inet.h> //inet_pton
#include <cstring>     //memset
#include <errno.h>		//strerror
#include <stdio.h>	//printf
#include <iostream>
#include <sys/socket.h>
#include <unistd.h> //close
#include <string>
#include <algorithm>

class Client {
public:
    
private:
    int clientSocket;

    int SOCK_PORT;
    const char* SOCK_IP;

public:
    Client();
    ~Client();

    void Run();

private:
    void Init();
    int CreateSocket();
    int HandleMessage(char* msg);
};