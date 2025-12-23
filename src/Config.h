#pragma once

// --- GAME CONFIGURATION ---
const int TILE_SIZE = 30;
const int H = 22;        // Board height (20 playable + 2 border)
const int W = 12;        // Board width (10 playable + 2 border)
const int STATS_W = 120; // Left panel width for statistics
const int SIDEBAR_W = 180; // Right sidebar width
const int PLAY_W_PX = W * TILE_SIZE;
const int PLAY_H_PX = H * TILE_SIZE;
const int WINDOW_W = STATS_W + PLAY_W_PX + SIDEBAR_W;
const int WINDOW_H = PLAY_H_PX;

// --- GAME STATE ---
enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    SETTINGS,
    HOWTOPLAY,
};

// --- DIFFICULTY ---
enum class Difficulty {
    EASY,
    NORMAL,
    HARD
};
