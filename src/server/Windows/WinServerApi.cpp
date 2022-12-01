#include "WinServerApi.h"

//#pragma comment(lib, "ws2_32.lib")

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

Server::Server(): SOCK_IP("127.0.0.1"), SOCK_PORT(7300) {
    this->Init();
}

Server::Server(const char* sock_ip, int sock_port): SOCK_IP(sock_ip), SOCK_PORT(sock_port) {
    this->Init();
}

Server::~Server() {
    //this->CloseAllConnections();
    closesocket(this->servSocket);
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

SOCKET Server::CreateSocket() {
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)) {
        fprintf(stderr, "Failed to initialize.\n");
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
		printf("socket: %s\n", get_error_text());
        return -1;
    } else {
        //printf("Created socket()\n");
    }

    sockaddr_in sa;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(SOCK_PORT);
    inet_pton(AF_INET, SOCK_IP, &sa.sin_addr);

    if (bind(sock, (struct sockaddr*)&sa, sizeof sa) == -1) {
		printf("bind: %s\n", get_error_text());
        closesocket(sock);
        return -1;
    } else {
        //printf("Binded\n");
    }

    if (listen(sock, 5) == -1) {
		printf("listen: %s\n", get_error_text());
        closesocket(sock);
        return -1;
    } else {
        //printf("listen()\n");
    }

    return sock;
}

int Server::MaxFD() {
    int max = 0;
    for (int i = 1; i < MAX_CONNECTIONS; i++) {
        if (this->allConnections[i] > max)
            max = this->allConnections[i];
    }
    return max;
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
            closesocket(this->servSocket);
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
		            printf("accept: %s\n", get_error_text());
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
                        closesocket(this->allConnections[i]);
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
                            //if (this->allConnections[j] > 0) {
                                //printf("Sending %d bytes to %d.\n", ret_val, all_connections[i]);
                                send(this->allConnections[j], toSend.c_str(), toSend.length(), 0);
                            }
                        }
                        memset(buffer, 0, sizeof buffer);
                    } else {
		                //printf("recv: %s\n", get_error_text());
                    }
                }

                ret_val--;
                if (!ret_val) continue;
            }
        }
    }
}