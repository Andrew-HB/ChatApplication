#include "WinClientApi.h"

#include <iostream>

#define BUFFER_SIZE 128

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
const char* get_error_text()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return 0; //No error message has been recorded
    }
    
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
            
    return message.c_str();
}

Client::Client(): SOCK_IP("127.0.0.1"), SOCK_PORT(7300) {
    this->Init();
}

Client::~Client() {
    //closesocket(this->clientSocket);
	shutdown(this->clientSocket, SD_BOTH);
}

void Client::Init() {
    this->clientSocket = this->CreateSocket();
    if (this->clientSocket == -1) {
        printf("Failed to create client!");
    }
}

unsigned int Client::CreateSocket() {
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
    }
    
    unsigned int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0) {
		//printf("socket: %s\n", get_error_text());
		std::cout << "socket: " << WSAGetLastError() << std::endl;
        return -1;
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(SOCK_PORT);
	inet_pton(AF_INET, SOCK_IP, &sa.sin_addr);

	if(connect(sock, (struct sockaddr*)&sa, sizeof sa) == -1) {
		//printf("connect: %s\n", get_error_text());
		std::cout << "connect: " << WSAGetLastError() << std::endl;
        //closesocket(this->clientSocket);
		shutdown(this->clientSocket, SD_BOTH);
        return -1;
	}

	printf("Connected to %s:%d\n", this->SOCK_IP, this->SOCK_PORT);

	return sock;
}

int Client::HandleMessage(char* msg) {
	std::string str(msg);

	std::string res;
    std::size_t found = str.find(std::string("NewPort-<"));
	if (found != std::string::npos) {
        printf("Found new port\n");
		found += 9;
		while(str[found] != '>') {
			res += str[found];
			found++;
		}
		//printf("New port: %s", res.c_str());
		if (!res.empty() && 
	    std::find_if(res.begin(), res.end(), [](unsigned char c) { return !std::isdigit(c); }) == res.end()) {
        	//printf("New port: %s\n", res.c_str());
        	this->SOCK_PORT = std::stoi(res);
        	//closesocket(this->clientSocket);
			shutdown(this->clientSocket, SD_BOTH);
			printf("Connection moved to a new port: %s\n\n\n", res.c_str());
        	this->clientSocket = this->CreateSocket();

			return 1;
    	}
	}
    
	return -1;
}

void Client::Run() {
    fd_set fd_sets;
	
	fd_sets.fd_count = 2;
	fd_sets.fd_array[0] = this->clientSocket;
	fd_sets.fd_array[1] = 0;

	char recv_buffer[BUFFER_SIZE];
	char send_buffer[BUFFER_SIZE];

    while(true) {
		FD_ZERO(&fd_sets);

		FD_SET(this->clientSocket, &fd_sets);
		FD_SET(0, &fd_sets);

		if (select(0, &fd_sets, 0, 0, 0) == SOCKET_ERROR) {
			//printf("select: %s\n", GetLastError());
			std::cout << "select error: " << WSAGetLastError() << std::endl;
		}
		
		if (FD_ISSET(this->clientSocket, &fd_sets)) {
			recv(this->clientSocket, recv_buffer, BUFFER_SIZE, 0);
			if(this->HandleMessage(recv_buffer) != -1) {
				continue;
			}
			printf("\n%s", recv_buffer);
			memset(recv_buffer, 0, sizeof recv_buffer);
		}
		if (FD_ISSET(0, &fd_sets)) {
			fgets(send_buffer, BUFFER_SIZE, stdin);
			send(this->clientSocket, send_buffer, strlen(send_buffer), 0);
			//printf("%s\n", send_buffer);
			memset(send_buffer, 0, sizeof send_buffer);
		}
    }
}