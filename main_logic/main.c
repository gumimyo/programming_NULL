#include <stdio.h>

// 상수를 활용한 보드 크기 정의 (15x15 표준 오목판)
#define BOARD_SIZE 15

// 전역 변수로 바둑판 선언 (0: 빈칸, 1: 흑돌 '●', 2: 백돌 '○')
int board[BOARD_SIZE][BOARD_SIZE];

// 함수 선언
void initBoard();
void printBoard();
int placeStone(int row, int col, int player);
int checkWin(int row, int col, int player);

int main() {
    int turn = 1; // 1: 흑돌(Player 1), 2: 백돌(Player 2)
    int row, col;
    int winner = 0;

    initBoard(); // 바둑판 초기화

    printf("===================================\n");
    printf("     C언어 기초 오목 게임\n");
    printf("     흑돌: ● | 백돌: ○\n");
    printf("===================================\n");

    // 게임 루프
    while (1) {
        printBoard();

        // 입력 받기
        printf("\n[Player %d (%s)] 좌표 입력 (행 열, 예: 7 7): ", turn, (turn == 1) ? "●" : "○");
        scanf("%d %d", &row, &col);

        // 돌 놓기 검증 및 실행
        if (placeStone(row, col, turn) == 0) {
            printf("[오류] 올바르지 않은 위치입니다. 다시 입력하세요!\n");
            continue; // 잘못 입력했을 경우 차례를 넘기지 않고 다시 입력
        }

        // 승리 조건 검사
        if (checkWin(row, col, turn)) {
            winner = turn;
            break; // 승자가 나오면 루프 탈출
        }

        // 턴 전환 (1 -> 2, 2 -> 1)
        if (turn == 1) {
            turn = 2;
        } else {
            turn = 1;
        }
    }

    // 최종 결과 출력
    printBoard();
    printf("\n===================================\n");
    printf("★ 게임 종료! Player %d (%s)의 승리입니다! ★\n", winner, (winner == 1) ? "●" : "○");
    printf("===================================\n");

    return 0;
}

// 바둑판을 모두 빈칸(0)으로 초기화하는 함수
void initBoard() {
    int i, j;
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = 0;
        }
    }
}

// 화면에 바둑판을 그려주는 함수 (이중 반복문 활용)
void printBoard() {
    int i, j;
    
    // 열 번호 출력
    printf("\n   ");
    for (i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
    }
    printf("\n");

    for (i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i); // 행 번호 출력
        for (j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0) {
                printf(" + "); // 빈 칸
            } else if (board[i][j] == 1) {
                printf(" ● "); // 흑돌
            } else if (board[i][j] == 2) {
                printf(" ○ "); // 백돌
            }
        }
        printf("\n");
    }
}

// 돌을 놓는 함수 (성공 시 1 반환, 실패 시 0 반환)
int placeStone(int row, int col, int player) {
    // 범위를 벗어났는지 확인
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
        return 0;
    }
    // 이미 돌이 놓여있는지 확인
    if (board[row][col] != 0) {
        return 0;
    }
    
    board[row][col] = player;
    return 1;
}

// 돌이 놓인 시점에서 오목(5개 연속)이 완성되었는지 확인하는 함수
int checkWin(int row, int col, int player) {
    // 탐색할 4가지 방향 (가로, 세로, 우하향 대각선, 우상향 대각선)
    int dr[4] = {0, 1, 1, 1};
    int dc[4] = {1, 0, 1, -1};
    int i, step;

    for (i = 0; i < 4; i++) {
        int count = 1; // 방금 놓은 돌을 포함하여 1부터 시작

        // 정방향 탐색 (step = 1, 2, 3, 4)
        for (step = 1; step < 5; step++) {
            int nr = row + dr[i] * step;
            int nc = col + dc[i] * step;

            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
                count++;
            } else {
                break;
            }
        }

        // 역방향 탐색 (step = 1, 2, 3, 4)
        for (step = 1; step < 5; step++) {
            int nr = row - dr[i] * step;
            int nc = col - dc[i] * step;

            if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE && board[nr][nc] == player) {
                count++;
            } else {
                break;
            }
        }

        // 연속된 돌이 정확히 5개이거나 그 이상(장목 포함 기본 5개 이상)이면 승리
        if (count >= 5) {
            return 1; 
        }
    }

    return 0; // 아직 5개가 안 됨
}