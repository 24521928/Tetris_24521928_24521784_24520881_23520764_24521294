#include "Game.h"
#include "Audio.h"
#include "UI.h"
#include <fstream>

// --- GLOBAL GAME STATE ---
char board[H][W] = {};
int x = 4, y = 0;
float gameDelay = 0.8f;
float baseDelay = 0.8f;
bool isGameOver = false;

// --- SCORE & LEVEL ---
int gScore = 0;
int gLines = 0;
int gLevel = 0;
int currentLevel = 0;
int highScore = 0;

// --- COMBO SYSTEM ---
int comboCount = 0;
int lastClearLines = 0;

// --- T-SPIN & BACK-TO-BACK SYSTEM ---
bool lastMoveWasRotate = false;
bool backToBackActive = false;
int tSpinCount = 0;

// --- STATISTICS ---
float playTime = 0.f;
int tetrisCount = 0;
int totalPieces = 0;
int pieceCount[7] = {0, 0, 0, 0, 0, 0, 0};  // I, O, T, S, Z, J, L

// Helper to get piece index from char
int getPieceIndex(char c) {
    switch (c) {
        case 'I': return 0;
        case 'O': return 1;
        case 'T': return 2;
        case 'S': return 3;
        case 'Z': return 4;
        case 'J': return 5;
        case 'L': return 6;
        default: return -1;
    }
}

// --- CURRENT PIECES ---
Piece* currentPiece = nullptr;
Piece* nextPiece = nullptr;
Piece* nextQueue[4] = {nullptr, nullptr, nullptr, nullptr};
Piece* holdPiece = nullptr;
bool canHold = true;

// --- DIFFICULTY ---
Difficulty difficulty = Difficulty::NORMAL;

// --- 7-BAG RANDOM SYSTEM ---
int pieceBag[7] = {0, 1, 2, 3, 4, 5, 6};
int bagIndex = 7; // Force refill on first call

// --- DAS & ARR (Auto-Repeat System) ---
float dasTimer = 0.f;
float arrTimer = 0.f;
bool leftHeld = false;
bool rightHeld = false;
bool downHeld = false;

// --- LOCK DELAY & INFINITY ---
float lockTimer = 0.f;
int lockMoves = 0;
bool onGround = false;

// --- SETTINGS ---
float musicVolume = 50.f;
float sfxVolume = 50.f;
float brightness = 255.f;
bool ghostPieceEnabled = true;

float getBaseDelayForDifficulty() {
    switch (difficulty) {
        case Difficulty::EASY:   return 1.0f;
        case Difficulty::NORMAL: return 0.8f;
        case Difficulty::HARD:   return 0.5f;
        default: return 0.8f;
    }
}

void loadHighScore() {
    std::ifstream file("highscore.dat");
    if (file.is_open()) {
        file >> highScore;
        file.close();
    }
}

void saveHighScore() {
    if (gScore > highScore) {
        highScore = gScore;
        std::ofstream file("highscore.dat");
        if (file.is_open()) {
            file << highScore;
            file.close();
        }
    }
}

void loadSettings() {
    std::ifstream file("config.ini");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.find("musicVolume=") == 0) {
                musicVolume = std::stof(line.substr(12));
            } else if (line.find("sfxVolume=") == 0) {
                sfxVolume = std::stof(line.substr(10));
            } else if (line.find("brightness=") == 0) {
                brightness = std::stof(line.substr(11));
            } else if (line.find("ghostPiece=") == 0) {
                ghostPieceEnabled = (line.substr(11) == "1");
            } else if (line.find("difficulty=") == 0) {
                int diff = std::stoi(line.substr(11));
                difficulty = static_cast<Difficulty>(diff);
            }
        }
        file.close();
    }
}

void saveSettings() {
    std::ofstream file("config.ini");
    if (file.is_open()) {
        file << "musicVolume=" << musicVolume << "\n";
        file << "sfxVolume=" << sfxVolume << "\n";
        file << "brightness=" << brightness << "\n";
        file << "ghostPiece=" << (ghostPieceEnabled ? 1 : 0) << "\n";
        file << "difficulty=" << static_cast<int>(difficulty) << "\n";
        file.close();
    }
}

