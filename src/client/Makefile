ifdef OS
	ClientApi = WinClientApi
	Folder = Windows
	Lib = -lws2_32
	RM = del
else
	ClientApi = LinuxClientApi
	Folder = Linux
	Lib = 
	RM = rm -rf
endif

all: client

client: client-application.o $(ClientApi).o
	g++ client-application.o $(ClientApi).o -o client $(Lib)
	./client

client-application.o: client-application.cpp
	g++ -c client-application.cpp

$(ClientApi).o: $(Folder)/$(ClientApi).cpp
	g++ -c $(Folder)/$(ClientApi).cpp

clean:
	$(RM) *.o

#all: client
#
#client: client-application.o WinClientApi.o
#	g++ client-application.o WinClientApi.o -o client -lws2_32
#	./client
#
#client-application.o: client-application.cpp
#	g++ -c client-application.cpp
#
#WinClientApi.o: Windows/WinClientApi.cpp
#	g++ -c Windows/WinClientApi.cpp
#
#clean:
#	del *.o