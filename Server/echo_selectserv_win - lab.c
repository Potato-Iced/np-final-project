#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#define BUF_SIZE 1024
#define DRONE_AMOUNT 3	// ��� ����

typedef struct pos {	// ��� ������ ���� ����ü
	int port;	// �ĺ��� ��Ʈ
	int x;		// x ��ǥ
	int y;		// y ��ǥ
}POS;
void ErrorHandling(char *message);
POS* RoutePlanner(POS*);	// ��� �̵� ��� 

int main(void)
{
	POS				droneList[DRONE_AMOUNT] = { 0, };
	WSADATA			wsaData;
	SOCKET			hServSock, hClntSock;
	SOCKADDR_IN		servAdr, clntAdr, clientaddr;
	int				adrSz;
	int				strLen, fdNum, i;
	char			buf[BUF_SIZE];
	char*			split;	// buf�� ���� �������� �Ľ��ؼ� ����, strtok() ���
	int				addrlen;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAdr.sin_port		= htons(9000);
	
	if(bind(hServSock, (SOCKADDR*) &servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	fd_set  reads, cpyReads;
	TIMEVAL  timeout;
	FD_ZERO(&reads);
	FD_SET(hServSock, &reads);

	while (1)
	{
		printf("M Ű�� ���� ��� �̵� ���, QŰ�� ���� ��� ���� �� ����\n");
		// select() �Լ� ȣ��
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		fdNum = select(0, &cpyReads, 0, 0, &timeout);
		if (fdNum == SOCKET_ERROR) {
			closesocket(hClntSock);
			break;
		}
		// else if(fdNum == 0) continue; // �� �κ� ����, �Ʒ��� ����
		else if (fdNum != 0){
			// signal�� �� ���¿��� signal �� �ڵ� Ȯ�� �۾� ���� LOOP ����.
			for (i = 0; i < reads.fd_count; i++) {
				// signal �� �ڵ� Ȯ��.
				if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
					if (reads.fd_array[i] == hServSock) {
						// accept ����...
						// client�� ���� ��û ����.
						adrSz = sizeof(clntAdr);
						printf("server> wait a client connection.\n");
						hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
						printf("server> connected client: Port:%d, IP:%s \n",
							clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

						FD_SET(hClntSock, &reads);
					}
					else {
						// data ����.
						strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
						if (strLen <= 0)    // close request!
						{
							closesocket(reads.fd_array[i]);
							FD_CLR(reads.fd_array[i], &reads);
							printf("server> close connection with (port:%d, IP:%s)\n",
								clientaddr.sin_port, inet_ntoa(clientaddr.sin_addr));
							break;
						}
						else
						{
							addrlen = sizeof(clientaddr);
							getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);

							buf[strLen] = '\0';
							printf("server> (port:%d, IP:%s), Msg : %s \n",
								clientaddr.sin_port, inet_ntoa(clientaddr.sin_addr), buf);
							send(reads.fd_array[i], buf, strLen, 0);    // echo!
							
							droneList[i].x = *(int*)(buf + (1 + 0 * sizeof(int)));
							droneList[i].y = *(int*)(buf + (1 + 1 * sizeof(int)));
							droneList[i].port = clientaddr.sin_port;

							printf("��� %d");
							printf("cnt : %d\n", (int)buf[0]);
							printf("command : %c\n", buf[2 * sizeof(int) + 1]);

							
						}
					}
				}
			}
		}
	
		// ��� ��ǥ ���� �� ����� Ű���� �Է��� �޾� ��� �̵���� ���� ��ȯ
		// M�� ������ ��� ��� �̵� ���
		// Q�� ������ ��� ��� 1�� ���� -> 2�� �̵� -> ���� ���� ����

		if (GetAsyncKeyState(0x4D)) {	// �Է� Ű�� M�� ���

			continue;
		}
		if (GetAsyncKeyState(0x51)) {	// �Է� Ű�� Q�� ���
			break;
		}
	}
	
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

POS* RoutePlanner(POS* arr) {
	for (int i = 0; i < DRONE_AMOUNT; i++) {
		if (arr + i == NULL) {
			ErrorHandling("[ERROR] Out of Bounds\n");
		}

	}
}

