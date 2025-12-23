/*
 * Tetris Game - Core game logic
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#pragma once
#include "Config.h"
#include "Piece.h"

extern char board[H][W];


// Game state management
extern int x, y;
extern float gameDelay;
extern float baseDelay;
extern bool isGameOver;

extern int gScore;
extern int gLines;
extern int gLevel;
extern int currentLevel;
extern int highScore;

extern int comboCount;
extern int lastClearLines;

extern bool lastMoveWasRotate;
extern bool backToBackActive;
extern int tSpinCount;

extern float playTime;
extern int tetrisCount;
extern int totalPieces;
extern int pieceCount[7];

extern Piece* currentPiece;
extern Piece* nextPiece;
extern Piece* nextQueue[4];
extern Piece* holdPiece;
extern bool canHold;

extern Difficulty difficulty;

extern int pieceBag[7];
extern int bagIndex;

extern float dasTimer;
extern float arrTimer;
extern bool leftHeld;
extern bool rightHeld;
extern bool downHeld;
extern float DAS_DELAY;
extern float ARR_DELAY;

extern float lockTimer;
extern int lockMoves;
extern bool onGround;
const float LOCK_DELAY = 0.5f;
const int MAX_LOCK_MOVES = 15;

extern float musicVolume;
extern float sfxVolume;
extern float brightness;
extern bool ghostPieceEnabled;

void initBoard();
void block2Board();
bool canMove(int dx, int dy);
int getGhostY();
void SpeedIncrement();
void applyLineClearScore(int cleared);


// Game functions
int removeLine();
void resetGame();
void swapHold();
void loadHighScore();
void saveHighScore();
void loadSettings();
void saveSettings();
float getBaseDelayForDifficulty();
int getPieceIndex(char c);
void resetLockDelay();
bool isTSpin();
bool isPerfectClear();
