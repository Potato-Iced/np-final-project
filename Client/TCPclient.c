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

// 사용자 정의 데이터 수신 함수
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
	
	// 0 ~ 200 난수 생성
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

	
	printf("[TCP 클라이언트] 최초 위치 초기화.. 현재 좌표 전송 (%d, %d)\n", x, y);
	// 명령어 생성 - add
	// 첫 글자에 개수
	Buf[0] = (char)2;
	// 나머지는 x, y 대입
	*(int*)(Buf + 1) = x;
	*(int*)(Buf + (1 + sizeof(int))) = y;
	// 마지막에 명령어 종류, 최초 초기화이므로 a
	*(int*)(Buf + 1 + A_LEN * sizeof(int)) = 'a';

	retval = send(ClientSocket, Buf, 2 * sizeof(int) + 3, 0);
	if (retval == SOCKET_ERROR) {
		printf("<ERROR> send(%s) falied.\n", Buf);
	}

	printf("[TCP 클라이언트] 성공적으로 메세지를 보냈습니다. (%dB)\n", retval);
	printf("cnt : %d\n", (int)Buf[0]);
	for (int i = 0; i < A_LEN; i++) {
		printf("좌표%d : %d\n", i + 1, *(int*)(Buf + (1 + i * sizeof(int))));
	}
	printf("command : %c\n", Buf[2 * sizeof(int) + 1]);




	
	while (1)
	{
		ZeroMemory(Buf, sizeof(Buf));
		// 명령어 생성 - refresh
		// 첫 글자에 개수
		Buf[0] = (char)2;
		// 나머지는 x, y 대입
		*(int*)(Buf + 1) = x;
		*(int*)(Buf + (1 + sizeof(int))) = y;
		// 마지막에 명령어 종류, 최초 초기화이므로 a
		*(int*)(Buf + 1 + A_LEN * sizeof(int)) = 'r';
		
		retval = send(ClientSocket, Buf, strlen(Buf), 0);
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> send()(SOCKET_ERROR)!!!\n");
			break;
		}
		printf("[TCP 클라이언트] %d 바이트를 보냈습니다.\n", retval);

		// 원래 서버는 에코로 다시 전송, 기존에 보낸 바이트 수와 받아야 하는 바이트 수가 같음
		// 근데 이 바이트 수가 다르면 서버에서 정렬 
		retval = recvn( ClientSocket, Buf, retval, 0 );
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> recvn()(SOCKET_ERROR)!!!\n");
			break;
		}else if(retval == 0)	break;

		Buf[retval]= '\0';
		printf("[TCP 클라이언트] %d 바이트를 받았습니다.\n", retval);
		printf("[받은 데이타] %s \n", Buf);
		Sleep(1000);
	}
	
	closesocket( ClientSocket );
	WSACleanup();
	return 0;
}