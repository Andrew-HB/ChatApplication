#ifdef __linux__
#include "Linux/LinuxServerApi.h"
#elif _WIN32
#include "Windows/WinServerApi.h"
#endif

int main() {
    #pragma comment(lib, "ws2_32.lib")
    Server* server = new Server;

    server->Run();
}