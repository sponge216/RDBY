#include "main.h"

HHOOK _kbd_hook; //keyboard hook
HHOOK _ms_hook; // mouse hook
KBDLLHOOKSTRUCT kbdStruct; // keyboard struct to hold info given from hook
MSLLHOOKSTRUCT msStruct; // mouse struct to hold info given from hook
SOCKET connectSocket;
LPPOINT first_point = (LPPOINT)malloc(sizeof(POINT));
BOOL pos_res = GetCursorPos(first_point);


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

	fprintf(stdout, "%d %d %d %d\n",
		buffer,
		buffer + sizeof(DWORD),
		buffer + 2 * sizeof(DWORD),
		buffer + 3 * sizeof(DWORD));
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

//Mouse callback function. Called when a keyboard input event is raised.
LRESULT __stdcall MouseHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode >= 0)
	{
		// the action is valid: HC_ACTION.

		unsigned char buffer[PACKET_SIZE] = { 0 };
		MSLLHOOKSTRUCT* pMsStruct = (MSLLHOOKSTRUCT*)(lParam);
		UINT msg_id = HIWORD(wParam);
		DWORD flags = MOUSEEVENTF_MOVE; // default event is moving.
		switch (msg_id) {

		case WM_LBUTTONDOWN:
			flags = MOUSEEVENTF_LEFTDOWN;
			break;

		case WM_LBUTTONUP:
			flags = MOUSEEVENTF_LEFTUP;
			break;

		case WM_MOUSEWHEEL:
			flags = MOUSEEVENTF_WHEEL;
			break;

		case WM_RBUTTONDOWN:
			flags = MOUSEEVENTF_RIGHTDOWN;
			break;

		case WM_RBUTTONUP:
			flags = MOUSEEVENTF_RIGHTUP;
			break;
		}
		LONG tempx = pMsStruct->pt.x;
		LONG tempy = pMsStruct->pt.y;
		pMsStruct->pt.x -= first_point->x;
		pMsStruct->pt.y -= first_point->y;
		first_point->x = tempx;
		first_point->y = tempy;

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
		// the action is valid: HC_ACTION.
		if (wParam == WM_KEYDOWN) {
			unsigned char buffer[PACKET_SIZE - 2 * sizeof(DWORD)] = { 0 };
			KBDLLHOOKSTRUCT* pKbdStruct = (KBDLLHOOKSTRUCT*)(lParam);
			packetFormatKeyboard(pKbdStruct, buffer);

			send(connectSocket, (char*)buffer, PACKET_SIZE - 2 * sizeof(DWORD), 0); // send keystroke.
		}
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


DWORD WINAPI socket_output_thread(LPVOID data) {
	ExitThread(1);
	return NULL;
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