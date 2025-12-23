/*
 * Tetris Game - Piece system with 7-bag randomizer
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#pragma once
#include <SFML/Graphics.hpp>
#include "Config.h"

extern char board[H][W];


// Game state management
extern int x;



// Types and structures
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

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[j][3 - i] = shape[i][j];
            }
        }

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

class IPiece : public Piece {
public:
    IPiece() {
        shape[1][0] = 'I';
        shape[1][1] = 'I';
        shape[1][2] = 'I';
        shape[1][3] = 'I';
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
    int rotationState = 0;

    TPiece() {
        shape[0][1] = 'T';
        shape[1][0] = 'T'; shape[1][1] = 'T'; shape[1][2] = 'T';
    }

    void rotate(int currentX, int currentY) override {
        char temp[4][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[i][j] = ' ';
            }
        }

        int nextState = (rotationState + 1) % 4;

        switch (nextState) {
            case 0:
                temp[0][1] = 'T';
                temp[1][0] = 'T'; temp[1][1] = 'T'; temp[1][2] = 'T';
                break;
            case 1:
                temp[0][1] = 'T';
                temp[1][1] = 'T'; temp[1][2] = 'T';
                temp[2][1] = 'T';
                break;
            case 2:
                temp[1][0] = 'T'; temp[1][1] = 'T'; temp[1][2] = 'T';
                temp[2][1] = 'T';
                break;
            case 3:
                temp[0][1] = 'T';
                temp[1][0] = 'T'; temp[1][1] = 'T';
                temp[2][1] = 'T';
                break;
        }

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
                rotationState = nextState;
                return;
            }
        }
    }
};

class SPiece : public Piece {
public:
    SPiece() {
        shape[0][1] = 'S'; shape[0][2] = 'S';
        shape[1][0] = 'S'; shape[1][1] = 'S';
    }
};

class ZPiece : public Piece {
public:
    ZPiece() {
        shape[0][0] = 'Z'; shape[0][1] = 'Z';
        shape[1][1] = 'Z'; shape[1][2] = 'Z';
    }
};

class JPiece : public Piece {
public:
    JPiece() {
        shape[0][0] = 'J';
        shape[1][0] = 'J'; shape[1][1] = 'J'; shape[1][2] = 'J';
    }
};

class LPiece : public Piece {
public:
    LPiece() {
        shape[0][2] = 'L';
        shape[1][0] = 'L'; shape[1][1] = 'L'; shape[1][2] = 'L';
    }
};

inline sf::Color getColor(char c) {
    switch (c) {
        case 'I': return sf::Color(0, 240, 240);
        case 'J': return sf::Color(80, 120, 255);
        case 'L': return sf::Color(240, 160, 0);
        case 'O': return sf::Color(240, 240, 0);
        case 'S': return sf::Color(0, 240, 0);
        case 'T': return sf::Color(160, 0, 240);
        case 'Z': return sf::Color(240, 0, 0);
        case '#': return sf::Color(60, 60, 80);
        default:  return sf::Color(20, 20, 30);
    }
}

inline sf::Color getHighlightColor(char c) {
    switch (c) {
        case 'I': return sf::Color(150, 255, 255);
        case 'J': return sf::Color(150, 180, 255);
        case 'L': return sf::Color(255, 200, 100);
        case 'O': return sf::Color(255, 255, 150);
        case 'S': return sf::Color(150, 255, 150);
        case 'T': return sf::Color(200, 100, 255);
        case 'Z': return sf::Color(255, 100, 100);
        case '#': return sf::Color(100, 100, 120);
        default:  return sf::Color(40, 40, 50);
    }
}

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

void shuffleBag();
Piece* createRandomPiece();
