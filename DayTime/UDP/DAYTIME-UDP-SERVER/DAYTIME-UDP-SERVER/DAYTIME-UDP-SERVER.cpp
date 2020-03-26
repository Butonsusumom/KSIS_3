// DAYTIME-UDP-SERVER.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <ctime>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
int main()
{
	printf("DAYTIME UDP SERVER\n");
	const int bufNum = 100; //размер буфера
	int iResult;
	WSAData wsaData;
	iResult = WSAStartup(MAKEWORD(2, 1), &wsaData);//настраиваем библиотеку
	if (iResult != NO_ERROR) {
		std::cout << "Error 1";
		return 1;
	}
	SOCKADDR_IN localAdress; //занимаем все IP-адреса и порт 
	int sizeaddr = sizeof(localAdress);
	localAdress.sin_addr.s_addr = INADDR_ANY;
	localAdress.sin_port = htons(13);
	localAdress.sin_family = AF_INET;

	SOCKET ServerSocket = socket(AF_INET, SOCK_DGRAM, NULL); //создаем сокет сервера
	if (ServerSocket == INVALID_SOCKET) {
		std::cout << "Error 2";
		WSACleanup();
		return 1;
	}

	iResult = bind(ServerSocket, (SOCKADDR*)&localAdress, sizeaddr); // связываем сокет с локальным адресом
	if (iResult == SOCKET_ERROR) {
		std::cout << "Error 3";
		iResult = closesocket(ServerSocket);
		if (iResult == SOCKET_ERROR) {
			std::cout << "Error 4";
		}
		WSACleanup();
		return 1;
	}
	SOCKADDR_IN ClientAdress;
	int ClientAdresssize = sizeof(ClientAdress);


	while (1) {
		char bufchar[bufNum];
		recvfrom(ServerSocket, bufchar, bufNum, 0, (SOCKADDR*)&ClientAdress, &ClientAdresssize);
		time_t curTime = time(NULL);
		tm* curTimeST;
		ZeroMemory(bufchar, bufNum);
		curTimeST = localtime(&curTime);
		strftime(bufchar, bufNum, "%A, %d %B %Y %H:%M:%S", curTimeST); //получаем системное время
		sendto(ServerSocket, bufchar, bufNum, 0, (SOCKADDR*)&ClientAdress, ClientAdresssize); //отправляем его клиенту
		std::cout << "Time sent by server: " << bufchar << std::endl;
	}
	iResult = closesocket(ServerSocket);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Error 5";
		return 1;
	}
	WSACleanup();
}