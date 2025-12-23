/*
 * Tetris Game - Piece implementation
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#include "Piece.h"
#include "Game.h"
#include <cstdlib>
#include <algorithm>

// Shuffle the 7-piece bag using Fisher-Yates algorithm
void shuffleBag() {
    for (int i = 6; i > 0; i--) {
        int j = rand() % (i + 1);
        std::swap(pieceBag[i], pieceBag[j]);
    }
    bagIndex = 0;
}

// 7-Bag Random: ensures fair distribution of all 7 pieces
Piece* createRandomPiece() {
    // Refill bag when empty
    if (bagIndex >= 7) {
        shuffleBag();
    }
    
    int pieceType = pieceBag[bagIndex++];
    
    switch (pieceType) {
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
