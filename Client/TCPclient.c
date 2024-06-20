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
	char	datacnt;
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
	y = rand() % 181 + 20;


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
	Buf[1] = x;
	Buf[1 + 1 * sizeof(int)] = y;
	// �������� ��ɾ� ����, ���� �ʱ�ȭ�̹Ƿ� a
	Buf[1 + 2 * sizeof(int)] = 'a';

	retval = send(ClientSocket, Buf, 2 * sizeof(int) + 3, 0);
	if (retval == SOCKET_ERROR) {
		printf("<ERROR> send(%s) falied.\n", Buf);
	}

	printf("[TCP Ŭ���̾�Ʈ] ���������� �޼����� ���½��ϴ�. (%d Bytes)\n", retval);
	printf("���� �޼���:\n");
	printf("$\tcnt : %d\n", Buf[0]);
	printf("$\t���� ��ǥ <%d, %d>\n", Buf[1], Buf[1 + 1 * sizeof(int)] = (char)y);
	printf("$\tcommand : %c\n", Buf[1 + 2 * sizeof(int)]);




	
	while (1)
	{
		ZeroMemory(Buf, sizeof(Buf));
		// ��ɾ� ���� - refresh
		// ù ���ڿ� ����
		Buf[0] = (char)2;
		// �������� x, y ����
		Buf[1] = x;
		Buf[1 + 1 * sizeof(int)] = y;
		// �������� ��ɾ� ����, �������ʹ� �ݺ������� ��ǥ ��� �����̹Ƿ� r(refresh)
		Buf[1 + 2 * sizeof(int)] = 'r';
		
		
		retval = send(ClientSocket, Buf, 2 * sizeof(int) + 3, 0);
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> send()(SOCKET_ERROR)!!!\n");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] ���������� �޼����� ���½��ϴ�. (%d Bytes)\n", retval);

		// ������� �� targetLen - recvLen �̰� �ؾ��ҵ�
		// �ڲ� recv�ϸ鼭 ���� �д°� �����µ� -> ���� �� send()���� ���� ����Ʈ ����, �ذ��
		retval = recvn( ClientSocket, Buf, retval, 0 );
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> recvn()(SOCKET_ERROR)!!!\n");
			break;
		}else if(retval == 0)	break;

		//Buf[retval]= '\0';
		datacnt = Buf[0];	// �Ǿ� ������ ����
		printf("[TCP Ŭ���̾�Ʈ] �����κ��� %d ����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("�ѤѤѤѤ�[������ �޼��� ����]�ѤѤѤѤ�\n");
		printf("$\tcnt : %d\n", datacnt);
		printf("$\t���� ��ǥ <%d, %d>\n", Buf[1], Buf[1 + 1 * sizeof(int)]);
		printf("$\tcommand : %c\n", Buf[1 + (int)datacnt * sizeof(int)]);
		printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n\n");

		

		Sleep(5000); // �ݺ� �ֱ�
	}
	
	closesocket( ClientSocket );
	WSACleanup();
	return 0;
}