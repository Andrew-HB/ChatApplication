#include "LinuxClientApi.h"

#define BUFFER_SIZE 128

Client::Client(): SOCK_IP("127.0.0.1"), SOCK_PORT(7300) {
    this->Init();
}

Client::~Client() {
    close(this->clientSocket);
}

void Client::Init() {
    this->clientSocket = this->CreateSocket();
    if (this->clientSocket == -1) {
        printf("Failed to create client!");
    }
}

int Client::CreateSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sock < 0) {
		int err = errno;
		printf("socket: strerror=%d: %s \n", err, strerror(err));
        return -1;
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(SOCK_PORT);
	inet_pton(AF_INET, SOCK_IP, &sa.sin_addr);

	if(connect(sock, (struct sockaddr*)&sa, sizeof sa) == -1) {
		int err = errno;
		printf("connect: strerror=%d: %s \n", err, strerror(err));
        close(this->clientSocket);
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
        //printf("Found new port\n");
		found += 9;
		//while(str[found] != '>' || str[found] != str.length() - 1) {
		while(str[found] != '>') {
			res += str[found];
			found++;
		}
		//printf("New port: %s", res.c_str());
		if (!res.empty() && 
	    std::find_if(res.begin(), res.end(), [](unsigned char c) { return !std::isdigit(c); }) == res.end()) {
        	//printf("New port: %s\n", res.c_str());
        	this->SOCK_PORT = std::stoi(res);
        	close(this->clientSocket);
			printf("Connection moved to a new port: %s\n\n\n", res.c_str());
        	this->clientSocket = this->CreateSocket();

			return 1;
    	}
	}
    
	return -1;
}

void Client::Run() {
    fd_set read_fd_set;

	char recv_buffer[BUFFER_SIZE];
	char send_buffer[BUFFER_SIZE];

    while(true) {
		FD_ZERO(&read_fd_set);

		FD_SET(this->clientSocket, &read_fd_set);
		FD_SET(0, &read_fd_set);

		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			int err = errno;
			printf("select: strerror=%d: %s \n", err, strerror(err));
		}
		
		if (FD_ISSET(this->clientSocket, &read_fd_set)) {
			recv(this->clientSocket, recv_buffer, BUFFER_SIZE, 0);
			if(this->HandleMessage(recv_buffer) != -1) {
				continue;
			}
			printf("\n%s", recv_buffer);
			memset(recv_buffer, 0, sizeof recv_buffer);
		}
		if (FD_ISSET(0, &read_fd_set)) {
			fgets(send_buffer, BUFFER_SIZE, stdin);
			send(this->clientSocket, send_buffer, strlen(send_buffer), 0);
			//printf("%s\n", send_buffer);
			memset(send_buffer, 0, sizeof send_buffer);
		}
    }
}