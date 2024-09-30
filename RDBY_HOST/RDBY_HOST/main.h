#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <processthreadsapi.h>   
#include <math.h>
#include <WinUser.h>

#pragma comment(lib, "Ws2_32.lib")
#define PORT "36542"
#define BACKLOG 15
#define MAX_REQ_SIZE 1024


typedef struct client {

	SOCKET socket;

}client_t, * clientPtr;