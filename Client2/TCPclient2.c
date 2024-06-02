#include <winsock2.h>
#include <stdio.h>
#define	BUF_SIZE	512

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
int		retval;	

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
	
	retval = connect(ClientSocket, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr));
	if(retval == SOCKET_ERROR) {
		ErrorDisplay("connect() error(SOCKET_ERROR)");
	}

	char		Buf[BUF_SIZE+1];
	//int		iLen;
	while(1)
	{
		ZeroMemory(Buf, sizeof(Buf));
		printf("\n[보낼 데이타] ");
		if(fgets( Buf, BUF_SIZE+1, stdin ) == NULL)	break;
		retval = send(ClientSocket, Buf, strlen(Buf), 0);
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> send()(SOCKET_ERROR)!!!\n");
			break;
		}
		printf("[TCP 클라이언트] %d 바이트를 보냈습니다.\n", retval);

		retval = recvn( ClientSocket, Buf, retval, 0 );
		if(retval == SOCKET_ERROR) {
			printf("<ERROR> recvn()(SOCKET_ERROR)!!!\n");
			break;
		}else if(retval == 0)	break;

		Buf[retval]= '\0';
		printf("[TCP 클라이언트] %d 바이트를 받았습니다.\n", retval);
		printf("[받은 데이타] %s \n", Buf);
	}
	closesocket( ClientSocket );
	WSACleanup();
	return 0;
}