#pragma once
#include <SFML/Graphics.hpp>
#include "Config.h"

// Forward declarations
extern char board[H][W];
extern int x;

// --- BASE PIECE CLASS ---
class Piece {
public:
    char shape[4][4];

    Piece() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                shape[i][j] = ' ';
            }
        }
    }

    virtual ~Piece() {}

    virtual void rotate(int currentX, int currentY) {
        char temp[4][4];

        // 1. Rotate to temp matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[j][3 - i] = shape[i][j];
            }
        }

        // 2. Try wall kicks: original, left 1, right 1, left 2, right 2
        int kicks[] = {0, -1, 1, -2, 2};
        for (int kick : kicks) {
            bool canPlace = true;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (temp[i][j] != ' ') {
                        int tx = currentX + j + kick;
                        int ty = currentY + i;
                        
                        if (tx < 1 || tx >= W - 1 || ty >= H - 1) {
                            canPlace = false;
                            break;
                        }
                        if (board[ty][tx] != ' ') {
                            canPlace = false;
                            break;
                        }
                    }
                }
                if (!canPlace) break;
            }
            
            if (canPlace) {
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        shape[i][j] = temp[i][j];
                    }
                }
                x += kick;
                return;
            }
        }
    }
};

// --- PIECE TYPES ---
class IPiece : public Piece {
public:
    IPiece() {
        shape[0][1] = 'I';
        shape[1][1] = 'I';
        shape[2][1] = 'I';
        shape[3][1] = 'I';
    }
};

class OPiece : public Piece {
public:
    OPiece() {
        shape[1][1] = 'O'; shape[1][2] = 'O';
        shape[2][1] = 'O'; shape[2][2] = 'O';
    }
    void rotate(int, int) override {}
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

// --- UTILITY FUNCTIONS ---
// NES-style vibrant colors
inline sf::Color getColor(char c) {
    switch (c) {
        case 'I': return sf::Color(0, 240, 240);      // Cyan
        case 'J': return sf::Color(0, 0, 240);        // Blue
        case 'L': return sf::Color(240, 160, 0);      // Orange
        case 'O': return sf::Color(240, 240, 0);      // Yellow
        case 'S': return sf::Color(0, 240, 0);        // Green
        case 'T': return sf::Color(160, 0, 240);      // Purple
        case 'Z': return sf::Color(240, 0, 0);        // Red
        case '#': return sf::Color(60, 60, 80);       // Border - darker blue-gray
        default:  return sf::Color(20, 20, 30);       // Background - dark
    }
}

// Lighter highlight color for 3D effect
inline sf::Color getHighlightColor(char c) {
    switch (c) {
        case 'I': return sf::Color(150, 255, 255);
        case 'J': return sf::Color(100, 100, 255);
        case 'L': return sf::Color(255, 200, 100);
        case 'O': return sf::Color(255, 255, 150);
        case 'S': return sf::Color(150, 255, 150);
        case 'T': return sf::Color(200, 100, 255);
        case 'Z': return sf::Color(255, 100, 100);
        case '#': return sf::Color(100, 100, 120);
        default:  return sf::Color(40, 40, 50);
    }
}

// Darker shadow color for 3D effect
inline sf::Color getShadowColor(char c) {
    switch (c) {
        case 'I': return sf::Color(0, 160, 160);
        case 'J': return sf::Color(0, 0, 160);
        case 'L': return sf::Color(180, 100, 0);
        case 'O': return sf::Color(180, 180, 0);
        case 'S': return sf::Color(0, 160, 0);
        case 'T': return sf::Color(100, 0, 160);
        case 'Z': return sf::Color(160, 0, 0);
        case '#': return sf::Color(30, 30, 50);
        default:  return sf::Color(10, 10, 20);
    }
}

// 7-Bag Random Generator (fair piece distribution)
void shuffleBag();
Piece* createRandomPiece();
