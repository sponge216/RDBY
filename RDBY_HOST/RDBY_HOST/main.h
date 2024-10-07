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

typedef struct mouse_data {
	LONG dx; // delta x
	LONG dy; // delta y
	DWORD mouse_flags;
	DWORD mouse_data;
}mouse_data_t, * pmouse_data;

//struct for keyboard press/release events.
typedef struct keyboard_data {
	DWORD vkCode;
	DWORD keyboard_flags;
}keyboard_data_t, * pkeyboard_data;

typedef struct packet_data {
	DWORD type;

	union {
		keyboard_data_t kbd_data;
		mouse_data_t ms_data;
	} DUMMYUNIONNAME;

}packet_data_t, * ppacket_data;