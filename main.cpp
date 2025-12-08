#include <iostream>
#include <conio.h>
#include <windows.h>
using namespace std;
#define H 20
#define W 15
char board[H][W] = {} ;
char blocks[][4][4] = {
        {{' ','I',' ',' '},
         {' ','I',' ',' '},
         {' ','I',' ',' '},
         {' ','I',' ',' '}},
        {{' ','I',' ',' '},
         {' ','I',' ',' '},
         {' ','I',' ',' '},
         {' ','I',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {'I','I','I','I'},
         {' ',' ',' ',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','O','O',' '},
         {' ','O','O',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','T',' ',' '},
         {'T','T','T',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ','S','S',' '},
         {'S','S',' ',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {'Z','Z',' ',' '},
         {' ','Z','Z',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {'J',' ',' ',' '},
         {'J','J','J',' '},
         {' ',' ',' ',' '}},
        {{' ',' ',' ',' '},
         {' ',' ','L',' '},
         {'L','L','L',' '},
         {' ',' ',' ',' '}}
};

int x=4,y=0,b=1;
int speed = 200;    
void gotoxy(int x, int y) {
    COORD c = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
void boardDelBlock(){
    for (int i = 0 ; i < 4 ; i++)
        for (int j = 0 ; j < 4 ; j++)
            if (blocks[b][i][j] != ' ' && y+j < H)
                board[y+i][x+j] = ' ';
}
void block2Board(){
    for (int i = 0 ; i < 4 ; i++)
        for (int j = 0 ; j < 4 ; j++)
            if (blocks[b][i][j] != ' ' )
                board[y+i][x+j] = blocks[b][i][j];
}
void initBoard(){
    for (int i = 0 ; i < H ; i++)
        for (int j = 0 ; j < W ; j++)
            if ((i==H-1) || (j==0) || (j == W-1)) board[i][j] = '#';
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
                    case 'I': setColor(11); break; // Cyan
                    case 'J': setColor(9);  break; // Blue
                    case 'L': setColor(6);  break; // Orange brown
                    case 'O': setColor(14); break; // Yellow
                    case 'S': setColor(10); break; // Green
                    case 'T': setColor(13); break; // Purple
                    case 'Z': setColor(12); break; // Red
                    default:  setColor(7);  break; // White
                }
                cout << "[]";

                setColor(7); 
            }
        }
    }
}
bool canMove(int dx, int dy){
    for (int i = 0 ; i < 4 ; i++)
        for (int j = 0 ; j < 4 ; j++)
            if (blocks[b][i][j] != ' '){
                int tx = x + j + dx;
                int ty = y + i + dy;
                if ( tx<1 || tx >= W-1 || ty >= H-1) return false;
                if ( board[ty][tx] != ' ') return false;
            }
    return true;
}
void SpeedIncrement()
{
    speed = max(100, speed - 5);    //  max speed boundary 
}
void removeLine(){
    for(int i = H - 2; i > 0; i--){
        bool isFull = true;
        for(int j = 1; j < W-1; j++){
            if(board[i][j] == ' '){
                isFull = false;
                break;
            }
        }
        if(isFull){
            for(int k = i ; k > 0 ; k--){
                for(int j = 1; j < W-1; j++){
                    // if the line is the first one => it only remove that line
                    if(k != 1){
                        board[k][j] = board[k-1][j];
                    }
                    else{
                        board[k][j] = ' ';
                    }
                }
            }
        }
        //recheck: whether the new line is full
        i++;
    }
    SpeedIncrement();
}

void rotateBlock() {
    char temp[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            temp[j][3 - i] = blocks[b][i][j];
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (temp[i][j] != ' ') {
                int tx = x + j;
                int ty = y + i;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return; 
                if (board[ty][tx] != ' ') return; 
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            blocks[b][i][j] = temp[i][j];
        }
    }
}

int main()
{
    srand(time(0));
    b = rand() % 7;
    system("cls");
    initBoard();
    while (1){
        boardDelBlock();
        if (kbhit()){
            char c = getch();
            if (c=='a' && canMove(-1,0)) x--;
            if (c=='d' && canMove(1,0) ) x++;
            if (c=='x' && canMove(0,1))  y++;
            if (c=='w') rotateBlock();
            if (c=='q') break;
        }
        if (canMove(0,1)) y++;
        else {
            block2Board();
            removeLine();
            x = 5; y = 0; b = rand() % 7;
        }
        block2Board();
        draw();
        Sleep(speed);
    }
    return 0;
}
