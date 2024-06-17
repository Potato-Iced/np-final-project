#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#define BUF_SIZE 1024
#define DRONE_AMOUNT 3	// 드론 개수

typedef struct pos {	// 드론 정보를 담을 구조체
	int port;	// 식별용 포트
	int x;		// x 좌표
	int y;		// y 좌표
}POS;
void ErrorHandling(char *message);
POS* RoutePlanner(POS*);	// 드론 이동 경로 

int main(void)
{
	POS				droneList[DRONE_AMOUNT] = { 0, };
	WSADATA			wsaData;
	SOCKET			hServSock, hClntSock;
	SOCKADDR_IN		servAdr, clntAdr, clientaddr;
	int				adrSz;
	int				strLen, fdNum, i;
	char			buf[BUF_SIZE];
	char*			split;	// buf를 공백 기준으로 파싱해서 구분, strtok() 사용
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
		printf("M 키를 눌러 드론 이동 모드, Q키를 눌러 드론 정렬 및 종료\n");
		// select() 함수 호출
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		fdNum = select(0, &cpyReads, 0, 0, &timeout);
		if (fdNum == SOCKET_ERROR) {
			closesocket(hClntSock);
			break;
		}
		// else if(fdNum == 0) continue; // 이 부분 삭제, 아래로 변경
		else if (fdNum != 0){
			// signal이 온 상태에서 signal 온 핸들 확인 작업 수행 LOOP 생성.
			for (i = 0; i < reads.fd_count; i++) {
				// signal 온 핸들 확인.
				if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
					if (reads.fd_array[i] == hServSock) {
						// accept 실행...
						// client의 연결 요청 수신.
						adrSz = sizeof(clntAdr);
						printf("server> wait a client connection.\n");
						hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
						printf("server> connected client: Port:%d, IP:%s \n",
							clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

						FD_SET(hClntSock, &reads);
					}
					else {
						// data 수신.
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

							printf("드론 %d");
							printf("cnt : %d\n", (int)buf[0]);
							printf("command : %c\n", buf[2 * sizeof(int) + 1]);

							
						}
					}
				}
			}
		}
	
		// 드론 좌표 수신 후 사용자 키보드 입력을 받아 드론 이동명령 모드로 전환
		// M을 누르는 경우 드론 이동 모드
		// Q를 누르는 경우 드론 1차 정렬 -> 2차 이동 -> 종료 과정 수행

		if (GetAsyncKeyState(0x4D)) {	// 입력 키가 M인 경우

			continue;
		}
		if (GetAsyncKeyState(0x51)) {	// 입력 키가 Q인 경우
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

