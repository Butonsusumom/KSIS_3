
#include "pch.h"
#include < iostream >
#pragma comment( lib, "ws2_32.lib" )
#include < Windows.h >
#include < conio.h >

#include <ctime>
#include <regex>
#include <cstdlib>
using namespace std;

#define MY_PORT    7 // Порт, который слушает сервер

// заведём глобальную переменную  ниже для обмена данным между нитями сервера   
char cbuff[] = "common buffer"; // общий буфер для записи сообщений от всех клиентов

char sendMessageArray[100][100] =
{ {"Login Client : "}, {"Login Server : "}, { "Terminal : " }, {"\\0"}, {"Password : "} };
string login = "root";  string password = "admin";

#define sendMessageArrayLen 5


// макрос для печати количества активных
// пользователей 
#define PRINTNUSERS if (nclients)\
  printf("%d user on-line\n",nclients);\
  else printf("No User on line\n");

							   // прототип функции, обслуживающий
							   // подключившихся пользователей
DWORD WINAPI WorkWithClient(LPVOID client_socket);

//будет просто циклически проверять знкачение глобальной переменной
DWORD WINAPI CheckCommonBuffer(LPVOID client_socket);

// глобальная переменная – количество
// активных пользователей 
int nclients = 0;




int main(int argc, char* argv[])
{
	srand(0);
	char buff[1024];    // Буфер для различных нужд

	printf("RLOGIN TCP SERVER\n");

	// Шаг 1 - Инициализация Библиотеки Сокетов
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// Ошибка!
		printf("error-WSAStartup %d\n",
			WSAGetLastError());
		return -1;
	}

	// Шаг 2 - создание сокета
	SOCKET mysocket;
	// AF_INET     - сокет Интернета
	// SOCK_STREAM  - потоковый сокет (с установкой соединения)
	// 0      - по умолчанию выбирается TCP протокол
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		// Ошибка!
		printf("error-socket %d\n", WSAGetLastError());
		WSACleanup();
		// Деиницилизация библиотеки Winsock
		return -1;
	}

	// Шаг 3 связывание сокета с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT);
	// не забываем о сетевом порядке!!!
	local_addr.sin_addr.s_addr = 0;
	// сервер принимает подключения
	// на все IP-адреса

	// вызываем bind для связывания
	if (bind(mysocket, (sockaddr *)&local_addr,
		sizeof(local_addr)))
	{
		// Ошибка
		printf("error-bind %d\n", WSAGetLastError());
		closesocket(mysocket);  // закрываем сокет!
		WSACleanup();
		return -1;
	}

	// Шаг 4 ожидание подключений
	// размер очереди – 0x100
	if (listen(mysocket, 20))
	{
		// Ошибка
		printf("error-listen %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return -1;
	}

	printf("waiting connect\n");

	// Шаг 5 извлекаем сообщение из очереди
	SOCKET client_socket[20];    // сокет для клиента
	sockaddr_in client_addr;    // адрес клиента
								// (заполняется системой)

								// функции accept необходимо передать размер
								// структуры
	int client_addr_size = sizeof(client_addr);

	// цикл извлечения запросов на подключение из
	// очереди
	/* accept - держит управление и не даёт циклу вращаться
	(то есть не даёт потоку- нити выполняться вообще)
	пока не поступит очередной запрос на соединение*/
	while ((client_socket[nclients] = accept(mysocket, (sockaddr *)
		&client_addr, &client_addr_size)))
	{
		nclients++;      // увеличиваем счетчик
						 // подключившихся клиентов

						 // пытаемся получить имя хоста
		HOSTENT *hst;
		hst = gethostbyaddr((char *)
			&client_addr.sin_addr.s_addr, 4, AF_INET);

		// вывод сведений о клиенте
		printf("+%s [%s] new connect!\n", (hst) ? hst->h_name : "", inet_ntoa(client_addr.sin_addr));
		PRINTNUSERS

			// Вызов нового потока для обслужвания клиента
			// Да, для этого рекомендуется использовать
			// _beginthreadex но, поскольку никаких вызов
			// функций стандартной Си библиотеки поток не
			// делает, можно обойтись и CreateThread
			DWORD thID;

		CreateThread(NULL, NULL, WorkWithClient, client_socket, NULL, &thID);

		/* запускаем функции прослушивания чужих сообщений
		нам нужен именно отдельный поток, потому что recv()
		забирает управление и не позволит послать клиенту
		сообщение пока от этого самого ("родного" для данной
		нити сервера) клиента что-нибудь не придёт */
		//CreateThread(NULL,NULL, CheckCommonBuffer, &client_socket,NULL,&thID);
	}
	return 0;
}



// Эта функция создается в отдельном потоке и
// обсуживает очередного подключившегося клиента
// независимо от остальных
DWORD WINAPI WorkWithClient(LPVOID sockets) // Ksusha one love progrram is top )))))))))))))))))
{
	int pokets_amount = 2;
	SOCKET my_sock;
	my_sock = ((SOCKET *)sockets)[nclients - 1];
	char buff[1024] = "0";
	//  char buff2[20*1024];
#define sHELLO "Put message...\r\n"
	//printf(" \n new thread is started connect!\n");

	// отправляем клиенту приветствие 
	int sendMessageIndex = 0;
	int index = sendMessageIndex++ % sendMessageArrayLen;

	send(my_sock, sendMessageArray[index], sizeof(sHELLO), 0);
	cout << "socket: " << my_sock << " -send: " << &sendMessageArray[index][0] << std::endl;



	// цикл эхо-сервера: прием строки от клиента и
	int bytes_recv;
	char sendMessage[] = "\\0";
	string put_log, put_password;
	// возвращение ее клиенту

	while (1)
	{

		if ((bytes_recv = recv(my_sock, buff, sizeof(buff), 0)) != SOCKET_ERROR) {

			cout << "socket: " << my_sock << " -recived: " << &buff[0] << std::endl;

			char message[100];

			index = sendMessageIndex;
			strcpy_s(message, sendMessageArray[index]);


			if (sendMessageIndex == 3) {

				int i = 2;
				string buf = string(buff);
				while (buff[i] != '0' && buff[i - 1] != '\\') i++;
				put_log = buf.substr(2, i - 3);


			}


			if (sendMessageIndex == 0) {

				put_password = string(buff);
				strcpy_s(message, "");

				if (put_log != login) {
					index = sendMessageIndex;
					strcat_s(message, "Incorrent Login ");



					if (put_password != password) {
						index = sendMessageIndex;
						strcat_s(message, "Incorrent Password ");
					}
				}

				strcat_s(message, sendMessageArray[index]);

			}

			cout << "socket: " << my_sock << " -send: " << &message[0] << std::endl;




			bytes_recv = sizeof(message);


			memset(buff, 0, sizeof(buff));

			for (int i = 0; i < nclients; i++)
			{
				SOCKET temp;
				temp = ((SOCKET*)sockets)[i];

				send(temp, &message[0], sizeof(message), 0);

			}
			// печатает сообщения всех клиентов в консоли сервера
			if (++sendMessageIndex == sendMessageArrayLen) sendMessageIndex = 0;
		}
		else {

			break;
		}

	}

	// если мы здесь, то произошел выход из цикла по
	// причине возращения функцией recv ошибки –
	// соединение клиентом разорвано
	nclients--; // уменьшаем счетчик активных клиентов
	printf("-disconnect\n"); PRINTNUSERS

		// закрываем сокет
		closesocket(my_sock);
	return 0;
}

