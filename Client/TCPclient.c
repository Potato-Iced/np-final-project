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

// ����� ���� ������ ���� �Լ�
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

        printf("\n[TCP Ŭ���̾�Ʈ] ���� ��� ��ǥ�� �����մϴ�. (%d, %d)\n", x, y);
        int sent = send(s, buf, 2 * sizeof(int) + 3, 0);
        printf("[TCP Ŭ���̾�Ʈ] ���������� �޼����� ���½��ϴ�. (%d Bytes)\n", sent);
        printf("@@@@@@@@@@[�߼��� �޼��� ����]@@@@@@@@@@\n");
        printf("$\tcnt : %d\n", buf[0]);
        printf("$\t���� ��ǥ <%d, %d>\n", x, y);
        printf("$\tcommand : %c\n", buf[1 + 2 * sizeof(int)]);
        printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n\n");
        Sleep(5000);    // �ݺ� �ֱ�, 5��

        // select �Լ��� ����Ͽ� ������ ���¸� ����͸�
        struct timeval timeout = { 0, 0 };
        int retval = select(0, &readfds, NULL, NULL, &timeout);

        if (retval == SOCKET_ERROR)
        {
            return SOCKET_ERROR;
        }
        else if (retval == 0)
        {
            // Ÿ�Ӿƿ�
            printf("[TCP Ŭ���̾�Ʈ]�������� ���� �޼����� �����ϴ�.\n");
            continue;
        }
        else if (FD_ISSET(s, &readfds))
        {
            printf("[TCP Ŭ���̾�Ʈ] �����κ��� �޼����� �����Ͽ����ϴ�.\n");
            // �����Ͱ� ���������Ƿ� recv �Լ� ȣ��
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

    // 0 ~ 200 ���� ����
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
    //printf("$\t���� ��ǥ <%d, %d>\n", Buf[1], Buf[1 + 1 * sizeof(int)] = (char)y);
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

        printf("[TCP Ŭ���̾�Ʈ] �����κ��� ���ο� ��ǥ ����: <%d, %d>\n", x, y);
        //Buf[retval]= '\0';
        datacnt = Buf[0];	// �Ǿ� ������ ����
        printf("[TCP Ŭ���̾�Ʈ] �����κ��� %d ����Ʈ�� �޾ҽ��ϴ�.\n", retval);
        printf("�ѤѤѤѤ�[������ �޼��� ����]�ѤѤѤѤ�\n");
        printf("$\tcnt : %d\n", datacnt);
        printf("$\t���� ��ǥ <%d, %d>\n", opening_x, opening_y);
        printf("$\t���� ��ǥ <%d, %d>\n", x, y);
        printf("$\tcommand : %c\n", Buf[1 + (int)datacnt * sizeof(int)]);
        printf("�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n\n\n");

    }

    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}
