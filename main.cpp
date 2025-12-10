#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <ctime>

using namespace std;

#define H 20
#define W 15

char board[H][W] = {};
int x = 4, y = 0;
int speed = 400;

class Piece {
public:
    char shape[4][4];

    Piece() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                shape[i][j] = ' ';
    }

    virtual ~Piece() {}

    virtual void rotate(int currentX, int currentY) {
        char temp[4][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[j][3 - i] = shape[i][j];
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (temp[i][j] != ' ') {
                    int tx = currentX + j;
                    int ty = currentY + i;
                    if (tx < 1 || tx >= W - 1 || ty >= H - 1) return; 
                    if (board[ty][tx] != ' ') return; 
                }
            }
        }

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                shape[i][j] = temp[i][j];
            }
        }
    }
};

class IPiece : public Piece {
public:
    IPiece() {
        shape[0][1] = 'I'; shape[1][1] = 'I'; shape[2][1] = 'I'; shape[3][1] = 'I';
    }
};

class OPiece : public Piece {
public:
    OPiece() {
        shape[1][1] = 'O'; shape[1][2] = 'O';
        shape[2][1] = 'O'; shape[2][2] = 'O';
    }
    void rotate(int currentX, int currentY) override {
    }
};

class TPiece : public Piece {
public:
    TPiece() {
        shape[1][1] = 'T';
        shape[2][0] = 'T'; shape[2][1] = 'T'; shape[2][2] = 'T';
    }
};

class SPiece : public Piece {
public:
    SPiece() {
        shape[1][1] = 'S'; shape[1][2] = 'S';
        shape[2][0] = 'S'; shape[2][1] = 'S';
    }
};

class ZPiece : public Piece {
public:
    ZPiece() {
        shape[1][0] = 'Z'; shape[1][1] = 'Z';
        shape[2][1] = 'Z'; shape[2][2] = 'Z';
    }
};

class JPiece : public Piece {
public:
    JPiece() {
        shape[1][0] = 'J';
        shape[2][0] = 'J'; shape[2][1] = 'J'; shape[2][2] = 'J';
    }
};

class LPiece : public Piece {
public:
    LPiece() {
        shape[1][2] = 'L';
        shape[2][0] = 'L'; shape[2][1] = 'L'; shape[2][2] = 'L';
    }
};

Piece* currentPiece = nullptr;

Piece* createRandomPiece() {
    int r = rand() % 7;
    switch (r) {
        case 0: return new IPiece();
        case 1: return new OPiece();
        case 2: return new TPiece();
        case 3: return new SPiece();
        case 4: return new ZPiece();
        case 5: return new JPiece();
        case 6: return new LPiece();
        default: return new IPiece();
    }
}

void gotoxy(int x, int y) {
    COORD c = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void boardDelBlock() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (currentPiece->shape[i][j] != ' ' && y + j < H)
                board[y + i][x + j] = ' ';
}

void block2Board() {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (currentPiece->shape[i][j] != ' ')
                board[y + i][x + j] = currentPiece->shape[i][j];
}

void initBoard() {
    for (int i = 0; i < H; i++)
        for (int j = 0; j < W; j++)
            if ((i == H - 1) || (j == 0) || (j == W - 1)) board[i][j] = '#';
            else board[i][j] = ' ';
}

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void draw() {
    gotoxy(0, 0);
    for (int i = 0; i < H; i++, cout << endl) {
        for (int j = 0; j < W; j++) {
            if (board[i][j] == ' ') {
                cout << "  ";
            }
            else if (board[i][j] == '#') {
                setColor(8);
                cout << "\xB2\xB2";
                setColor(7);
            }
            else {
                switch (board[i][j]) {
                case 'I': setColor(11); break;
                case 'J': setColor(9);  break;
                case 'L': setColor(6);  break;
                case 'O': setColor(14); break;
                case 'S': setColor(10); break;
                case 'T': setColor(13); break;
                case 'Z': setColor(12); break;
                default:  setColor(7);  break;
                }
                cout << "[]";
                setColor(7);
            }
        }
    }
}

bool canMove(int dx, int dy) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (currentPiece->shape[i][j] != ' ') {
                int tx = x + j + dx;
                int ty = y + i + dy;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return false;
                if (board[ty][tx] != ' ') return false;
            }
    return true;
}
void SpeedIncrement()
{
    speed = max(100, speed - 30);
}
void removeLine(){
    for(int i = H - 2; i > 0; i--){
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') {
                isFull = false;
            }
        }
        if (isFull) {
            for (int k = i; k > 0; k--) {
                for (int j = 1; j < W - 1; j++) {
                    if (k != 1) {
                        board[k][j] = board[k - 1][j];
                    }
                    else {
                        board[k][j] = ' ';
                    }
                }
            }
            //recheck: whether the new line is full
            i++;
            
            SpeedIncrement();
        }   
    }
}

int main()
{
    srand(time(0));
    
    currentPiece = createRandomPiece();

    system("cls");
    initBoard();
    while (1) {
        boardDelBlock();
        if (kbhit()){
            char c = getch();
            if (c == 'a' && canMove(-1, 0)) x--;
            if (c == 'd' && canMove(1, 0)) x++;
            if (c == 'x' && canMove(0, 1)) y++;
            if (c == 'w') currentPiece->rotate(x, y);
            if (c == 'q') break;
        }
        if (canMove(0, 1)) y++;
        else {
            block2Board();
            removeLine();
            x = 5; y = 0;
            delete currentPiece; 
            currentPiece = createRandomPiece();
        }
        block2Board();
        draw();
        Sleep(speed);
    }
    return 0;
}