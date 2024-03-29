// FINGER-UDP-CLIENT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")

//Прекратить работу библиотеки Winsock.dll и выйти с кодом -1
#define fatal() { WSACleanup(); return -1; }  

#define PORT 79
#define SERVERADDR "127.0.0.1"
#define CRLF "\r\n"
#define ALL "\\all"
#define ECHO "foobar"

int main()
{
	printf("FINGER UDP CLIENT\n");
	int errorCode;
	WSADATA WSAData;

	//Шаг 1 - инициализация библиотеки Winsock.dll
	errorCode = WSAStartup(0x0202, (LPWSADATA)&WSAData);
	if (errorCode)
	{
		printf("Error during WSA startup: %d\n", errorCode);
		return -1;
	}

	//Шаг 2 - создание сокета сервера
	SOCKET clientSocket;

	//AF_INET - указание семейства адресов 
	//в данном случае - семейство адресов протокола IPv4
	//SOCK_DGRAM - тип сокета. В данном случае - датаграммы, т.к. UDP
	//0 - транспортный протокол для указанного семейства адресов - по умолчанию (UDP)
	clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Error during socket creation: %d\n", WSAGetLastError());
		fatal();
	}

	//Указываем IP адрес и порт сервера
	SOCKADDR_IN serverAdress;
	serverAdress.sin_family = AF_INET;
	serverAdress.sin_port = htons(PORT);
	serverAdress.sin_addr.s_addr = inet_addr(SERVERADDR);
	int addrSize = sizeof(serverAdress);


	char buff[1024]; //Буфер для отправки сообщений
	char exitMsg[5] = "EXIT";  //Команда для корректного закрытия (с освобождением системных ресурсов)


	sendto(clientSocket, CRLF, strlen(CRLF) + 1, 0, (sockaddr *)&serverAdress, sizeof(serverAdress));
	recvfrom(clientSocket, buff, 1024, 0, (sockaddr *)&serverAdress, &addrSize);
	printf(buff);


	scanf("%s", buff);
	if (strcmp(buff, ALL) == 0)
	{
		sendto(clientSocket, CRLF, strlen(CRLF) + 1, 0, (sockaddr *)&serverAdress, sizeof(serverAdress));
	}
	else
	{
		strcat(buff, CRLF);
		sendto(clientSocket, buff, strlen(buff) + 1, 0, (sockaddr *)&serverAdress, sizeof(serverAdress));
	}

	recvfrom(clientSocket, buff, 1024, 0, (sockaddr *)&serverAdress, &addrSize);
	while (strcmp(buff, exitMsg) != 0)
	{
		printf(buff);
		sendto(clientSocket, ECHO, sizeof(ECHO), 0, (sockaddr *)&serverAdress, addrSize);
		recvfrom(clientSocket, buff, 1024, 0, (sockaddr *)&serverAdress, &addrSize);
	}

	printf("Server closed the connection. Press any key to exit.\n", WSAGetLastError());
	system("pause");
	closesocket(clientSocket);
	WSACleanup();
	return 1;
}
