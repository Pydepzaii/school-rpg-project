#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <time.h>
#include <windows.h>

// config
#define cols 20
#define rows 20
#define TOTAL_COINS 5

// ===================== global variables =====================

char base_board[cols * rows];   // map 
char board[cols * rows];        

int gameOver = 0;
int locateX = 5;
int locateY = 5;

int score = 0;
int totalCoins = TOTAL_COINS;

// NPC
int npcX = 10;
int npcY = 10;

// ===================== console helpers (giảm nháy) =====================
void clear_screen_fast() {
    static HANDLE hConsole = NULL;
    if (!hConsole) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo(hConsole, &info);
        info.bVisible = FALSE; // ẩn cursor
        SetConsoleCursorInfo(hConsole, &info);
    }
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hConsole, pos);
}

// ===================== board helpers =====================
void fill_base_board() {
    int x, y;
    for (y = 0; y < rows; y++) {
        for (x = 0; x < cols; x++) {
            if (x == 0 || y == 0 || x == cols - 1 || y == rows - 1) {
                base_board[y * cols + x] = '#';
            } else {
                base_board[y * cols + x] = ' ';
            }
        }
    }
}

int in_bounds(int x, int y) {
    return (x >= 0 && x < cols && y >= 0 && y < rows);
}

char get_base_cell(int x, int y) {
    if (!in_bounds(x, y)) return '#';
    return base_board[y * cols + x];
}

void set_base_cell(int x, int y, char c) {
    if (!in_bounds(x, y)) return;
    base_board[y * cols + x] = c;
}

int manhattan(int ax, int ay, int bx, int by) {
    int dx = ax - bx; if (dx < 0) dx = -dx;
    int dy = ay - by; if (dy < 0) dy = -dy;
    return dx + dy;
}

// copy base -> board, rồi overlay npc + player
void build_frame() {
    int i;
    for (i = 0; i < cols * rows; i++) board[i] = base_board[i];

    // NPC overlay (để NPC không bị “xóa” khi build frame)
    board[npcY * cols + npcX] = 'N';

    // Player overlay
    board[locateY * cols + locateX] = '8';
}

void print_board() {
    int x, y;

    clear_screen_fast();

    for (y = 0; y < rows; y++) {
        for (x = 0; x < cols; x++) {
            putch(board[y * cols + x]);
        }
        putch('\n');
    }

    printf("Score: %d/%d | WASD move | E talk | Q quit\n", score, totalCoins);

    if (manhattan(locateX, locateY, npcX, npcY) == 1) {
        printf("NPC nearby: press E to talk\n");
    } else {
        printf("                           \n");
    }
}

// ===================== gameplay: coins + npc + movement =====================
void place_coins_random(int n) {
    int placed = 0;
    while (placed < n) {
        int x = 1 + rand() % (cols - 2);
        int y = 1 + rand() % (rows - 2);

        // tránh đặt trùng player hoặc NPC hoặc trùng coin
        if ((x == locateX && y == locateY) || (x == npcX && y == npcY)) continue;
        if (get_base_cell(x, y) == '+') continue;

        set_base_cell(x, y, '+');
        placed++;
    }
}

int is_blocked(int x, int y) {
    if (get_base_cell(x, y) == '#') return 1;
    // NPC coi như vật cản
    if (x == npcX && y == npcY) return 1;
    return 0;
}

void try_move(int dx, int dy) {
    int nx = locateX + dx;
    int ny = locateY + dy;

    if (!in_bounds(nx, ny)) return;
    if (is_blocked(nx, ny)) return;

    // nếu bước lên coin -> nhặt
    if (get_base_cell(nx, ny) == '+') {
        score++;
        set_base_cell(nx, ny, ' ');
    }

    locateX = nx;
    locateY = ny;
}

void show_dialog_box(const char *line1, const char *line2) {
    // In dialog đơn giản dưới board
    printf("\n+------------------DIALOG------------------+\n");
    printf("| %s\n", line1);
    printf("| %s\n", line2);
    printf("+------------------------------------------+\n");
    printf("Press any key...\n");
    _getch();
}

void interact() {
    // chỉ nói chuyện khi đứng cạnh NPC
    if (manhattan(locateX, locateY, npcX, npcY) == 1) {
        show_dialog_box(
            "Hello!",
            "Collect all '+' to win!"
        );
    }
}

void endGame_check() {
    if (score >= totalCoins) {
        clear_screen_fast();
        system("cls");
        printf("YOU WIN! Score %d/%d\n", score, totalCoins);
        gameOver = 1;
    }
}

int read_keyboard() {
    int ch = _getch();

    // hỗ trợ arrow keys (optional)
    if (ch == 0 || ch == 224) {
        int ch2 = _getch();
        switch (ch2) {
            case 72: return 'w'; // up
            case 80: return 's'; // down
            case 75: return 'a'; // left
            case 77: return 'd'; // right
        }
        return 0;
    }

    return tolower(ch);
}

void gamerun(){
    while (!gameOver) {
        build_frame();
        print_board();

        int key = read_keyboard();
        if (key == 'q') {
            gameOver = 1;
            break;
        }

        switch (key) {
            case 'w': try_move(0, -1); break;
            case 's': try_move(0, 1);  break;
            case 'a': try_move(-1, 0); break;
            case 'd': try_move(1, 0);  break;
            case 'e': interact();      break;
        }

        endGame_check();
    }
}

int main() {
    srand((unsigned)time(NULL));

    fill_base_board();
    place_coins_random(TOTAL_COINS);
    gamerun();

    printf("\nGame Over.\n");
    return 0;
}