
#include "pch.h"
#include < iostream >
#pragma comment( lib, "ws2_32.lib" )
#include < Windows.h>
#include < conio.h>

#include <regex>
#include <time.h>

using namespace std;
#define PORT 7
#define SERVERADDR "127.0.0.1"

#define sendMessageArrayLen 5

char message[100];
char loginClient[100], loginServer[100], terminal[100];
char speed[100] = "1000";
char str_array[50][100];
int length_mes = 0;
int recivedMessageIndex = 0;

bool waitKey(const size_t secondsToWait)
{
	int i = secondsToWait * 10;
	for (; (i) && (!_kbhit()); i--)
	{
		Sleep(100);
		if (i % 10 == 0)
		{
			cout << i / 10 << endl;
		}
	}
	//очистка буфера клавиатуры
	while (_kbhit())
	{
		_getch();
	}
	return(i);
}

void recieveMessage(SOCKET ConnectSocket) {

	char buf[1024];
	int packages_recive = -1;


	for (;;)
	{
		memset(buf, 0, sizeof(buf));
		if (recv(ConnectSocket, buf, 1024, NULL))
		{
			++packages_recive;

			switch (recivedMessageIndex)
			{
			case 0: {}
			case 1: {}
			case 2: {}
			case 4: {
				printf(buf);
				break;
			}
			default:
				break;
			}


		}
		recivedMessageIndex = (recivedMessageIndex + 1) % sendMessageArrayLen;
	}

	packages_recive = 0;
	delete(buf);
}

void sendMessage(SOCKET ConnectSocket) // Клиент отправляет серверу четыре строки:  
{

	char buf[1024];
	char tempBuf[924];
	regex rx("getInfo");




	for (;;)
	{

		memset(buf, 0, sizeof(buf));
		buf[0] = '\\0';


		switch (recivedMessageIndex)
		{
		case 0: {
			cin >> loginClient;
			strcat_s(loginClient, "\\0");
			break; }
		case 1: {
			cin >> loginServer;
			strcat_s(loginServer, "\\0");
			break; }
		case 2: {
			memset(buf, 0, sizeof(buf));
			std::cin >> terminal;
			strcat_s(terminal, "/");

			strcat_s(buf, "\\0");
			strcat_s(buf, loginClient);
			strcat_s(buf, loginServer);
			strcat_s(buf, terminal);
			strcat_s(buf, speed);
			strcat_s(buf, "\\0");
			// expample (\0John\0John\0ibmpc3/9600\0)

			break;
		}
		case 3: {
			memset(buf, 0, sizeof(buf));
			strcpy_s(buf, "\\0");
			Sleep(10);
			break;
		}


		case 4: {
			std::cin >> message;
			strcpy_s(buf, message);
		}
		default:
			break;
		}

		//	cout << "buf = " << buf << endl;
		send(ConnectSocket, buf, (int)strlen(buf), NULL);
	}
}



void measureSpeed(HANDLE sendm, HANDLE recievem, SOCKET ConnectSoket) {


	WaitForSingleObject(recievem, INFINITE);
	WaitForSingleObject(sendm, INFINITE);


}


int main(int argc, char* argv[])
{

	printf("RLOGIN TCP CLIENT\n");
	char buff[1024];


	// Шаг 1 - инициализация библиотеки Winsock
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		printf("WSAStart error %d\n", WSAGetLastError());
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET my_sock;
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0)
	{
		printf("Socket() error %d\n", WSAGetLastError());
		return -1;
	}

	// Шаг 3 - установка соединения

	// заполнение структуры sockaddr_in
	// указание адреса и порта сервера
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst;

	// преобразование IP адреса из символьного в сетевой формат
	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
		// попытка получить IP адрес по доменному имени сервера
		if (hst = gethostbyname(SERVERADDR))
			// hst->h_addr_list массив указателей на адреса
			((unsigned long *)&dest_addr.sin_addr)[0] =
			((unsigned long **)hst->h_addr_list)[0][0];
		else
		{
			printf("error address %s\n", SERVERADDR);
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}

	// адрес сервера получен – пытаемся установить соединение 
	if (connect(my_sock, (sockaddr *)&dest_addr,
		sizeof(dest_addr)))
	{
		printf("connect error %d\n", WSAGetLastError());
		return -1;
	}
	cout << "Client conneted to the server \n";



	DWORD thID;

	// Шаг 4 - чтение и передача сообщений
	int nsize;

	HANDLE sendm;
	HANDLE recievem;

	sendm = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)sendMessage, (LPVOID)(my_sock), NULL, NULL);
	recievem = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)recieveMessage, (LPVOID)(my_sock), NULL, NULL);



	WaitForSingleObject(recievem, INFINITE);
	WaitForSingleObject(sendm, INFINITE);


	printf("Recv error %d\n", WSAGetLastError());
	closesocket(my_sock);
	WSACleanup();
	return -1;
}
