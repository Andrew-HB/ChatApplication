#ifdef __linux__
#include "Linux/LinuxClientApi.h"
#elif _WIN32
#include "Windows/WinClientApi.h"
#endif

int main() {
	Client* client = new Client;
	client->Run();
}