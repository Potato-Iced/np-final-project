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

void gotoxy(int x, int y);
void ErrorHandling(char* message);
//POS* RoutePlanner(POS*);	// 드론 이동 경로 탐색용
DWORD WINAPI KeyInputThread(LPVOID param); // 드론 조종 명령 스레드
void statusDraw();	// 현재 드론 위치정보 시각화
void droneInit();	// 드론 배열 초기화


POS droneList[DRONE_AMOUNT + 1] = { 0, }; // �ε��� 0���� �����ε� �����δ� 1, 2, 3�� ���Ŷ� +1, [0]�κ��� �� ����
SOCKET hServSock;
fd_set reads;

int main(void) {
    WSADATA wsaData;
    SOCKADDR_IN servAdr, clntAdr;
    int adrSz, strLen, fdNum, i;
    unsigned char buf[BUF_SIZE];
    int addrlen;

    system("mode con lines=81 cols=122");	// 서버 콘솔 창 크기 조정
    droneInit();							// 드론 저장 배열 초기화

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

    // 키 입력 처리 스레드 생성
    HANDLE hThread = CreateThread(NULL, 0, KeyInputThread, NULL, 0, NULL);
    if (hThread == NULL) {
        ErrorHandling("CreateThread() error");
    }

    while (closenum == 1) {
        fd_set cpyReads = reads;
        TIMEVAL timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 2000;

        fdNum = select(0, &cpyReads, 0, 0, &timeout);
        if (fdNum == SOCKET_ERROR) {
            closesocket(hServSock);
            break;
        }
        else if (fdNum == 0) {
            continue;
        }

        // GUI 갱신
        system("cls");
        statusDraw();

        printf("M 키를 눌러 드론 이동 모드, Q키를 눌러 드론 정렬 및 종료..\n\n");
        if (totalcount > 0) {
            printf("현재 연결된 드론 정보:\n");
            for (int i = 1; i <= totalcount; i++) {
                printf("%d) port = %d, 좌표 = <%d, %d>\n", i, droneList[i].port, droneList[i].x, droneList[i].y);
            }
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
                    // data 수신.
                    strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
                    if (strLen <= 0) {
                        closesocket(reads.fd_array[i]);
                        FD_CLR(reads.fd_array[i], &reads);
                        printf("server> 드론 %d 접속 종료.. (IP:%s)\n",
                            ntohs(clntAdr.sin_port), inet_ntoa(clntAdr.sin_addr));
                        totalcount--;
                        // 등록된 드론 정보 지우기 + 표시 지우기
                        droneList[i].port = 0;
                        gotoxy(droneList[i].x / 4, droneList[i].y / 4);
                        printf(" ");
                        gotoxy(0, 52);
                        break;
                    }
                    else {
                        addrlen = sizeof(clntAdr);
                        getpeername(reads.fd_array[i], (SOCKADDR*)&clntAdr, &addrlen);

                        printf("server> 드론 %d 로부터 메세지 수신 (%d Bytes)\n",
                            ntohs(clntAdr.sin_port), strLen);

                        // 드론의 위치 업데이트
                        //for (int j = 0; j < DRONE_AMOUNT; j++) {
                        //    if (droneList[j].port == 0 || droneList[j].port == ntohs(clntAdr.sin_port)) {
                        //        droneList[j].x = buf[1];
                        //        droneList[j].y = buf[1 + 1 * sizeof(int)];
                        //        droneList[j].port = ntohs(clntAdr.sin_port);
                        //        break;
                        //    }
                        //}
                        // 드론 정보 구조체에 저장
                        // 명령어 종류 상관 없이 x좌표의 인덱스는 [1], y좌표 인덱스는 [1 + 1*sizeof(int)] 고정
                        printf("%%%  현재 for문 i : %d  %%%\n", i);	// debug
                        droneList[i].x = buf[1];
                        droneList[i].y = buf[1 + 1 * sizeof(int)];
                        droneList[i].port = ntohs(clntAdr.sin_port);
                        printf("[드론 %d]\n", droneList[i].port);
                        printf("$\t현재 좌표 <%d, %d>\n", droneList[i].x, droneList[i].y);
                        printf("$\tcommand : %c\n\n\n", buf[1 + 2 * sizeof(int)]);
                    }
                }
            }
        }
        DWORD threadExitCode;
        GetExitCodeThread(hThread, &threadExitCode);
        if (threadExitCode != STILL_ACTIVE) {
            CloseHandle(hThread);
            hThread = CreateThread(NULL, 0, KeyInputThread, NULL, 0, NULL);
            if (hThread == NULL) {
                ErrorHandling("CreateThread() error");
            }
            printf("키 입력 처리 스레드가 재시작되었습니다.\n");
        }
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    closesocket(hServSock);
    WSACleanup();
    return 0;
}

void gotoxy(int x, int y) {
    COORD pos = { x * 2,y };	// 콘솔에서 알파벳은 0.5크기라 2배 해줌
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}


DWORD WINAPI KeyInputThread(LPVOID param) {
    while (1) {
        if (GetAsyncKeyState(0x4D) & 0x8000) {
            int con_port, newX, newY;
            printf("M키 입력 감지\n");
            printf("포트 번호, x 좌표, y 좌표를 입력하세요: ");
            scanf("%d %d %d", &con_port, &newX, &newY);

            if (newY > 200 || newY < 20 || newX > 200)
            {
                printf("좌표값 범위 x좌표 0~200, y좌표 20~200\n");
                printf("좌표값 범위 벗어남, 명령 취소\n");
                break;
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
                        buf[2] = newY;
                        send(reads.fd_array[i], buf, 3, 0);
                        printf("드론 %d의 새로운 좌표 <%d, %d>를 전송했습니다.\n", con_port, newX, newY);
                        break;
                    }
                }
            }
        }

        if (GetAsyncKeyState(0x51) & 0x8000) {
            printf("Q키 입력 감지\n");
            printf("[드론 정렬 모드]\n");

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
                    printf("드론 %d의 정렬좌표 좌표 <%d, %d>를 전송했습니다.\n", ntohs(clientaddr.sin_port), buf[1], buf[2]);
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
                    buf[2] = 100;
                    send(reads.fd_array[i], buf, 3, 0);
                    printf("드론 %d의 정렬후 이동좌표 좌표 <%d, %d>를 전송했습니다.\n", ntohs(clientaddr.sin_port), buf[1], buf[2]);
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

    printf("고도200mㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n"); // 50개
    for (int i = 0; i <= DRONE_AMOUNT; i++) {	// 느낌이 항상 i는 1 2 3만 왔다갔다 하던데
        if (droneList[i].port != 0) {
            gotoxy(droneList[i].x / 4, droneList[i].y / 4);
            printf("★");
        }

    }
    gotoxy(0, 51);
    printf("고도20m-ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ\n"); // 50개
    printf("                                                  ↑기지국\n\n");
}

void ErrorHandling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
void droneInit() {
    for (int i = 0; i <= DRONE_AMOUNT; i++) {
        droneList[i].port = 0;
    }
    return;
}
