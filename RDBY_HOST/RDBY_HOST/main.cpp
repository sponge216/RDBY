
#include "main.h"

void end_function(SOCKET socket, const char* msg) {
	perror(msg);
	closesocket(socket);
}

void convertPacketToINPUT(char* buffer, INPUT* input, float ratios[])
{
	KEYBDINPUT* pKi = &input[0].ki;
	MOUSEINPUT* pMi = &input[0].mi;
	BYTE i = 0;

	input[0].type = 0;



	for (i; i < 4; i++) {
		input[0].type = (input[0].type << (sizeof(char) * 8)) | buffer[i];
	}

	if (input[0].type == 1) {
		//KEYBOARD
		/*pKi->dwExtraInfo = NULL;
		pKi->dwFlags = 0;
		pKi->time = 0;
		pKi->wScan = 0;
		pKi->wVk = 0;*/
		for (i; i < 8; i++) {
			pKi->wVk = (pKi->wVk << (sizeof(char) * 8)) | buffer[i];
		}

		for (i; i < 12; i++) {
			pKi->dwFlags = (pKi->dwFlags << (sizeof(char) * 8)) | buffer[i];
		}
		pKi->wScan = MapVirtualKeyA(pKi->wVk, MAPVK_VK_TO_VSC);
		fprintf(stdout, "%l %l %l\n",
			pKi->wScan,
			pKi->wVk,
			pKi->dwFlags);
	}
	else if (input[0].type == 0) {
		//MOUSE
		/*pMi->dwFlags = 0;
		pMi->dx = 0;
		pMi->dy = 0;
		pMi->mouseData = 0;
		pMi->time = 0;*/

		for (i; i < 8; i++) {
			pMi->dx = (pMi->dx << (sizeof(char) * 8)) | buffer[i];
		}
		for (i; i < 12; i++) {
			pMi->dy = (pMi->dy << (sizeof(char) * 8)) | buffer[i];
		}
		for (i; i < 16; i++) {
			pMi->dwFlags = (pMi->dwFlags << (sizeof(char) * 8)) | buffer[i];
		}
		for (i; i < 20; i++) {
			pMi->mouseData = (pMi->mouseData << (sizeof(char) * 8)) | buffer[i];
		}
		if (pMi->dx)
			pMi->dx = ((pMi->dx = (LONG)(pMi->dx * ratios[0] * ratios[2])) != 0) ? pMi->dx : 1;
		if (pMi->dy)
			pMi->dy = ((pMi->dy = (LONG)(pMi->dy * ratios[1] * ratios[2])) != 0) ? pMi->dy : 1;

	}

}


DWORD WINAPI client_handler(LPVOID data) {
	clientPtr client = (clientPtr)data;
	SOCKET socket = client->socket;
	free(client);

	INPUT input[1] = { 0 };
	KEYBDINPUT* pKi = &input[0].ki;
	MOUSEINPUT* pMi = &input[0].mi;
	char buffer[MAX_REQ_SIZE] = { 0 };

	int valread = 0;
	int client_metrics[3] = { 0 }; // 0 - x, 1 - y

	valread = recv(socket, (char*)client_metrics, 3 * sizeof(int), 0);

	int width = GetSystemMetrics(SM_CXSCREEN); 	//get primary screen's width and height.
	int height = GetSystemMetrics(SM_CYSCREEN);
	UINT dpi = GetDpiForSystem();
	fprintf(stdout, "%d %d %d\n", client_metrics[0], client_metrics[1], client_metrics[2]);

	float w_ratio = ((float)client_metrics[0] / width);
	float h_ratio = ((float)client_metrics[1] / height);
	float dpi_ratio = ((float)client_metrics[2] / dpi);
	float ratios[3] = { w_ratio,h_ratio,dpi_ratio };
	fprintf(stdout, "%f %f %f", w_ratio, h_ratio, dpi_ratio);

	while ((valread = recv(socket, buffer, MAX_REQ_SIZE, 0)) > 0) {
		convertPacketToINPUT(buffer, input, ratios);
		valread = SendInput(1, input, 40);

		/*fprintf(stdout, "%d %d %d %d\n", pMi->dx, pMi->dy, pMi->dwFlags, pMi->mouseData);*/

	}
	fprintf(stdout, "%s", buffer);
	end_function(socket, "RECV FAILED");
	fprintf(stdout, "%d\n", valread);
	ExitThread(1);
	return NULL;


}
int main(int argc, char** argv) {

	WSADATA wsa_data; // needed in order to use sockets in windows.

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) { // start the window socket application
		perror("SETUP FAILED"); exit(1);
	}

	SOCKET keys_input_socket; // socket for the server.
	struct addrinfo* server_addr = NULL, // the server's address info struct, that holds all info about the address.
		hints; // used to set the socket's behavior and address.

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // set internet family.
	hints.ai_socktype = SOCK_STREAM; //set socket type. We use TCP so we set it to sock_stream.
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int res = getaddrinfo(NULL, PORT, &hints, &server_addr);
	if (res != 0) {
		printf("getaddrinfo failed: %d\n", res);
		WSACleanup();
		exit(1);
	}
	if ((keys_input_socket = socket(server_addr->ai_family, server_addr->ai_socktype, server_addr->ai_protocol)) <= 1) {
		perror("SOCKET FAILED");
		exit(1);
	}
	if (keys_input_socket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(server_addr);
		WSACleanup();
		exit(1);
	}

	if (bind(keys_input_socket, server_addr->ai_addr, (int)server_addr->ai_addrlen) == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(server_addr);
		closesocket(keys_input_socket);
		WSACleanup();
		exit(1);
	}
	printf("all good!\n");
	if (listen(keys_input_socket, BACKLOG) < 0) {
		perror("LISTEN FAILED");
		exit(1);
	}
	while (1) {

		SOCKET client_socket;

		if ((client_socket = accept(keys_input_socket, NULL, NULL)) != INVALID_SOCKET) {

			clientPtr client = (clientPtr)malloc(sizeof(client_t));
			if (!client) { // validate malloc.
				perror("MALLOC FAILED!");
				continue;
			}
			fprintf(stdout, "NEW CLIENT!!!  :%llu\n", client_socket);
			client->socket = client_socket;
			if (!(CreateThread(NULL, 0, client_handler, (LPVOID)client, 0, NULL))) {
				fprintf(stdout, "THREAD FAILED\n");
			}

		}

	}
	return 0;
}