/*
 * Tetris Game - Configuration constants
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#pragma once

const int TILE_SIZE = 36;
const int H = 22;
const int W = 12;
const int STATS_W = 140;
const int SIDEBAR_W = 220;
const int PLAY_W_PX = W * TILE_SIZE;
const int PLAY_H_PX = H * TILE_SIZE;
const int WINDOW_W = 800;
const int WINDOW_H = 800;

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    SETTINGS,
    HOWTOPLAY,
};

enum class Difficulty {
    EASY,
    NORMAL,
    HARD
};
