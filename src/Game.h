#pragma once
#include "Config.h"
#include "Piece.h"

// --- GLOBAL GAME STATE ---
extern char board[H][W];
extern int x, y;
extern float gameDelay;
extern float baseDelay;
extern bool isGameOver;

// --- SCORE & LEVEL ---
extern int gScore;
extern int gLines;
extern int gLevel;
extern int currentLevel;
extern int highScore;

// --- COMBO SYSTEM ---
extern int comboCount;
extern int lastClearLines;

// --- T-SPIN & BACK-TO-BACK SYSTEM ---
extern bool lastMoveWasRotate;
extern bool backToBackActive;
extern int tSpinCount;

// --- STATISTICS ---
extern float playTime;
extern int tetrisCount;
extern int totalPieces;
extern int pieceCount[7];  // I, O, T, S, Z, J, L

// --- CURRENT PIECES ---
extern Piece* currentPiece;
extern Piece* nextPiece;
extern Piece* nextQueue[4];  // Show 5 total (nextPiece + 4 more)
extern Piece* holdPiece;
extern bool canHold;

// --- DIFFICULTY ---
extern Difficulty difficulty;

// --- 7-BAG RANDOM SYSTEM ---
extern int pieceBag[7];
extern int bagIndex;

// --- DAS & ARR (Auto-Repeat System) ---
extern float dasTimer;
extern float arrTimer;
extern bool leftHeld;
extern bool rightHeld;
extern bool downHeld;
const float DAS_DELAY = 0.133f;  // 133ms before auto-repeat starts
const float ARR_DELAY = 0.0f;    // 0ms = instant repeat

// --- LOCK DELAY & INFINITY ---
extern float lockTimer;
extern int lockMoves;
extern bool onGround;
const float LOCK_DELAY = 0.5f;   // 500ms lock delay
const int MAX_LOCK_MOVES = 15;   // Max moves before force lock

// --- SETTINGS ---
extern float musicVolume;
extern float sfxVolume;
extern float brightness;
extern bool ghostPieceEnabled;

// --- GAME FUNCTIONS ---
void initBoard();
void block2Board();
bool canMove(int dx, int dy);
int getGhostY();
void SpeedIncrement();
void applyLineClearScore(int cleared);
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
