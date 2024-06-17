#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define	BUF_SIZE	512
#define A_LEN		2
#define R_LEN		2
#define M_LEN		3

void ErrorDisplay( char *str )
{
	printf("<ERROR> %s!!!\n", str );
	exit(-1);
}

// ����� ���� ������ ���� �Լ�
int recvn( SOCKET s, char *buf, int len, int flags )
{
	int 	received;
	char 	*ptr = buf;
	int 	left = len;

	while( left > 0 ) {
		received = recv(s, ptr, left, flags);
		if(received == SOCKET_ERROR) 
			return SOCKET_ERROR;
		else if(received == 0) 
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}



int main( int argc, char* argv[] )
{
	int		retval, x, y;
	int		rcvLen, targetLen;
	WSADATA	wsa;
	retval = WSAStartup(  MAKEWORD(2, 2), &wsa );
	if(retval != 0)	return -1;

	SOCKET	ClientSocket;
	ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(ClientSocket == INVALID_SOCKET) {
		ErrorDisplay("socket() error(INVALID_SOCKET)");
	}

	SOCKADDR_IN	ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family		= AF_INET;
	ServerAddr.sin_port			= htons(9000);
	ServerAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	
	// 0 ~ 200 ���� ����
	srand(time(NULL));
	x = rand() % 201;
	y = rand() % 201;


	retval = connect(ClientSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));
	if(retval == SOCKET_ERROR) {
		ErrorDisplay("connect() error(SOCKET_ERROR)");
	}

	char		Buf[BUF_SIZE+1], itemp[16] = {0,};
	//char*		cPtr;
	//int		iLen;
	ZeroMemory(Buf, sizeof(Buf));

	
	printf("[TCP Ŭ���̾�Ʈ] ���� ��ġ �ʱ�ȭ.. ���� ��ǥ ���� (%d, %d)\n", x, y);
	// ��ɾ� ���� - add
	// ù ���ڿ� ����
	Buf[0] = (char)2;
	// �������� x, y ����
	*(int*)(Buf + 1) = x;
	*(int*)(Buf + (1 + sizeof(int))) = y;
	// �������� ��ɾ� ����, ���� �ʱ�ȭ�̹Ƿ� a
	*(int*)(Buf + 1 + A_LEN * sizeof(int)) = 'a';

	retval = send(ClientSocket, Buf, 2 * sizeof(int) + 3, 0);
	if (retval == SOCKET_ERROR) {
		printf("<ERROR> send(%s) falied.\n", Buf);
	}

	printf("[TCP Ŭ���̾�Ʈ] ���������� �޼����� ���½��ϴ�. (%dB)\n", retval);
	printf("cnt : %d\n", (int)Buf[0]);
	for (int i = 0; i < A_LEN; i++) {
		printf("��ǥ%d : %d\n", i + 1, *(int*)(Buf + (1 + i * sizeof(int))));
	}
	printf("command : %c\n", Buf[2 * sizeof(int) + 1]);




	
	while (1)
	{
		ZeroMemory(Buf, sizeof(Buf));
		// ��ɾ� ���� - refresh
		// ù ���ڿ� ����
		Buf[0] = (char)2;
		// �������� x, y ����
		*(int*)(Buf + 1) = x;
		*(int*)(Buf + (1 + sizeof(int))) = y;
		// �������� ��ɾ� ����, ���� �ʱ�ȭ�̹Ƿ� a
		*(int*)(Buf + 1 + A_LEN * sizeof(int)) = 'r';
		
		retval = send(ClientSocket, Buf, strlen(Buf), 0);
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> send()(SOCKET_ERROR)!!!\n");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] %d ����Ʈ�� ���½��ϴ�.\n", retval);

		// ���� ������ ���ڷ� �ٽ� ����, ������ ���� ����Ʈ ���� �޾ƾ� �ϴ� ����Ʈ ���� ����
		// �ٵ� �� ����Ʈ ���� �ٸ��� �������� ���� 
		retval = recvn( ClientSocket, Buf, retval, 0 );
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> recvn()(SOCKET_ERROR)!!!\n");
			break;
		}else if(retval == 0)	break;

		Buf[retval]= '\0';
		printf("[TCP Ŭ���̾�Ʈ] %d ����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[���� ����Ÿ] %s \n", Buf);
		Sleep(1000);
	}
	
	closesocket( ClientSocket );
	WSACleanup();
	return 0;
}