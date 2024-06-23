#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>

#define BUF_SIZE 512
#define DRONE_AMOUNT 3

int totalcount = 0, closenum = 1;

typedef struct pos {
    int port;
    int x;
    int y;
} POS;

void ErrorHandling(char* message);

void gotoxy(int x, int y);  // Ŀ�� �̵�
DWORD WINAPI KeyInputThread(LPVOID param); // ������� Ű �Է��� �����ϴ� ������
void statusDraw();	// ���� ��� ��ġ���� �ð�ȭ
void droneInit();	// �ʱ�ȭ

POS droneList[DRONE_AMOUNT + 1] = { 0, };   // �迭 1, 2, 3 ��� �����̶� +1
SOCKET hServSock;
fd_set reads;

int main(void) {
    WSADATA wsaData;
    SOCKADDR_IN servAdr, clntAdr;
    int adrSz, strLen, fdNum, i;
    unsigned char buf[BUF_SIZE];
    int addrlen;

    system("mode con lines=81 cols=122");	// ���� �ܼ� â ũ�� ����
    droneInit();							// ��� ���� �迭 �ʱ�ȭ

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
        timeout.tv_sec = 1;
        timeout.tv_usec = 1000;

        // GUI ����
        system("cls");
        statusDraw();

        printf("M Ű�� ���� ��� �̵� ���, QŰ�� ���� ��� ���� �� ����..\n\n");
        printf("���� ����� ��� ����:\n");
        if (totalcount > 0) {
            for (int i = 1; i <= totalcount; i++) {
                printf("%d) port = %d, ��ǥ = <%d, %d>\n", i, droneList[i].port, droneList[i].x, droneList[i].y);
            }
            printf("\n");
        }
        else {
            printf("server> ���� ����� ����� �����ϴ�!\n");
        }

        fdNum = select(0, &cpyReads, 0, 0, &timeout);


        if (fdNum == SOCKET_ERROR) {
            closesocket(hServSock);
            break;
        }
        else if (fdNum == 0) {
            Sleep(1000);
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
                    // data ����.
                    strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
                    if (strLen <= 0) {
                        closesocket(reads.fd_array[i]);
                        FD_CLR(reads.fd_array[i], &reads);
                        printf("server> ��� %d ���� ����.. (IP:%s)\n",
                            ntohs(clntAdr.sin_port), inet_ntoa(clntAdr.sin_addr));
                        totalcount--;
                        // ��ϵ� ��� ���� ����� + ǥ�� �����
                        droneList[i].port = 0;
                        gotoxy(droneList[i].x / 4, droneList[i].y / 4);
                        printf(" ");
                        gotoxy(0, 52);
                        break;
                    }
                    else {
                        addrlen = sizeof(clntAdr);
                        getpeername(reads.fd_array[i], (SOCKADDR*)&clntAdr, &addrlen);

                        printf("server> ��� %d �κ��� �޼��� ���� (%d Bytes)\n\n",
                            ntohs(clntAdr.sin_port), strLen);

                        // ��� ���� ����ü�� ����
                        droneList[i].x = buf[1];
                        droneList[i].y = buf[1 + 1 * sizeof(int)];
                        droneList[i].port = ntohs(clntAdr.sin_port);
                        printf("[��� %d]\n", droneList[i].port);
                        printf("\t$cnt : \t\t%d\n", buf[0]);
                        printf("\t$���� ��ǥ : \t<%d, %d>\n", droneList[i].x, droneList[i].y);
                        //printf("\t$command :\t%c\n", buf[1 + 2 * sizeof(int)]);
                        Sleep(2000);
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

void gotoxy(int x, int y) {
    COORD pos = { x * 2,y };	// �ֿܼ��� ���ĺ��� 0.5ũ��� x2
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}


DWORD WINAPI KeyInputThread(LPVOID param) {
    while (1) {
        if (GetAsyncKeyState(0x4D) & 0x8000) {  // MŰ�� �������� ������ ���
            int con_port, newX, newY;
            printf("server> MŰ �Է� ����\n");
            printf("server> ��Ʈ ��ȣ, x ��ǥ, y ��ǥ�� �Է��ϼ���: ");
            scanf("%d %d %d", &con_port, &newX, &newY);

            if (newY > 200 || newY < 20 || newX > 200)
            {
                printf("server> ��ǥ�� ���� x��ǥ 0~200, y��ǥ 20~200\n");
                printf("server> [ERROR] ��ǥ�� ���� ���, ��� ���\n");
                fflush(stdin); // ���� ����
            }

            for (int i = 1; i < totalcount + 1; i++) {
                if (reads.fd_array[i] != hServSock) {
                    SOCKADDR_IN clientaddr;
                    int addrlen = sizeof(clientaddr);
                    getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);
                    if (ntohs(clientaddr.sin_port) == con_port) {
                        unsigned char buf[BUF_SIZE] = { 0 };
                        buf[0] = 2;
                        buf[1] = newX;
                        buf[1 + sizeof(int)] = newY;
                        send(reads.fd_array[i], buf, 1 + sizeof(int) * 2, 0);
                        printf("server> ��� %d�� ���ο� ��ǥ <%d, %d>�� �����߽��ϴ�.\n", con_port, newX, newY);
                        break;
                    }
                }
            }
        }

        if (GetAsyncKeyState(0x51) & 0x8000) { // QŰ�� �������� ������ ���
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
                    buf[1 + sizeof(int)] = 100;
                    send(reads.fd_array[i], buf, 1 + sizeof(int) * 2, 0);
                    printf("server> ��� %d�� ������ǥ ��ǥ <%d, %d>�� �����߽��ϴ�.\n", ntohs(clientaddr.sin_port), buf[1], buf[2]);
                    Sleep(5000);

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
                    buf[1 + sizeof(int)] = 100;
                    send(reads.fd_array[i], buf, 1 + sizeof(int) * 2, 0);
                    printf("��� %d�� ������ �̵���ǥ ��ǥ <%d, %d>�� �����߽��ϴ�.\n", ntohs(clientaddr.sin_port), buf[1], buf[2]);
                    Sleep(5000);

                }
            }
            for (int i = 0; i < reads.fd_count; i++) {
                Sleep(5000);

                closesocket(reads.fd_array[i]);

            }
            closenum = 0;
            break;
        }

        Sleep(100);
    }
    return 0;
}

void statusDraw() {

    printf("��200m�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n"); // 50��
    for (int i = 0; i <= DRONE_AMOUNT; i++) {
        if (droneList[i].port != 0) {
            gotoxy(droneList[i].x / 4, droneList[i].y / 4);
            printf("��");
        }

    }
    gotoxy(0, 51);
    printf("��20m-�ѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤѤ�\n"); // 50��
    printf("                                                  �������\n\n");
}

void ErrorHandling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void droneInit() {
    printf("\n�ѤѤѤѤ� TCP ��Ʈ��ũ ��� ��� ���α׷� �ѤѤѤ�\n\n");
    printf("server> ��Ʈ��ũ���α׷��� ��������Ʈ - 2024�г⵵ 1�б�\n");
    printf("server> ����1 : 2020152019 ������ - GUI ����, ���� ���� ����\n");
    printf("server> ����2 : 2019156012 ����ǥ - ��Ƽ������ ����, ���� �ۼ�\n");
    Sleep(3000);
    for (int i = 0; i <= DRONE_AMOUNT; i++) {
        droneList[i].port = 0;
    }
    printf("server> �� ���α׷��� �����մϴ�.\n");
    Sleep(2000);
    return;
}