void initBoard() {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            if ((i == H - 1) || (j == 0) || (j == W - 1)) {
                board[i][j] = '#';
            } else {
                board[i][j] = ' ';
            }
        }
    }
}

void block2Board() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] != ' ') {
                board[y + i][x + j] = currentPiece->shape[i][j];
            }
        }
    }
}

bool canMove(int dx, int dy) {
    if (!currentPiece) return false;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] != ' ') {
                int tx = x + j + dx;
                int ty = y + i + dy;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return false;
                if (board[ty][tx] != ' ') return false;
            }
        }
    }
    return true;
}

int getGhostY() {
    int ghostY = y;
    while (true) {
        bool canGo = true;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentPiece->shape[i][j] != ' ') {
                    int tx = x + j;
                    int ty = ghostY + i + 1;
                    if (ty >= H - 1 || board[ty][tx] != ' ') {
                        canGo = false;
                        break;
                    }
                }
            }
            if (!canGo) break;
        }
        if (!canGo) break;
        ghostY++;
    }
    return ghostY;
}

void SpeedIncrement() {
    if (gameDelay > 0.1f) {
        gameDelay -= 0.08f;
    }
}

void applyLineClearScore(int cleared) {
    if (cleared <= 0) {
        comboCount = 0;
        // Reset B2B if no difficult clear
        if (!isTSpin()) {
            backToBackActive = false;
        }
        return;
    }
    
    gLines += cleared;
    lastClearLines = cleared;
    
    // Check for T-Spin
    bool tSpin = isTSpin();
    if (tSpin) tSpinCount++;
    
    // Check for Perfect Clear
    bool perfectClear = isPerfectClear();
    
    // Base score with combo multiplier
    float comboMultiplier = 1.0f + (comboCount * 0.5f);
    
    // Score based on lines cleared
    int baseScore = 0;
    std::string clearType = "";
    
    if (tSpin) {
        // T-Spin scoring
        switch (cleared) {
            case 1: baseScore = 800; clearType = "T-SPIN SINGLE"; break;
            case 2: baseScore = 1200; clearType = "T-SPIN DOUBLE"; break;
            case 3: baseScore = 1600; clearType = "T-SPIN TRIPLE"; break;
            default: baseScore = 800;
        }
    } else {
        // Normal scoring
        switch (cleared) {
            case 1: baseScore = 100; break;
            case 2: baseScore = 300; break;
            case 3: baseScore = 500; break;
            case 4: baseScore = 800; tetrisCount++; clearType = "TETRIS"; break;
            default: baseScore = 100 * cleared;
        }
    }
    
    // Back-to-Back bonus (50% extra for Tetris or T-Spin after another)
    float b2bMultiplier = 1.0f;
    if ((cleared == 4 || tSpin) && backToBackActive) {
        b2bMultiplier = 1.5f;
        clearType += " B2B";
    }
    
    // Activate B2B for next difficult clear
    if (cleared == 4 || tSpin) {
        backToBackActive = true;
    } else {
        backToBackActive = false;
    }
    
    // Perfect Clear bonus
    if (perfectClear) {
        baseScore += 3000;
        clearType += " PERFECT!";
    }
    
    gScore += static_cast<int>(baseScore * comboMultiplier * b2bMultiplier);
    comboCount++;
    
    gLevel = gLines / 10;
    if (gLevel > currentLevel) {
        SpeedIncrement();
        currentLevel = gLevel;
        Audio::playLevelUp();
    }
}

int removeLine() {
    int cleared = 0;
    int clearedLines[4] = {-1, -1, -1, -1};
    
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') { isFull = false; break; }
        }
        if (isFull) {
            if (cleared < 4) {
                clearedLines[cleared] = i;
            }
            cleared++;
            Audio::playClear();
            
            // Add particles for line clear
            for (int j = 1; j < W - 1; j++) {
                sf::Color color = getColor(board[i][j]);
                UI::addParticles(STATS_W + j * TILE_SIZE + TILE_SIZE/2, 
                                i * TILE_SIZE + TILE_SIZE/2, color, 5);
            }
            
            for (int k = i; k > 0; k--) {
                for (int j = 1; j < W - 1; j++) {
                    board[k][j] = (k != 1) ? board[k - 1][j] : ' ';
                }
            }
            i++;
        }
    }
    
    // Start line clear animation if lines were cleared
    if (cleared > 0) {
        UI::startLineClearAnim(clearedLines, cleared);
    }
    
    return cleared;
}

