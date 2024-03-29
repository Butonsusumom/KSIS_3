// WHOIS-TCP-CLIENT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <winsock.h>
#pragma comment (lib, "ws2_32.lib")

#define MY_PORT 43    //номер порта для whois - 43
#define SERVERADDR "127.0.0.1"     // локальный адрес сервера

struct domainInfoStruct  //структура с информацией по домену
{
	char* domainName;
	char* domainID;
	char* createdOn;
	char* registrarCountry;
	char* techOrg;
};

int main()
{
	domainInfoStruct domainInfo;
	char** fieldsPtr;         //указатель на поле структуры
	fieldsPtr = (char**)&domainInfo;
	char Buf[1024];      //буфер для получаемых сообщений
	int BufSize;       //размер строки в буфере
	int i;
	int errorCode;       //переменная для проверки на ошибки
	int serverSize;    //размер серверного адреса
	bool isRequest;     //был ли выполнен запрос
	WORD versionRequired;  //требуемая версия
	WSADATA wsaData;     //информация о спецификации сокета Windows
	SOCKET clientSocket;      //создаваемый сокет клиента
	SOCKADDR_IN saServer;      //серверный адрес сокета
	printf_s("%s\n", "WHOIS TCP CLIENT\n");
	//Инициализируем WinsockDll
	versionRequired = MAKEWORD(2, 2);
	errorCode = WSAStartup(versionRequired, &wsaData);
	if (errorCode == SOCKET_ERROR)
	{
		printf("Sorry, it's not available to find a usable Winsock DLL\n");
		return 1;
	}
	if ((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 2))
	{
		printf("Sorry, it's not available to find a usable Winsock DLL\n");
		return 1;
	}
	//Создаем сокет
	//AF-INET - сокет интернета
	//SOCK-STREAM - потоковый сокет
	//IPROTO-TCP - прокол TCP
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		printf_s("socket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//Получаем адрес сервера
	saServer.sin_family = AF_INET;
	saServer.sin_port = htons(MY_PORT);
	saServer.sin_addr.s_addr = inet_addr(SERVERADDR);
	serverSize = sizeof(saServer);
	//Устанавливаем соединение
	errorCode = connect(clientSocket, (sockaddr*)&saServer, serverSize);
	if (errorCode == SOCKET_ERROR)
	{
		printf_s("connect failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	printf_s("%s", "Connected succesfully.\n");
	//Соединение установлено - начинаем общение
	do
	{
		//отправляем серверу запрос на получение информации
		fgets(Buf, 1024, stdin);
		send(clientSocket, Buf, strlen(Buf), 0);
		//получаем ответ от сервера
		BufSize = recv(clientSocket, Buf, sizeof(Buf) - 1, 0);
		if (BufSize == SOCKET_ERROR)
		{
			printf_s("recv failed with error = %d\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		Buf[BufSize] = 0;
		//проверяем, принял ли сервер наш запрос
		if (strcmp(Buf, "OK") != 0)
			isRequest = false;
		else
			isRequest = true;
	} while (!isRequest);
	//получаем информацию от сервера
	BufSize = recv(clientSocket, Buf, sizeof(Buf) - 1, 0);
	if (BufSize == SOCKET_ERROR)
	{
		printf_s("recv failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	Buf[BufSize] = 0;
	if (strcmp(Buf, "Invalid domain") == 0)  //получаем сообщение об ошибке...
	{
		printf("%s\n", Buf);
	}
	else
	{
		//...либо информацию о домене
		//здесь в цикле разбиваем строку с пробелами на отдельные слова и помещаем их в поля структуры
		*fieldsPtr = Buf;
		fieldsPtr++;
		for (i = 0; i < BufSize; i++)
		{
			if (Buf[i] == ' ')
			{
				Buf[i] = 0;
				*fieldsPtr = &Buf[i + 1];
				fieldsPtr++;
			}
		}
		printf("Domain Name: %s\n", domainInfo.domainName);
		printf("Domain ID: %s\n", domainInfo.domainID);
		printf("Created on: %s\n", domainInfo.createdOn);
		printf("Registrar country: %s\n", domainInfo.registrarCountry);
		printf("Tech organization: %s\n", domainInfo.techOrg);
	}
	printf("%s\n", "Disconnected.");
	//Закрываем сокет и деинициализируем WinsockDll
	errorCode = closesocket(clientSocket);
	if (errorCode == SOCKET_ERROR)
	{
		printf("closesocket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();
	return 0;
}
