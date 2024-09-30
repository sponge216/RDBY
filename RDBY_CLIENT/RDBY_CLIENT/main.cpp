#include "main.h"

HHOOK _kbd_hook; //keyboard hook
HHOOK _ms_hook; // mouse hook
KBDLLHOOKSTRUCT kbdStruct; // keyboard struct to hold info given from hook
MSLLHOOKSTRUCT msStruct; // mouse struct to hold info given from hook
SOCKET connectSocket;
LPPOINT first_point = (LPPOINT)malloc(sizeof(POINT));
BOOL pos_res = GetCursorPos(first_point);

pqueue proc_queue = (pqueue)malloc(sizeof(queue_t));
BYTE run = 1;

//Function that takes a KDLLHOOKSTRUCT pointer and fits it into an unsigned char buffer given by pointer
//Format: type (32) - virtual key code (32) - scan code (32) - flags (32)
void packetFormatKeyboard(KBDLLHOOKSTRUCT* pKbdStruct, unsigned char* buffer) {
	// TODO: REPLACE MODULU WITH A FASTER ALTERNATIVE.
	BYTE i = 0;
	for (i; i < 4; i++) {
		buffer[i] = (INPUT_KEYBOARD >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}
	for (i; i < 8; i++) {
		buffer[i] = (pKbdStruct->vkCode >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}
	for (i; i < 12; i++) {
		buffer[i] = (pKbdStruct->flags >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}


}
//Function that takes a MSLLHOOKSTRUCT pointer and fits it into an unsigned char buffer given by pointer)
//Format: type (32) - x (32) - y (32) - flags (32) - mouse data (32).
void packetFormatMouse(MSLLHOOKSTRUCT* pMsStruct, unsigned char* buffer, DWORD mouse_flags) {
	// TODO: REPLACE MODULU WITH A FASTER ALTERNATIVE.
	BYTE i = 0;

	for (i; i < 4; i++) {
		buffer[i] = (INPUT_MOUSE >> (sizeof(char) * 8 * (3 - i))) & 0xff;
	}
	for (i; i < 8; i++) {
		buffer[i] = (pMsStruct->pt.x >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}
	for (i; i < 12; i++) {
		buffer[i] = (pMsStruct->pt.y >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}
	for (i; i < 16; i++) {
		buffer[i] = (mouse_flags >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}
	for (i; i < 20; i++) {
		buffer[i] = (pMsStruct->mouseData >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}


}
//Releases keyboard hook.
void ReleaseKeyboardHook()
{
	UnhookWindowsHookEx(_kbd_hook);
}
//Releases mouse hook.
void ReleaseMouseHook()
{
	UnhookWindowsHookEx(_ms_hook);
}

DWORD WINAPI socket_output_thread(LPVOID data) {
	unsigned char buffer[PACKET_SIZE] = { 0 };
	queue_cell_t cell = { 0 };
	BYTE res = 0;
	DWORD timeout = 3;
	while (run) {
		while (res = popFromQueue(proc_queue, &cell, timeout)) {

		}
		if (cell.type == 0) {
			MSLLHOOKSTRUCT* pMsStruct = (MSLLHOOKSTRUCT*)(cell.lParam);

			LONG tempx = pMsStruct->pt.x;
			LONG tempy = pMsStruct->pt.y;
			pMsStruct->pt.x -= first_point->x;
			pMsStruct->pt.y -= first_point->y;
			first_point->x = tempx;
			first_point->y = tempy;

			DWORD flags = MOUSEEVENTF_MOVE; // default event is moving.
			//fprintf(stdout, "%x", wParam);
			switch (cell.wParam) {
			case WM_MOUSEMOVE:
				flags = MOUSEEVENTF_MOVE;
				//fprintf(stdout, "MOVE");
				break;
			case WM_LBUTTONDOWN:
				//fprintf(stdout, "LEFT DOWN!");
				flags = MOUSEEVENTF_LEFTDOWN;
				break;

			case WM_LBUTTONUP:
				//fprintf(stdout, "LEFT UP!");
				flags = MOUSEEVENTF_LEFTUP;
				break;

			case WM_MOUSEWHEEL:
				flags = MOUSEEVENTF_WHEEL;
				break;

			case WM_RBUTTONDOWN:
				//fprintf(stdout, "RIGHT DOWN!");
				flags = MOUSEEVENTF_RIGHTDOWN;
				break;

			case WM_RBUTTONUP:
				//fprintf(stdout, "RIGHT UP!");
				flags = MOUSEEVENTF_RIGHTUP;
				break;
			}


			packetFormatMouse(pMsStruct, buffer, flags);
			send(connectSocket, (char*)buffer, PACKET_SIZE, 0); // send keystroke.
		}
		else if (cell.type == 1) {
			KBDLLHOOKSTRUCT* pKbdStruct = (KBDLLHOOKSTRUCT*)(cell.lParam);
			packetFormatKeyboard(pKbdStruct, buffer);

			send(connectSocket, (char*)buffer, PACKET_SIZE - 2 * sizeof(DWORD), 0); // send keystroke.
		}
	}
	ExitThread(1);
	return NULL;
}
//Mouse callback function. Called when a keyboard input event is raised.
LRESULT __stdcall MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.
		queue_cell_t cell = { (void*)lParam,wParam,0 };
		pushToQueue(proc_queue, cell);
		unsigned char buffer[PACKET_SIZE] = { 0 };
		MSLLHOOKSTRUCT* pMsStruct = (MSLLHOOKSTRUCT*)(lParam);

		LONG tempx = pMsStruct->pt.x;
		LONG tempy = pMsStruct->pt.y;
		pMsStruct->pt.x -= first_point->x;
		pMsStruct->pt.y -= first_point->y;
		first_point->x = tempx;
		first_point->y = tempy;

		DWORD flags = MOUSEEVENTF_MOVE; // default event is moving.
		//fprintf(stdout, "%x", wParam);
		switch (wParam) {
		case WM_MOUSEMOVE:
			flags = MOUSEEVENTF_MOVE;
			//fprintf(stdout, "MOVE");
			break;
		case WM_LBUTTONDOWN:
			//fprintf(stdout, "LEFT DOWN!");
			flags = MOUSEEVENTF_LEFTDOWN;
			break;

		case WM_LBUTTONUP:
			//fprintf(stdout, "LEFT UP!");
			flags = MOUSEEVENTF_LEFTUP;
			break;

		case WM_MOUSEWHEEL:
			flags = MOUSEEVENTF_WHEEL;
			break;

		case WM_RBUTTONDOWN:
			//fprintf(stdout, "RIGHT DOWN!");
			flags = MOUSEEVENTF_RIGHTDOWN;
			break;

		case WM_RBUTTONUP:
			//fprintf(stdout, "RIGHT UP!");
			flags = MOUSEEVENTF_RIGHTUP;
			break;
		}


		packetFormatMouse(pMsStruct, buffer, flags);
		send(connectSocket, (char*)buffer, PACKET_SIZE, 0); // send keystroke.


	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_ms_hook, nCode, wParam, lParam);
}

//Keyboard callback function. Called when a keyboard input event is raised.
LRESULT __stdcall KeyboardHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		queue_cell_t cell = { (void*)lParam,wParam,0 };
		pushToQueue(proc_queue, cell);
		// the action is valid: HC_ACTION.
		//if (wParam == WM_KEYDOWN) {
		unsigned char buffer[PACKET_SIZE - 2 * sizeof(DWORD)] = { 0 };
		KBDLLHOOKSTRUCT* pKbdStruct = (KBDLLHOOKSTRUCT*)(lParam);
		packetFormatKeyboard(pKbdStruct, buffer);

		send(connectSocket, (char*)buffer, PACKET_SIZE - 2 * sizeof(DWORD), 0); // send keystroke.
		//}
	}

	// call the next hook in the hook chain. This is nessecary or your hook chain will break and the hook stops
	return CallNextHookEx(_kbd_hook, nCode, wParam, lParam);
}

void SetKeyboardHook()
{

	if (!(_kbd_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookCallback, NULL, 0)))
	{
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}
}

void SetMouseHook()
{

	if (!(_ms_hook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookCallback, NULL, 0)))
	{
		LPCWSTR a = L"Failed to install hook!";
		LPCWSTR b = L"Error";
		MessageBox(NULL, a, b, MB_ICONERROR);
	}
}




void firstInteraction() {
	//TODO: add checks to api calls.
	//send primary screen ratio
	//TODO: detect when screen changes.
	int valread = 0;
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	UINT dpi = GetDpiForSystem();
	fprintf(stdout, "%d %d %d\n", width, height, dpi);
	int client_metrics[3] = { width,height, dpi };
	valread = send(connectSocket, (char*)client_metrics, 3 * sizeof(int), 0);

}
int main(int argc, char** argv) {
	/*LONG test = -12345;
	LONG temp = 0;
	unsigned char buffer[4] = { 0 };
	for (int i = 0; i < 4; i++) {
		buffer[i] = (test >> (sizeof(char) * 8 * (3 - i % 4))) & 0xff;
	}
	for (int i = 0; i < 4; i++) {
		temp = (temp << (sizeof(char) * 8)) | buffer[i];
	}
	fprintf(stdout, "%d %d", test, temp);*/
	//----------------------
   // Initialize Winsock

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup function failed with error: %d\n", iResult);
		return 1;
	}
	//----------------------
		//Create a SOCKET for connecting to server

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.

	SOCKADDR_IN clientService;
	clientService.sin_family = AF_INET;
	inet_pton(AF_INET, SERVER_IP, &clientService.sin_addr.s_addr);
	clientService.sin_port = htons(SERVER_PORT);

	//----------------------
	// Connect to server.
	iResult = connect(connectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
		iResult = closesocket(connectSocket);
		if (iResult == SOCKET_ERROR)
			wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	initQueue(proc_queue); // init queue
	firstInteraction(); // exchange info
	SetKeyboardHook();
	SetMouseHook();

	if (!(CreateThread(NULL, 0, socket_output_thread, (LPVOID)0, 0, NULL))) {
		fprintf(stdout, "THREAD FAILED\n");
	}
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
	}
}