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
//POS* RoutePlanner(POS*);	// ��� �̵� ��� Ž����
void gotoxy(int x, int y);

void statusDraw();

POS				droneList[DRONE_AMOUNT] = { 0, };	// ���������� ����

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hServSock, hClntSock;
	SOCKADDR_IN		servAdr, clntAdr, clientaddr;
	int				adrSz;
	int				strLen, fdNum, i;
	unsigned char	buf[BUF_SIZE], datacnt;
//	char*			split;	// buf�� ���� �������� �Ľ��ؼ� ����, strtok() ��� -> ���ڿ� �ٷ�Ⱑ ���� ����
	int				addrlen;

	system("mode con lines=80 cols=120");	// ���� �ܼ� â ũ�� ����

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
		printf("M Ű�� ���� ��� �̵� ���, QŰ�� ���� ��� ���� �� ����..\n\n");

		// select() �Լ� ȣ��
		cpyReads = reads;
		timeout.tv_sec = 3;
		timeout.tv_usec = 3000;

		printf("server> select() - Ŭ���̾�Ʈ ������ ��ٸ��� ��..\n");
		fdNum = select(0, &cpyReads, 0, 0, &timeout);
		if (fdNum == SOCKET_ERROR) {
			closesocket(hClntSock);
			break;
		}
		// else if(fdNum == 0) continue; // �� �κ� ����, �Ʒ��� ����
		else if (fdNum != 0) {
			// signal�� �� ���¿��� signal �� �ڵ� Ȯ�� �۾� ���� LOOP ����.
			for (i = 0; i < reads.fd_count; i++) {
				// signal �� �ڵ� Ȯ��.
				if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
					if (reads.fd_array[i] == hServSock) {
						// accept ����...
						// client�� ���� ��û ����.
						adrSz = sizeof(clntAdr);
						printf("server> waiting for a client connection.\n");
						hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
						printf("server> connected client: Port:%d, IP:%s \n",
							clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

						FD_SET(hClntSock, &reads);
					}
					else {
						// data ����.
						// GUI ����
						system("cls");
						statusDraw();
						strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
						if (strLen <= 0)    // close request!
						{
							closesocket(reads.fd_array[i]);
							FD_CLR(reads.fd_array[i], &reads);
							printf("server> ��� %d ���� ����.. (IP:%s)\n",
								clientaddr.sin_port, inet_ntoa(clientaddr.sin_addr));

							// ��ϵ� ��� ���� ����� + ǥ�� �����
							droneList[i].port = 0;
							gotoxy(droneList[i].x / 4, droneList[i].y / 4);
							printf("   ");
							gotoxy(0, 52);
							break;
						}
						else
						{
							addrlen = sizeof(clientaddr);
							getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);

							//buf[strLen] = '\0';
							printf("server> ��� %d �κ��� �޼��� ���� (%d Bytes)\n",
								clientaddr.sin_port, strLen);
							send(reads.fd_array[i], buf, strLen, 0);    // echo!

							// ��� ���� ����ü�� ����
							// ��ɾ� ���� ��� ���� x��ǥ�� �ε����� [1], y��ǥ �ε����� [1 + 1*sizeof(int)] ����
							printf("%%%  ���� for�� i : %d  %%%\n", i);	// debug
							droneList[i].x = buf[1];
							droneList[i].y = buf[1 + 1 * sizeof(int)];
							droneList[i].port = clientaddr.sin_port;

							datacnt = buf[0];	// �Ǿ� ������ ����
							printf("[��� %d]\n", droneList[i].port);
							printf("$\tcnt : %d\n", datacnt);
							printf("$\t���� ��ǥ <%d, %d>\n", droneList[i].x, droneList[i].y);
							printf("$\tcommand : %c\n\n\n", buf[1 + (int)datacnt * sizeof(int)]);
						}
					}
				}
			}
		}
		// �ƹ� siganl�� ���� �ʾ��� ��
		else {
			printf("server> ����� Ŭ���̾�Ʈ ����\n");
		}

		// ��� ��ǥ ���� �� ����� Ű���� �Է��� �޾� ��� �̵���� ���� ��ȯ
		// M�� ������ ��� ��� �̵� ���
		// Q�� ������ ��� ��� 1�� ���� -> 2�� �̵� -> ���� ���� ����

		// ������ - ��׶���(�ܼ� â �ּ�ȭ ��) ��Ȳ������ Ű�� �ν��ع���
		// �ܼ� â �ּ�ȭ �ϰ��� ���ͳ��ϴٰ� m�̳� q�� ������ - �׷��� �νĵ�
		if (GetAsyncKeyState(0x4D)) {	// �Է� Ű�� M�� ���
			printf("MŰ �Է� ����\n");
			printf("[��� �̵� ���]\n");
			break;
		}
		if (GetAsyncKeyState(0x51)) {	// �Է� Ű�� Q�� ���
			printf("QŰ �Է� ����\n");
			printf("[��� ���� ���]\n");
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

/*
POS* RoutePlanner(POS* arr) {
	for (int i = 0; i < DRONE_AMOUNT; i++) {
		if (arr + i == NULL) {
			ErrorHandling("[ERROR] Out of Bounds\n");
		}

	}
}
*/

void gotoxy(int x, int y) {
	COORD pos = { x*2,y };	// �ֿܼ��� ���ĺ��� 0.5ũ��� 2�� ����
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}


void statusDraw() {
	// �ƹ����� ��ũ�� ũ��� 50x50�� �ִ��ε� �ϴ� -> ����� ���Ƿ� ���� ����, �ذ�(Line 35)
	// ��ǥ�� �� 180, 200 ������ϱ� 1ĭ �� ��ǥ 4�� ġ�� �ݿø�
	//		-> 0123 | 4567 | 891011 |
	//		->  0	|   1  |    2   |

	// �׸��� ��ǥ ������ �������� ��ȯ�ؾ���
	// 0,0 ���� �⺻���� ���� ��, ���� ��ǥ�� ������ ������ 0,20�̰� ������ �翷���� x������ -100 ~ 100, y ������ �Ʒ������� 20 ~ 200
	// x��ǥ�� 0�̻� 100�̸��� -100 ~ 0, 100 �̻���ʹ� 0 ~ 100����
	// y��ǥ�� 0 ~ 180 -> 200 ~ 20����
	printf("��200m�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n"); // 50��
	for (int i = 0; i <= DRONE_AMOUNT; i++) {
		if (droneList[i].port != 0) {
			gotoxy(droneList[i].x / 4, droneList[i].y / 4);
			printf("��");
		}
		else {
			// ���࿡ ���� ���������� ��� �����ִ� �ڵ�
			// �ƿ� ó�� ����� �Ÿ� x��ǥ�� 0�̹Ƿ� �̶��� continue�� ��ŵ
			if (droneList[i].x == 0) continue; 
			gotoxy(droneList[i].x / 4, droneList[i].y / 4);
			printf("  ");
		}
	}
	gotoxy(0,50);
	printf("��20m-�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n"); // 50��
	printf("                                                  �������\n\n");

}

