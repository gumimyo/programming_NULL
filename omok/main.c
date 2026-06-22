#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
/* 윈도우 콘솔에서 한글/유니코드/색상이 깨지지 않도록 설정 (윈도우 전용) */
void enableConsole(void){
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | 0x0004 /* 가상 터미널 처리 켜기 */);
}
#else
void enableConsole(void){}
#endif

/* ===== 색상 코드(ANSI)를 문자 배열에 저장 (매크로 없이 변수로 관리) ===== */
char reset[]  = "\033[0m";
char cBoard[] = "\033[38;5;179m";   /* 나무색 판 */
char cStone[] = "\033[1;38;5;231m"; /* 돌 (밝은 흰색) */
char cLast[] = "\033[1;38;5;121m"; /* 마지막에 둔 돌 (빨강 강조) */
char cWarn[]  = "\033[1;38;5;214m"; /* 경고 메시지 */
char cWin[]   = "\033[1;38;5;46m";  /* 승리 메시지 */
char cTurn2[] = "\033[1;38;5;245m"; /* 백돌 차례 글자 */

/* ===== 오목판: 2차원 배열 (0=빈칸, 1=흑돌, 2=백돌), 판 크기 15 x 15 ===== */
int  board[15][15];
int  lastR = -1, lastC = -1;   /* 가장 최근에 둔 좌표 */
char status[128] = "";         /* 화면 하단 안내/오류 메시지 */

/* 판 초기화: 모든 칸을 0(빈칸)으로 */
void initBoard(void){
    for (int i = 0; i < 15; i++)
        for (int j = 0; j < 15; j++)
            board[i][j] = 0;
}

/* 화점(별 위치)인지 확인 — 보기 좋으라고 표시 */
int isStar(int i, int j){
    int r = (i == 3 || i == 7 || i == 11);
    int c = (j == 3 || j == 7 || j == 11);
    return r && c;
}

/* 빈 교차점에 그릴 격자 문자 (모서리/변/안쪽 구분) */
const char* gridChar(int i, int j){
    int top = (i == 0), bot = (i == 14);
    int lft = (j == 0), rgt = (j == 14);
    if (top && lft) return "┌";
    if (top && rgt) return "┐";
    if (bot && lft) return "└";
    if (bot && rgt) return "┘";
    if (top) return "┬";
    if (bot) return "┴";
    if (lft) return "├";
    if (rgt) return "┤";
    if (isStar(i, j)) return "╋";   /* 화점 */
    return "┼";
}

/* 오목판 출력 */
void printBoard(void){
    printf("\033[2J\033[H");   /* 화면 지우기 + 커서 맨 위로 */

    printf("\n   %s \n\
██████╗ ███╗   ███╗ ██████╗ ██╗  ██╗\n\
██╔═══██╗████╗ ████║██╔═══██╗██║ ██╔╝\n\
██║   ██║██╔████╔██║██║   ██║█████╔╝\n\
██║   ██║██║╚██╔╝██║██║   ██║██╔═██╗\n\
╚██████╔╝██║ ╚═╝ ██║╚██████╔╝██║  ██╗\n\
 ╚═════╝ ╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═╝\n\
    ==========================\n\
        \n\
            < OMOK GAME >\n\
        \n\
    ==========================%s\n\n", cBoard, reset);

    /* 열 번호 */
    printf("   ");
    for (int j = 0; j < 15; j++)
        printf("%s%-2d%s", cBoard, j + 1, reset);
    printf("\n");

    for (int i = 0; i < 15; i++){
        printf("%s%2d %s", cBoard, i + 1, reset);      /* 행 번호 */
        for (int j = 0; j < 15; j++){
            if (board[i][j] == 1){                     /* 흑돌 */
                if (i == lastR && j == lastC) printf("%s●%s", cLast, reset);
                else                          printf("%s●%s", cStone, reset);
            } else if (board[i][j] == 2){              /* 백돌 */
                if (i == lastR && j == lastC) printf("%s○%s", cLast, reset);
                else                          printf("%s○%s", cStone, reset);
            } else {                                   /* 빈칸 → 격자 */
                printf("%s%s%s", cBoard, gridChar(i, j), reset);
            }
            if (j < 14) printf("%s─%s", cBoard, reset);
        }
        printf("\n");
    }

    printf("\n   %s●%s 흑돌   %s○%s 백돌   %s●%s 마지막 수\n",
           cStone, reset, cStone, reset, cLast, reset);
    if (status[0]) printf("   %s\n", status);
}

/* (dr,dc) 방향으로 같은 돌이 몇 개 이어지는지 세기 */
int countDir(int r, int c, int dr, int dc, int p){
    int cnt = 0, nr = r + dr, nc = c + dc;
    while (nr >= 0 && nr < 15 && nc >= 0 && nc < 15 && board[nr][nc] == p){
        cnt++;
        nr += dr; nc += dc;
    }
    return cnt;
}

/* (r,c)에 둔 돌 기준으로 5목 완성 여부 검사 */
int checkWin(int r, int c, int p){
    int dr[4] = {0, 1, 1,  1};   /* 가로, 세로, ↘, ↙ */
    int dc[4] = {1, 0, 1, -1};
    for (int i = 0; i < 4; i++){
        int total = 1
                  + countDir(r, c,  dr[i],  dc[i], p)
                  + countDir(r, c, -dr[i], -dc[i], p);
        if (total == 5) return 1;
    }
    return 0;
}

int main(void){
    enableConsole();
    initBoard();

    int player = 1;   /* 1=흑돌 선공, 2=백돌 */
    int moves  = 0;

    while (1){
        printBoard();

        if (player == 1) printf("\n   %s● 흑돌 차례%s", cStone, reset);
        else             printf("\n   %s○ 백돌 차례%s", cTurn2, reset);
        printf("  —  좌표를 [행 열] 로 입력 (예: 8 8 / 종료: 0 0): ");

        int r, c;
        int got = scanf("%d %d", &r, &c);

        /* 숫자가 아닌 값이 들어온 경우 입력 버퍼 비우기 */
        if (got != 2){
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            sprintf(status, "%s⚠ 숫자 두 개를 공백으로 구분해 입력하세요.%s", cWarn, reset);
            continue;
        }

        if (r == 0 && c == 0){
            printf("\n   게임을 종료합니다.\n");
            break;
        }
        if (r < 1 || r > 15 || c < 1 || c > 15){
            sprintf(status, "%s⚠ 1 ~ 15 범위 안에서 입력하세요.%s", cWarn, reset);
            continue;
        }
        if (board[r-1][c-1] != 0){
            sprintf(status, "%s⚠ 이미 돌이 놓인 자리입니다.%s", cWarn, reset);
            continue;
        }

        /* 돌 놓기 */
        board[r-1][c-1] = player;
        lastR = r - 1; lastC = c - 1;
        moves++;
        status[0] = '\0';

        /* 승리 판정 */
        if (checkWin(r-1, c-1, player)){
            printBoard();
            if (player == 1) printf("\n   %s★  흑돌(●) 승리!  ★%s\n", cWin, reset);
            else             printf("\n   %s★  백돌(○) 승리!  ★%s\n", cWin, reset);
            break;
        }
        /* 무승부 (판이 꽉 참) */
        if (moves == 15 * 15){
            printBoard();
            printf("\n   무승부입니다!\n");
            break;
        }

        player = (player == 1) ? 2 : 1;   /* 차례 넘기기 */
    }
    return 0;
}
//이거 한번 실행해봐 개선 안됬으면 다시 해옴
