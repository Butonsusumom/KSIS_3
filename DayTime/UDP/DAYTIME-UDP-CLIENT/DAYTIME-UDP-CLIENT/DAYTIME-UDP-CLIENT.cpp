// DAYTIME-UDP-CLIENT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <winsock2.h>
#include <fstream>
#include <cstdlib>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")

int main()
{
	printf("DAYTIME UDP CLIENT\n");
	const int bufNum = 100; //размер буфера
	int iResult;
	WSAData wsaData;
	iResult = WSAStartup(MAKEWORD(2, 1), &wsaData); //настраиваем библиотеку
	if (iResult != NO_ERROR) {
		std::cout << "Error 1";
		return 1;
	}
	SOCKADDR_IN serverAdress;  //указываем IP-адрес и порт сервера
	int sizeaddr = sizeof(serverAdress);
	serverAdress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAdress.sin_port = htons(13);
	serverAdress.sin_family = AF_INET;

	SOCKET ClientSocket = socket(AF_INET, SOCK_DGRAM, NULL); //создаем сокет клиента
	if (ClientSocket == INVALID_SOCKET) {
		std::cout << "Error 2";
		WSACleanup();
		return 1;
	}

	char bufChar[bufNum];
	ZeroMemory(bufChar, bufNum);

	sendto(ClientSocket, "GetTime", 7, 0, (SOCKADDR*)&serverAdress, sizeaddr); //отправляем запрос на получение времени 

	recvfrom(ClientSocket, bufChar, bufNum, 0, (sockaddr*)&serverAdress, &sizeaddr);//получаем серверное время 

	std::cout << "Current time: " << bufChar << std::endl;
	iResult = closesocket(ClientSocket);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Error 3";
		return 1;
	}
	WSACleanup();
	getchar();
}
