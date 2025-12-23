/*
 * Tetris Game - UI system
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#pragma once
#include <SFML/Graphics.hpp>
#include "Config.h"
#include "Piece.h"



// Types and structures
struct SidebarUI {
    float x, y, w, h;
    float pad;
    float boxW;
    sf::FloatRect topScoreBox;
    sf::FloatRect scoreBox;
    sf::FloatRect levelBox;
    sf::FloatRect linesBox;
    sf::FloatRect nextBox;
    sf::FloatRect holdBox;
    sf::FloatRect statsBox;
};

struct LineClearAnim {
    bool active = false;
    float timer = 0.f;
    int lines[4] = {-1, -1, -1, -1};
    int count = 0;
};

extern LineClearAnim lineClearAnim;

namespace UI {


// UI functions
    void drawTile3D(sf::RenderWindow& window, float px, float py, float size, char c);

    void drawPieceStats(sf::RenderWindow& window, const sf::Font& font);

    SidebarUI makeSidebarUI();
    void drawSidebar(sf::RenderWindow& window, const SidebarUI& ui,
                     const sf::Font& font, int score, int level, int lines,
                     const Piece* next, Piece* const nextQueue[], const Piece* hold);

    void drawSettingsScreen(sf::RenderWindow& window, const sf::Font& font);
    void handleSettingsClick(sf::Vector2i mousePos);

    void drawMenu(sf::RenderWindow& window, const sf::Font& font);


// Game functions
    void handleMenuClick(sf::Vector2i mousePos, GameState& state, GameState& previousState, bool& shouldClose);

    void drawPauseScreen(sf::RenderWindow& window, const sf::Font& font);

    void drawGameOverScreen(sf::RenderWindow& window, const sf::Font& font);

    void drawHowToPlay(sf::RenderWindow& window, const sf::Font& font);
    void handleHowToPlayClick(sf::Vector2i mousePos, GameState& state, GameState& previousState);

    void drawBrightnessOverlay(sf::RenderWindow& window);

    void startLineClearAnim(int* clearedLines, int count);
    void updateLineClearAnim(float dt);
    void drawLineClearAnim(sf::RenderWindow& window);

    struct Particle {
        float x, y;
        float vx, vy;
        float life;
        sf::Color color;
    };

    void addParticles(float x, float y, sf::Color color, int count);
    void updateParticles(float dt);
    void drawParticles(sf::RenderWindow& window);

    void drawSoftDropTrail(sf::RenderWindow& window, const Piece* piece, int px, int py, bool isActive);

    void drawCombo(sf::RenderWindow& window, const sf::Font& font);
}
