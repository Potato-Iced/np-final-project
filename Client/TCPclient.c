#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define BUF_SIZE 512

int  x, y;
void ErrorDisplay(char* str)
{
    printf("<ERROR> %s!!!\n", str);
    exit(-1);
}

// 사용자 정의 데이터 수신 함수
int recvn(SOCKET s, char* buf, int len, int flags, int x, int y)
{
    int received;
    char* ptr = buf;
    int left = len;

    while (left > 0)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(s, &readfds);

        printf("\n[TCP 클라이언트] 현재 드론 좌표를 전송합니다. (%d, %d)\n", x, y);
        int sent = send(s, buf, 2 * sizeof(int) + 3, 0);
        printf("[TCP 클라이언트] 성공적으로 메세지를 보냈습니다. (%d Bytes)\n", sent);
        printf("@@@@@@@@@@[발송한 메세지 내용]@@@@@@@@@@\n");
        printf("$\tcnt : %d\n", buf[0]);
        printf("$\t현재 좌표 <%d, %d>\n", x, y);
        printf("$\tcommand : %c\n", buf[1 + 2 * sizeof(int)]);
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n\n");
        Sleep(5000);    // 반복 주기, 5초

        // select 함수를 사용하여 소켓의 상태를 모니터링
        struct timeval timeout = { 0, 0 };
        int retval = select(0, &readfds, NULL, NULL, &timeout);

        if (retval == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
        else if (retval == 0)
        {
            // 타임아웃
            printf("[TCP 클라이언트]서버에서 보낸 메세지가 없습니다.\n");
            continue;
        }
        else if (FD_ISSET(s, &readfds))
        {
            printf("[TCP 클라이언트] 서버로부터 메세지가 도착하였습니다.\n");
            // 데이터가 도착했으므로 recv 함수 호출
            received = recv(s, ptr, left, flags);
            if (received == SOCKET_ERROR)
                return SOCKET_ERROR;
            else if (received == 0)
                break;

            left -= received;
            ptr += received;
        }
    }

    return (len - left);
}


int main(int argc, char* argv[])
{
    int retval, opening_x, opening_y;
    //int		rcvLen, targetLen;
    char	datacnt;
    WSADATA wsa;
    retval = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (retval != 0) return -1;

    SOCKET ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSocket == INVALID_SOCKET) {
        ErrorDisplay("socket() error(INVALID_SOCKET)");
    }

    SOCKADDR_IN ServerAddr;
    ZeroMemory(&ServerAddr, sizeof(ServerAddr));
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(9000);
    ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 0 ~ 200 난수 생성
    srand(time(NULL));
    x = rand() % 201;
    y = rand() % 181 + 20;

    opening_x = x;
    opening_y = y;

    retval = connect(ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
    if (retval == SOCKET_ERROR) {
        ErrorDisplay("connect() error(SOCKET_ERROR)");
    }

    unsigned char Buf[BUF_SIZE] = { 0 };
    printf("[TCP 클라이언트] 최초 위치 초기화.. 현재 좌표 전송 (%d, %d)\n", x, y);
    // 명령어 생성 - add
    // 첫 글자에 개수
    Buf[0] = (char)2;
    // 나머지는 x, y 대입
    Buf[1] = x;
    Buf[1 + 1 * sizeof(int)] = y;
    // 마지막에 명령어 종류, 최초 초기화이므로 a
    Buf[1 + 2 * sizeof(int)] = 'a';

    retval = send(ClientSocket, Buf, 2 * sizeof(int) + 3, 0);
    if (retval == SOCKET_ERROR) {
        printf("<ERROR> send(%s) falied.\n", Buf);
    }

    printf("[TCP 클라이언트] 성공적으로 메세지를 보냈습니다. (%d Bytes)\n", retval);
    printf("보낸 메세지:\n");
    printf("$\tcnt : %d\n", Buf[0]);
    //printf("$\t현재 좌표 <%d, %d>\n", Buf[1], Buf[1 + 1 * sizeof(int)] = (char)y);
    printf("$\tcommand : %c\n", Buf[1 + 2 * sizeof(int)]);

    while (1)
    { 
        opening_x = x;
        opening_y = y;

        ZeroMemory(Buf, sizeof(Buf));
        Buf[0] = (char)2;
        Buf[1] = x;
        Buf[1 + 1 * sizeof(int)] = y;
        Buf[1 + 2 * sizeof(int)] = 'r';

        retval = send(ClientSocket, Buf, 2 * sizeof(int) + 3, 0);
        if (retval == SOCKET_ERROR) {
            printf("<ERROR> send()(SOCKET_ERROR)!!!\n");
            break;
        }

        retval = recvn(ClientSocket, Buf, 3, 0, x, y);
        if (retval == SOCKET_ERROR) {
            printf("<ERROR> recvn()(SOCKET_ERROR)!!!\n");
            break;
        }
        else if (retval == 0) break;

        x = Buf[1];
        y = Buf[2];

        printf("[TCP 클라이언트] 서버로부터 새로운 좌표 수신: <%d, %d>\n", x, y);
        //Buf[retval]= '\0';
        datacnt = Buf[0];	// 맨앞 데이터 개수
        printf("[TCP 클라이언트] 서버로부터 %d 바이트를 받았습니다.\n", retval);
        printf("ㅡㅡㅡㅡㅡ[수신한 메세지 내용]ㅡㅡㅡㅡㅡ\n");
        printf("$\tcnt : %d\n", datacnt);
        printf("$\t현재 좌표 <%d, %d>\n", opening_x, opening_y);
        printf("$\t변경 좌표 <%d, %d>\n", x, y);
        printf("$\tcommand : %c\n", Buf[1 + (int)datacnt * sizeof(int)]);
        printf("ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n\n\n");

    }

    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}
