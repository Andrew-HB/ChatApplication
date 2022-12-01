#include "LinuxServerApi.h"

#define BUFFER_SIZE 128

Server::Server(): SOCK_IP("127.0.0.1"), SOCK_PORT(7300) {
    this->Init();
}

Server::Server(const char* sock_ip, int sock_port): SOCK_IP(sock_ip), SOCK_PORT(sock_port) {
    this->Init();
}

Server::~Server() {
    this->CloseAllConnections();
    close(this->servSocket);
}

void Server::Init() {
    this->servSocket = this->CreateSocket();
    if (this->servSocket == -1) {
        printf("Failed to create server!");
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        this->allConnections[i] = -1;
    }
    this->allConnections[0] = this->servSocket;
}

int Server::CreateSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        int err = errno;
		printf("socket: strerror=%d: %s \n", err, strerror(err));
        return -1;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SOCK_PORT);
    inet_pton(AF_INET, SOCK_IP, &sa.sin_addr);

    if (bind(sock, (struct sockaddr*)&sa, sizeof sa) == -1) {
        int err = errno;
		printf("bind: strerror=%d: %s \n", err, strerror(err));
        close(sock);
        return -1;
    }

    if (listen(sock, 5) == -1) {
        int err = errno;
		printf("listen: strerror=%d: %s \n", err, strerror(err));
        close(sock);
        return -1;
    }

    return sock;
}

void Server::CloseAllConnections() {
    for (int i = 1; i < MAX_CONNECTIONS; i++) {
        if(allConnections[i] > 0) {
            printf("Closing connection for fd: %d\n", this->allConnections[i]);
            close(allConnections[i]);
            allConnections[i] = -1;
        }
    }
}

int Server::HandleMessage(char* msg, int fd_i) {
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

            printf("New port: %s\n", res.c_str());
            //this->CloseAllConnections();
            this->SOCK_PORT = std::stoi(res);
            close(this->servSocket);
            this->servSocket = this->CreateSocket();

            for (int i = 1; i < MAX_CONNECTIONS; i++) {
                if (this->allConnections[i] > 0) {
                    send(allConnections[i], str.c_str(), str.length(), 0);
                }
            }
            
            return 1;
        } else {
            printf("Incorrect port\n");
            return 1;
        }
	}
    
    return -1;
}

void Server::Run() {
    fd_set read_fd_set;

    char buffer[BUFFER_SIZE];

    while(true) {
        FD_ZERO(&read_fd_set);

        for(int i = 0; i < MAX_CONNECTIONS; i++) {
            if(this->allConnections[i] >= 0) {
                FD_SET(this->allConnections[i], &read_fd_set);
            }
        }

        //printf("Using select() to listen for incoming events\n");
        int ret_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);

        //printf("select() woke up!\n");
        if (ret_val >= 0) {
            //printf("select() returned with %d\n", ret_val);

            if (FD_ISSET(this->servSocket, &read_fd_set)) {
                //printf("Accepting new connection.\n");

                struct sockaddr_in new_addr;
                socklen_t new_addr_len;
                int new_fd = accept(this->servSocket, (struct sockaddr*)&new_addr, &new_addr_len);

                if (new_fd >= 0) {
                    printf("Accepted a new connection with fd:%d\n", new_fd);

                    for (int i = 1; i < MAX_CONNECTIONS; i++) {
                        if (this->allConnections[i] < 0) {
                            this->allConnections[i] = new_fd;

                            std::string toSend = "Welcome to the chat, user";
                            toSend += std::to_string(this->allConnections[i]);
                            toSend += "!\n";
                            
                            send(this->allConnections[i], toSend.c_str(), toSend.length(), 0);

                            break;
                        }
                    }
                } else {
                    int err = errno;
		            printf("accept: strerror=%d: %s \n", err, strerror(err));
                }

                ret_val--;
                if(!ret_val) continue;
            }

            for (int i = 1; i < MAX_CONNECTIONS; i++) {
                if ((this->allConnections[i] > 0) && (FD_ISSET(this->allConnections[i], &read_fd_set))) {
                    //printf("Returned fd is %d [index: i = %d]\n", this->allConnections[i], i);

                    ret_val = recv(this->allConnections[i], buffer, BUFFER_SIZE, 0);
                    if (ret_val == 0) {
                        printf("Closing connection for fd: %d\n", this->allConnections[i]);
                        close(this->allConnections[i]);
                        this->allConnections[i] = -1;
                    } else if (ret_val > 0) {
                        if (this->HandleMessage(buffer, i) != -1) {
                            continue;
                        }

                        buffer[ret_val] = '\0';
                        std::string toSend = "<user";
                        toSend += std::to_string(this->allConnections[i]);
                        toSend += ">: ";
                        toSend += buffer;

                        //printf("Received data (len %d bytes, fd: %d): %s\n", ret_val, this->allConnections[i], buffer);
                        
                        for (int j = 1; j < MAX_CONNECTIONS; j++) {
                            if (this->allConnections[j] > 0 && j != i) {
                                //printf("Sending %d bytes to %d.\n", ret_val, all_connections[i]);
                                send(this->allConnections[j], toSend.c_str(), toSend.length(), 0);
                            }
                        }
                        memset(buffer, 0, sizeof buffer);
                    } else {
                        int err = errno;
		                printf("recv: strerror=%d: %s \n", err, strerror(err));
                    }
                }

                ret_val--;
                if (!ret_val) continue;
            }
        }

    }
}