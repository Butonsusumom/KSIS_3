// WHOIS-UDP-SERVER.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <winsock.h>
#pragma comment( lib, "ws2_32.lib" )

#define MY_PORT 43

int main()
{
	char Buf[1024];       //буфер для получаемых сообщений
	char request[6];    //переменная для проверки, был ли выполнен запрос "whois"
	char domainName[20];   //переменная для хранения доменного имени
	char bufName[20];
	int errorCode;       //переменная для проверки на ошибки
	int BufSize;        //размер строки в буфере
	int clientSize;        //размеры клиентского и локального адресов
	int localSize;
	int i;
	int j;
	int length;
	int fileSize;    //размер файла в байтах
	int pos;       //позиция файлового указателя
	int maxArraySize;      //максимальный размер массива строк из файла
	bool isRequest;       //был ли выполнен запрос
	bool isSend;         //была ли выполнена отправка
	WORD versionRequired;  //требуемая версия
	WSADATA wsaData;     //информация о спецификации сокета Windows
	SOCKET serverSocket;   //создаваемый сокет сервера
	SOCKADDR_IN saClient, saLocal;  //локальный и клиентский адреса сокетов
	HOSTENT* hostent;       //информация о хосте
	FILE* domainFile;        //информация о доменах
	char domainInfo[20][200];     //массив строк с информацией о доменах
	printf_s("%s", "WHOIS UDP SERVER\n");
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
	//SOCK-DGRAM - дейтаграммный сокет, не ориентированный на соединение
	//IPROTO-UDP - прокол UDP
	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET)
	{
		printf("socket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	//Связываем сокет с локальным адресом
	saLocal.sin_family = AF_INET;
	saLocal.sin_port = htons(MY_PORT);
	saLocal.sin_addr.s_addr = INADDR_ANY;  //сервер принимает подключения со всех адресов
	localSize = sizeof(saLocal);
	errorCode = bind(serverSocket, (sockaddr*)&saLocal, sizeof(saLocal));
	if (serverSocket == INVALID_SOCKET)
	{
		printf("bind failed with error = %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	clientSize = sizeof(saClient);
	hostent = gethostbyaddr((char*)&saClient.sin_addr, 4, AF_INET); //получаем информацию о подключенном клиенте и выводи на экран
	printf_s("Accepted connection from %s, port %d\n", inet_ntoa(saClient.sin_addr), ntohs(saClient.sin_port));
	do
	{
		BufSize = recvfrom(serverSocket, &Buf[0], sizeof(Buf) - 1, 0, (sockaddr*)&saClient, &clientSize); //получаем сообщение
		if (BufSize == SOCKET_ERROR)
		{
			printf("recvfrom failed with error = %d\n", WSAGetLastError());
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}
		Buf[BufSize - 1] = 0; //добавляем завершающий ноль
		printf_s("%s\n", Buf);
		//проверяем на то, являются ли первые 5 символов словом "whois" (т.е. запросом)
		if (strlen(Buf) < 5)
		{
			isRequest = false;
			sendto(serverSocket, "NotOK", strlen("NotOK"), 0, (sockaddr*)&saClient, sizeof(saClient));
		}
		else
		{
			for (i = 0; i < 5; i++)
			{
				request[i] = Buf[i];
			}
			request[5] = 0;
			if (strcmp(request, "whois") != 0)
			{
				sendto(serverSocket, "NotOK", strlen("NotOK"), 0, (sockaddr*)&saClient, sizeof(saClient));
				isRequest = false;
			}
			else
			{
				sendto(serverSocket, "OK", strlen("OK"), 0, (sockaddr*)&saClient, sizeof(saClient));
				isRequest = true;
			}
		}
	} while (!isRequest);
	//получаем доменное имя
	for (i = 6; i < strlen(Buf); i++)
		domainName[i - 6] = Buf[i];
	domainName[strlen(Buf) - 6] = 0;
	//открываем файл доменных имен и получаем его размер
	domainFile = fopen("domaintable.txt", "rt");
	i = 0;
	pos = ftell(domainFile);
	fseek(domainFile, 0L, SEEK_END);
	length = ftell(domainFile);
	fseek(domainFile, pos, SEEK_SET);
	fileSize = length;
	if (fileSize != 0)
		while (!feof(domainFile))
		{
			fgets(domainInfo[i], 200, domainFile);
			i++;
		}
	maxArraySize = i;
	isSend = false;
	fclose(domainFile);
	//проходим по массиву строк и проверяем, является ли строка информацией по домену
	for (i = 0; i < maxArraySize; i++)
	{
		for (j = 0; domainInfo[i][j] != ' '; j++);
		strncpy(bufName, domainInfo[i], j);
		bufName[j] = 0;
		if (strcmp(bufName, domainName) == 0)
		{
			sendto(serverSocket, domainInfo[i], strlen(domainInfo[i]), 0, (sockaddr*)&saClient, sizeof(saClient));
			isSend = true;
		}
	}
	//если мы в итоге не отправили никакой информации, то отправляем сообщение о некорректности домена
	if (!isSend)
		sendto(serverSocket, "Invalid domain", strlen("Invalid domain"), 0, (sockaddr*)&saClient, sizeof(saClient));
	printf_s("%s\n", "Disconnected.");
	//Закрываем сокет и деинициализируем WinsockDll
	errorCode = closesocket(serverSocket);
	if (errorCode == SOCKET_ERROR)
	{
		printf("closesocket failed with error = %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	WSACleanup();
	return 0;
}