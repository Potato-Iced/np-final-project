#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>

#define BUF_SIZE 1024
#define DRONE_AMOUNT 3

int totalcount = 0, closenum = 1;

typedef struct pos {
    int port;
    int x;
    int y;
} POS;

void ErrorHandling(char* message);
//POS* RoutePlanner(POS*);	// ��� �̵� ��� Ž����
DWORD WINAPI KeyInputThread(LPVOID param); // ��� ���� ��� ������

POS droneList[DRONE_AMOUNT] = { 0, };
SOCKET hServSock;
fd_set reads;

int main(void) {
    WSADATA wsaData;
    SOCKADDR_IN servAdr, clntAdr;
    int adrSz, strLen, fdNum, i;
    unsigned char buf[BUF_SIZE];
    int addrlen;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servAdr.sin_port = htons(9000);

    if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");
    if (listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error");

    FD_ZERO(&reads);
    FD_SET(hServSock, &reads);

    // Ű �Է� ó�� ������ ����
    HANDLE hThread = CreateThread(NULL, 0, KeyInputThread, NULL, 0, NULL);
    if (hThread == NULL) {
        ErrorHandling("CreateThread() error");
    }

    while (closenum == 1) {
        fd_set cpyReads = reads;
        TIMEVAL timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        fdNum = select(0, &cpyReads, 0, 0, &timeout);
        if (fdNum == SOCKET_ERROR) {
            closesocket(hServSock);
            break;
        }
        else if (fdNum == 0) {
            continue;
        }

        for (i = 0; i < reads.fd_count; i++) {
            if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
                if (reads.fd_array[i] == hServSock) {
                    adrSz = sizeof(clntAdr);
                    SOCKET hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
                    if (hClntSock == INVALID_SOCKET) {
                        continue;
                    }
                    printf("server> connected client: Port:%d, IP:%s \n",
                        ntohs(clntAdr.sin_port), inet_ntoa(clntAdr.sin_addr));
                    totalcount++;
                    FD_SET(hClntSock, &reads);
                }
                else {
                    strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
                    if (strLen <= 0) {
                        closesocket(reads.fd_array[i]);
                        FD_CLR(reads.fd_array[i], &reads);
                    }
                    else {
                        addrlen = sizeof(clntAdr);
                        getpeername(reads.fd_array[i], (SOCKADDR*)&clntAdr, &addrlen);

                        printf("server> ��� %d �κ��� �޼��� ���� (%d Bytes)\n",
                            ntohs(clntAdr.sin_port), strLen);

                        // ����� ��ġ ������Ʈ
                        for (int j = 0; j < DRONE_AMOUNT; j++) {
                            if (droneList[j].port == 0 || droneList[j].port == ntohs(clntAdr.sin_port)) {
                                droneList[j].x = buf[1];
                                droneList[j].y = buf[1 + 1 * sizeof(int)];
                                droneList[j].port = ntohs(clntAdr.sin_port);
                                break;
                            }
                        }

                        printf("[��� %d]\n", ntohs(clntAdr.sin_port));
                        printf("$\t���� ��ǥ <%d, %d>\n", buf[1], buf[1 + 1 * sizeof(int)]);
                    }
                }
            }
        }
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    closesocket(hServSock);
    WSACleanup();
    return 0;
}

DWORD WINAPI KeyInputThread(LPVOID param) {
    while (1) {
        if (GetAsyncKeyState(0x4D) & 0x8000) {
            int con_port, newX, newY;
            printf("MŰ �Է� ����\n");
            printf("��Ʈ ��ȣ, x ��ǥ, y ��ǥ�� �Է��ϼ���: ");
            scanf("%d %d %d", &con_port, &newX, &newY);

            if (newY > 200 || newY < 20 || newX > 200)
            {
                printf("��ǥ�� ���� x��ǥ 0~200, y��ǥ 20~200\n");
                printf("��ǥ�� ���� ���, ��� ���\n");
                break;
            }

            for (int i = 1; i < totalcount+1; i++) {
                if (reads.fd_array[i] != hServSock) {
                    SOCKADDR_IN clientaddr;
                    int addrlen = sizeof(clientaddr);
                    getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);
                    if (ntohs(clientaddr.sin_port) == con_port) {
                        unsigned char buf[BUF_SIZE] = { 0 };
                        buf[0] = 2;
                        buf[1] = newX;
                        buf[2] = newY;
                        send(reads.fd_array[i], buf, 3, 0);
                        printf("��� %d�� ���ο� ��ǥ <%d, %d>�� �����߽��ϴ�.\n", con_port, newX, newY);
                        break;
                    }
                }
            }
        }

        if (GetAsyncKeyState(0x51) & 0x8000) {
            printf("QŰ �Է� ����\n");
            printf("[��� ���� ���]\n");

            for (int i = 0; i < reads.fd_count; i++) {
                if (reads.fd_array[i] != hServSock) {
                    SOCKADDR_IN clientaddr;
                    int addrlen = sizeof(clientaddr);
                    getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);
                    unsigned char buf[BUF_SIZE] = { 0 };
                    buf[0] = 2;
                    buf[1] = i * 20;
                    buf[2] = 100;
                    send(reads.fd_array[i], buf, 3, 0);
                    printf("��� %d�� ������ǥ ��ǥ <%d, %d>�� �����߽��ϴ�.\n", ntohs(clientaddr.sin_port), buf[1], buf[2]);
                    Sleep(7000);
                    
                }
            }

            for (int i = 0; i < reads.fd_count; i++) {
                if (reads.fd_array[i] != hServSock) {
                    SOCKADDR_IN clientaddr;
                    int addrlen = sizeof(clientaddr);
                    getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);
                    unsigned char buf[BUF_SIZE] = { 0 };
                    buf[0] = 2;
                    buf[1] = (i * 20) + 50;
                    buf[2] = 100;
                    send(reads.fd_array[i], buf, 3, 0);
                    printf("��� %d�� ������ �̵���ǥ ��ǥ <%d, %d>�� �����߽��ϴ�.\n", ntohs(clientaddr.sin_port), buf[1], buf[2]);
                    Sleep(7000);
                    
                }
            }
            for (int i = 0; i < reads.fd_count; i++) {
                Sleep(10000);

                closesocket(reads.fd_array[i]);

            }
            closenum = 0;
            break;
        }

        Sleep(100);
    }
    return 0;
}

void ErrorHandling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
