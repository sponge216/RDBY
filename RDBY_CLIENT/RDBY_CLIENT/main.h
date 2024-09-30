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
#define SERVER_PORT 36542
#define SERVER_IP "10.100.102.25"
#define SERVER_IP_X "172.20.10.7"
// default max request size.
#define MAX_REQ_SIZE 1024
// format for input binary header.
#define PACKET_FORMAT "%lu%lu%lu%lu%lu"
// sizeof(DWORD)*5
#define PACKET_SIZE 20
#define QUEUE_SIZE 1000
#define INITIAL_MESSAGE "0"
#define INITIAL_MESSAGE_SIZE 2
typedef struct client {

	SOCKET socket;

}client_t, * clientPtr;

typedef struct queue_cell {
	void* lParam; // struct pointer
	WPARAM wParam; // msg type
	BYTE type; // keyboard/mouse

} queue_cell_t, * pqueue_cell;

typedef struct queue {
	queue_cell_t arr[QUEUE_SIZE];
	WORD head;
	WORD tail;
	CRITICAL_SECTION access;
	HANDLE queue_semaphore;
}queue_t, * pqueue;

void initQueue(pqueue queuePtr) {
	queuePtr->head = 0;
	queuePtr->tail = -1;
	InitializeCriticalSection(&queuePtr->access);
	queuePtr->queue_semaphore = CreateSemaphore(NULL, 0, MAXINT, NULL);
}

void pushToQueue(pqueue queuePtr, queue_cell_t cell) {
	EnterCriticalSection(&queuePtr->access);
	queuePtr->arr[(++(queuePtr->tail)) % QUEUE_SIZE] = cell; // CHECK IF RIGHT
	LeaveCriticalSection(&queuePtr->access);
	ReleaseSemaphore(queuePtr->queue_semaphore, 1, NULL);

}

BYTE popFromQueue(pqueue queuePtr, pqueue_cell cellPtr, DWORD timeout) {
	if (WaitForSingleObject(queuePtr->queue_semaphore, timeout) == WAIT_OBJECT_0) {
		EnterCriticalSection(&queuePtr->access);
		cellPtr->lParam = queuePtr->arr[queuePtr->head].lParam;
		cellPtr->wParam = queuePtr->arr[queuePtr->head].wParam;
		cellPtr->type = queuePtr->arr[queuePtr->head].type;
		queuePtr->head = (queuePtr->head + 1) % QUEUE_SIZE; // inc head
		LeaveCriticalSection(&queuePtr->access);
		return 0; // true;

	}
	return 1; // false
}