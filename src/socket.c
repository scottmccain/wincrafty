#include <WinSock.h>

#define DEFAULT_PORT 27015

SOCKET ClientSocket = INVALID_SOCKET;

//When using select() multiple sockets may have errors
//This function will give us the socket specific error
//WSAGetLastError() can't be relied upon
int GetSocketSpecificError(SOCKET Socket)
{
     int nOptionValue;
     int nOptionValueLength = sizeof(nOptionValue);

     //Get error code specific to this socket
     getsockopt(Socket, SOL_SOCKET, SO_ERROR, (char*)&nOptionValue, 
                                            &nOptionValueLength);

     return nOptionValue;
}

int init_sockets()
{
	WSADATA wsaData;   // if this doesn't work

	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKADDR_IN server;

	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
		Print(4095, "WSAStartup failed.\n");
		return 1;
	}

    server.sin_port=htons (DEFAULT_PORT);
    server.sin_family = AF_INET;
 
    server.sin_addr.s_addr = INADDR_ANY;

	// Create a SOCKET for connecting to server
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, (SOCKADDR *)(&server), sizeof(server));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

	iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

	return 0;
}

void shutdown_sockets()
{
	closesocket(ClientSocket);
	WSACleanup();
}

int check_socket()
{
	struct fd_set readfds;
	struct fd_set exceptfds;
	struct timeval timeout;

	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);

	FD_SET(ClientSocket, &readfds);
	FD_SET(ClientSocket, &exceptfds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if(select(0, &readfds, NULL, &exceptfds, &timeout) > 0) 
	{
        //Check in Exception Set
        if (FD_ISSET(ClientSocket, &exceptfds))
        {
			printf("\nError occurred on the socket: %d.", 
			GetSocketSpecificError(ClientSocket));
			CraftyExit(1);
        }

		return FD_ISSET(ClientSocket, &readfds);
	}

	return 0;
}

int read_socket(char * dst, unsigned int count)
{
	int bytes;

	bytes = recv(ClientSocket, dst, count, 0);

	return 0;
}