void resetGame() {
    initBoard();
    delete currentPiece;
    delete nextPiece;
    for (int i = 0; i < 4; i++) {
        if (nextQueue[i]) {
            delete nextQueue[i];
            nextQueue[i] = nullptr;
        }
    }
    if (holdPiece) {
        delete holdPiece;
        holdPiece = nullptr;
    }
    currentPiece = createRandomPiece();
    nextPiece = createRandomPiece();
    for (int i = 0; i < 4; i++) {
        nextQueue[i] = createRandomPiece();
    }
    x = 4;
    y = 0;
    baseDelay = getBaseDelayForDifficulty();
    gameDelay = baseDelay;
    isGameOver = false;
    gScore = 0;
    gLines = 0;
    gLevel = 0;
    currentLevel = 0;
    comboCount = 0;
    lastClearLines = 0;
    playTime = 0.f;
    tetrisCount = 0;
    totalPieces = 0;
    canHold = true;
    for (int i = 0; i < 7; i++) pieceCount[i] = 0;
    dasTimer = 0.f;
    arrTimer = 0.f;
    leftHeld = false;
    rightHeld = false;
    downHeld = false;
    lockTimer = 0.f;
    lockMoves = 0;
    onGround = false;
    lastMoveWasRotate = false;
    backToBackActive = false;
    tSpinCount = 0;
}

void swapHold() {
    if (!canHold) return;
    
    if (holdPiece == nullptr) {
        holdPiece = currentPiece;
        currentPiece = nextPiece;
        nextPiece = nextQueue[0];
        // Shift queue and add new piece
        for (int i = 0; i < 3; i++) {
            nextQueue[i] = nextQueue[i + 1];
        }
        nextQueue[3] = createRandomPiece();
    } else {
        Piece* temp = holdPiece;
        holdPiece = currentPiece;
        currentPiece = temp;
    }
    
    x = 4;
    y = 0;
    canHold = false;
}

void resetLockDelay() {
    if (lockMoves < MAX_LOCK_MOVES) {
        lockTimer = 0.f;
        lockMoves++;
    }
}

// T-Spin detection: T piece, last move was rotate, at least 3 corners filled
bool isTSpin() {
    if (!currentPiece || !lastMoveWasRotate) return false;
    
    // Check if it's T piece
    bool isTpiece = false;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] == 'T') {
                isTpiece = true;
                break;
            }
        }
        if (isTpiece) break;
    }
    if (!isTpiece) return false;
    
    // Find T piece center position in shape matrix
    int centerR = -1, centerC = -1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] == 'T') {
                // Check if this is center (has 3 adjacent T blocks)
                int adjacent = 0;
                if (i > 0 && currentPiece->shape[i-1][j] == 'T') adjacent++;
                if (i < 3 && currentPiece->shape[i+1][j] == 'T') adjacent++;
                if (j > 0 && currentPiece->shape[i][j-1] == 'T') adjacent++;
                if (j < 3 && currentPiece->shape[i][j+1] == 'T') adjacent++;
                if (adjacent == 3) {
                    centerR = i;
                    centerC = j;
                    break;
                }
            }
        }
        if (centerR != -1) break;
    }
    
    if (centerR == -1) return false;
    
    // Check 4 corners around center in board space
    int boardY = y + centerR;
    int boardX = x + centerC;
    
    int filledCorners = 0;
    // Top-left, Top-right, Bottom-left, Bottom-right
    if (boardY > 0 && boardX > 0 && board[boardY-1][boardX-1] != ' ') filledCorners++;
    if (boardY > 0 && boardX < W-1 && board[boardY-1][boardX+1] != ' ') filledCorners++;
    if (boardY < H-1 && boardX > 0 && board[boardY+1][boardX-1] != ' ') filledCorners++;
    if (boardY < H-1 && boardX < W-1 && board[boardY+1][boardX+1] != ' ') filledCorners++;
    
    return filledCorners >= 3;
}

// Perfect Clear: entire board is empty (except borders)
bool isPerfectClear() {
    for (int i = 1; i < H - 1; i++) {
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] != ' ') return false;
        }
    }
    return true;
}
