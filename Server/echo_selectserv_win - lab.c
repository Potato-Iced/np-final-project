#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>
#define BUF_SIZE 1024
void ErrorHandling(char *message);

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hServSock, hClntSock;
	SOCKADDR_IN		servAdr, clntAdr, clientaddr;
	int				adrSz;
	int				strLen, fdNum, i;
	char			buf[BUF_SIZE];
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

	while(1)
	{	
		// select() �Լ� ȣ��
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		fdNum = select(0, &cpyReads, 0, 0, &timeout );
		if (fdNum == SOCKET_ERROR) {
			closesocket(hClntSock);
			break; 
		}
		else if (fdNum == 0) {
			continue;
		}

		// signal�� �� ���¿��� signal �� �ڵ� Ȯ�� �۾� ���� LOOP ����.
		for (i = 0; i<reads.fd_count; i++) {

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
						printf("server> (port:%d, IP:%s),Msg : %s \n",
							clientaddr.sin_port, inet_ntoa(clientaddr.sin_addr), buf);
						send(reads.fd_array[i], buf, strLen, 0);    // echo!
					}
				}
			}
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