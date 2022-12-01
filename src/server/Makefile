ifdef OS
	ServerApi = WinServerApi
	Folder = Windows
	Lib = -lws2_32
	RM = del
else
	ServerApi = LinuxServerApi
	Folder = Linux
	Lib = 
	RM = rm -rf
endif

all: server

server: server-application.o $(ServerApi).o
	g++ server-application.o $(ServerApi).o -o server $(Lib)
	./server

server-application.o: server-application.cpp
	g++ -c server-application.cpp

$(ServerApi).o: $(Folder)/$(ServerApi).cpp
	g++ -c $(Folder)/$(ServerApi).cpp

clean:
	$(RM) *.o

#all: server
#	
#server: server-application.o WinServerApi.o
#	g++ server-application.o WinServerApi.o -o server -lws2_32
#	./server
#
#server-application.o: server-application.cpp
#	g++ -c server-application.cpp
#
#WinServerApi.o: Windows/WinServerApi.cpp
#	g++ -c Windows/WinServerApi.cpp
#
#clean:
#	del *.o
#
#
#all: server
#
#server: server-application.o LinuxServerApi.o
#	g++ server-application.o LinuxServerApi.o -o server
#	./server
#
#server-application.o: server-application.cpp
#	g++ -c server-application.cpp
#
#LinuxServerApi.o: Linux/LinuxServerApi.cpp
#	g++ -c Linux/LinuxServerApi.cpp
#
#clean:
#	rm -rf *.o
