// FINGER-UDP-SERVER.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <fstream>
#include <string.h>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
#define CRLF "\r\n"
#define HELLO_STR "Connection established"
#define LOGIN_STR "User login: "
#define NAME_STR "User name: "
#define GROUP_STR "User group: "
#define PHONE_STR "User phone: "
#define USERINFO_STR "User information file:\r\n"
#define	NOUSERINFO_STR "User don't have an information file\r\n"
#define USERNOTFOUND_STR "There is no such user\r\n"
#define ECHO "foobar"

#define FINGER_PORT    79 // Порт, который слушает сервер


// прототип функции, обслуживающий
// подключившихся пользователей
DWORD WINAPI WorkWithClient(LPVOID client_socket_addr);


// глобальная переменная – количество
// активных пользователей 
int nclients = 0;

int main(int argc, char* argv[])
{
	char buff[1024];    // Буфер для различных нужд

	printf("FINGER UDP SERVER\n");

	// Шаг 1 - Инициализация Библиотеки Сокетов
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// Ошибка!
		printf("Error during WSAStartup %d\n",
			WSAGetLastError());
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET mysocket;
	// AF_INET     - сокет Интернета
	// SOCK_DGRAM  - датаграммный сокет
	// 0      - по умолчанию выбирается UDP протокол
	if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		// Ошибка!
		printf("Error during socket creation: %d\n", WSAGetLastError());
		WSACleanup();
		// Деиницилизация библиотеки Winsock
		return -1;
	}

	// Шаг 3 связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(FINGER_PORT);
	local_addr.sin_addr.s_addr = 0;
	// сервер принимает подключения
	// на все IP-адреса

	// вызываем bind для связывания
	if (bind(mysocket, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		// Ошибка
		printf("Binding error: %d\n", WSAGetLastError());
		closesocket(mysocket);  // закрываем сокет!
		WSACleanup();
		return -1;
	}

	printf("Server started successfully. Waiting for clients...\n");

	SOCKADDR_IN clientAdress;
	int addrSize = sizeof(clientAdress);
	char exitMsg[5] = "EXIT";

	while (recvfrom(mysocket, buff, 1024, 0, (sockaddr *)&clientAdress, &addrSize))
	{
		// отправляем клиенту приветствие 
		sendto(mysocket, HELLO_STR, sizeof(HELLO_STR), 0, (sockaddr *)&clientAdress, addrSize);

		//Принимаем запрос от клиента и отправляем соответствующую информацию в ответ
		int bytes_recv;
		recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);
		//Открыть файл с информацией о пользователях
		ifstream users;
		users.open("USERS.txt", ios::in);

		//Если Запрос вида \r\n, то по стандарту возвращается информация о всех пользователях 
		if (strcmp(CRLF, buff) == 0)
		{
			while (users.getline(buff, sizeof(buff)))
			{
				strcat(buff, CRLF);
				sendto(mysocket, buff, strlen(buff) + 1, 0, (sockaddr *)&clientAdress, addrSize);
				recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);
			}
		}
		else //Любой другой запрос расценивается как запрос о конкретном пользователе (по логину или по имени)
		{
			bool userFound = false;
			string tmp, userLogin, userName;
			char userInfo[256], tmpStr[128];
			//От запроса откидываются последние два символа, которые всегда \r\n по требованию стандарта
			buff[strlen(buff) - 2] = '\0';

			//Обходим весь файл с пользователями
			while (users.getline(userInfo, sizeof(userInfo)))
			{
				userLogin = strtok(userInfo, " _");
				userName = strtok(NULL, " _");
				//Если запрос совпал с именем либо с логином пользователя, то отправляем информацию о нём
				if (strcmp(buff, userLogin.c_str()) == 0 || strcmp(buff, userName.c_str()) == 0)
				{
					//Устанавливаем флаг что есть совпадение
					userFound = true;

					//Отправляем строку с логином в удобочитаемом виде
					tmp = LOGIN_STR + userLogin + "\r\n";
					strcpy(tmpStr, tmp.c_str());
					sendto(mysocket, tmpStr, strlen(tmpStr) + 1, 0, (sockaddr *)&clientAdress, addrSize);
					recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);

					//Отправляем строку с именем и фамилией в удобочитаемом виде 
					tmp = NAME_STR + userName + " " + strtok(NULL, " ") + "\r\n";
					strcpy(tmpStr, tmp.c_str());
					sendto(mysocket, tmpStr, strlen(tmpStr) + 1, 0, (sockaddr *)&clientAdress, addrSize);
					recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);

					//Отправляем строку с номером группы в удобочитаемом виде
					//тут костыльно, потому что конкатенация плюсовых строк почему-то ругается,
					//если в выражении только сишные строки, поэтому тут ещё и tmp="" засунуто
					tmp = "";
					tmp = tmp + GROUP_STR + strtok(NULL, " ") + "\r\n";
					strcpy(tmpStr, tmp.c_str());
					sendto(mysocket, tmpStr, strlen(tmpStr) + 1, 0, (sockaddr *)&clientAdress, addrSize);
					recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);

					//Отправляем строку с номером. Тут такой же костыль
					tmp = "";
					tmp = tmp + PHONE_STR + strtok(NULL, " ") + "\r\n";
					strcpy(tmpStr, tmp.c_str());
					sendto(mysocket, tmpStr, strlen(tmpStr) + 1, 0, (sockaddr *)&clientAdress, addrSize);
					recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);

					//У каждого пользователя может быть свой информационный файл.
					//Называться он должен логин_пользователя.txt
					//Проверяем существует ли он и если да, то перекидываем построчно клиенту
					//если нет, то отправляем соответствующее сообщение
					ifstream userInfoFile;
					userInfoFile.open(userLogin + ".txt", ios::in);
					if (userInfoFile.is_open())
					{
						sendto(mysocket, USERINFO_STR, sizeof(USERINFO_STR), 0, (sockaddr *)&clientAdress, addrSize);
						recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);

						while (userInfoFile.getline(buff, sizeof(buff)))
						{
							strcat(buff, CRLF);
							sendto(mysocket, buff, strlen(buff) + 1, 0, (sockaddr *)&clientAdress, addrSize);
							recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);
						}

					}
					else
					{
						sendto(mysocket, NOUSERINFO_STR, sizeof(NOUSERINFO_STR), 0, (sockaddr *)&clientAdress, addrSize);
						recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);
					}
				}
			}

			//Если совпадений не найдено, то отправляем соответствующее сообщение
			if (!userFound)
			{
				sendto(mysocket, USERNOTFOUND_STR, sizeof(USERNOTFOUND_STR), 0, (sockaddr *)&clientAdress, addrSize);
				recvfrom(mysocket, buff, sizeof(buff), 0, (sockaddr *)&clientAdress, &addrSize);
			}
		}


		sendto(mysocket, exitMsg, strlen(exitMsg) + 1, 0, (sockaddr *)&clientAdress, addrSize);

	}
	return 0;
}
