#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <ws2def.h>
#include <string.h>
#include <math.h>
#include <WinUser.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")
#define PORT "36542"
#define SERVER_PORT 8080
#define SERVER_IP "10.100.102.25"
// default max request size.
#define MAX_REQ_SIZE 1024
// format for input binary header.
#define PACKET_FORMAT "%lu%lu%lu%lu%lu"
// sizeof(DWORD)*5
#define PACKET_SIZE 20
#define QUEUE_SIZE 1000
typedef struct client {

	SOCKET socket;

}client_t, * clientPtr;

typedef struct queue {

};


