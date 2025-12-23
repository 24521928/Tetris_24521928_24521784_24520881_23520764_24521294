/*
 * Tetris Game - Modern implementation with advanced mechanics
 * Copyright (C) 2025 Tetris Game Contributors
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <SFML/Graphics.hpp>
#include <ctime>
#include "src/Config.h"
#include "src/Piece.h"
#include "src/Game.h"
#include "src/Audio.h"
#include "src/UI.h"

using namespace sf;

int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    // --- LOAD HIGH SCORE & SETTINGS ---
    loadHighScore();
    loadSettings();
    
    // --- WINDOW SETUP ---
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;  // Smooth edges (SFML 3 syntax)
    bool isFullscreen = false;
    RenderWindow window(VideoMode({WINDOW_W, WINDOW_H}), "TETRIS", sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(60);
    
    // Setup view for proper scaling
    sf::View gameView(sf::FloatRect({0, 0}, {(float)WINDOW_W, (float)WINDOW_H}));
    window.setView(gameView);

    // --- SET WINDOW ICON ---
    sf::Image icon;
    if (icon.loadFromFile("assets/logo.png")) {
        window.setIcon(icon);
    }

    // --- LOAD FONT ---
    Font font;
    if (!font.openFromFile("assets/fonts/Monocraft.ttf")) return -1;

    // --- INITIALIZE AUDIO ---
    if (!Audio::init()) return -1;

    // --- INITIALIZE GAME ---
    initBoard();
    currentPiece = createRandomPiece();
    nextPiece = createRandomPiece();
    for (int i = 0; i < 4; i++) {
        nextQueue[i] = createRandomPiece();
    }

    // --- GAME STATE ---
    GameState state = GameState::MENU;
    GameState previousState = GameState::MENU;  // Track previous state for Settings return
    Clock timer;
    Clock frameClock;
    SidebarUI sidebarUI = UI::makeSidebarUI();
    bool shouldClose = false;
    bool blockInput = false;

    // --- GAME FIELD OFFSET (after stats panel) ---
    const float fieldOffsetX = STATS_W;

    // --- GAME OVER UI POSITION ---
    const float fullW = WINDOW_W;
    const float goBtnW = 200.f;
    const float goBtnX = (fullW - goBtnW) / 2.f;

    // --- START MUSIC ---
    Audio::playMusic();

    // ================ GAME LOOP ================
    while (window.isOpen() && !shouldClose) {
        float dt = frameClock.restart().asSeconds();
        
        // --- EVENT HANDLING ---
        while (auto eventOpt = window.pollEvent()) {
            Event& event = *eventOpt;

            if (event.is<Event::Closed>()) {
                window.close();
            }
            
            // Toggle fullscreen with F11
            if (auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::F11) {
                    isFullscreen = !isFullscreen;
                    window.close();
                    
                    if (isFullscreen) {
                        // Get desktop resolution
                        auto desktop = VideoMode::getDesktopMode();
                        window.create(desktop, "TETRIS", sf::Style::Default, sf::State::Fullscreen, settings);
                        
                        // Calculate view to maintain aspect ratio (letterbox/pillarbox)
                        float scaleX = (float)desktop.size.x / WINDOW_W;
                        float scaleY = (float)desktop.size.y / WINDOW_H;
                        float scale = std::min(scaleX, scaleY);
                        
                        float viewWidth = WINDOW_W * scale;
                        float viewHeight = WINDOW_H * scale;
                        float viewX = (desktop.size.x - viewWidth) / 2.f;
                        float viewY = (desktop.size.y - viewHeight) / 2.f;
                        
                        gameView.setViewport(sf::FloatRect(
                            {viewX / desktop.size.x, viewY / desktop.size.y},
                            {viewWidth / desktop.size.x, viewHeight / desktop.size.y}
                        ));
                    } else {
                        window.create(VideoMode({WINDOW_W, WINDOW_H}), "TETRIS", sf::Style::Default, sf::State::Windowed, settings);
                        gameView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
                    }
                    
                    window.setFramerateLimit(60);
                    window.setView(gameView);
                    
                    // Restore icon
                    if (icon.getSize().x > 0) {
                        window.setIcon(icon);
                    }
                    timer.restart();
                }
            }

            if (event.is<Event::MouseButtonPressed>()) {
                Vector2i mousePos = Mouse::getPosition(window);

                // --- MENU CLICKS ---
                if (state == GameState::MENU) {
                    UI::handleMenuClick(mousePos, state, previousState, shouldClose);
                    if (state == GameState::PLAYING) {
                        timer.restart();
                    }
                }
                // --- PAUSE CLICKS ---
                else if (state == GameState::PAUSED) {
                    // Resume button
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 310 && mousePos.y <= 360) {
                        state = GameState::PLAYING;
                        timer.restart();
                    }
                    // Settings button
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 380 && mousePos.y <= 430) {
                        Audio::playOpenSettings();
                        previousState = GameState::PAUSED;  // Remember we came from pause
                        state = GameState::SETTINGS;
                    }
                    // Menu button
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 450 && mousePos.y <= 500) {
                        saveHighScore();
                        state = GameState::MENU;
                    }
                }
                // --- GAME OVER CLICKS ---
                else if (state == GameState::PLAYING && isGameOver) {
                    // RESTART button
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 230 && mousePos.y <= 280) {
                        saveHighScore();
                        resetGame();
                        timer.restart();
                    }
                    // MENU button
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 300 && mousePos.y <= 350) {
                        saveHighScore();
                        state = GameState::MENU;
                    }
                    // EXIT button
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 370 && mousePos.y <= 420) {
                        saveHighScore();
                        window.close();
                    }
                }
                // --- SETTINGS CLICKS ---
                else if (state == GameState::SETTINGS) {
                    UI::handleSettingsClick(mousePos);
                    
                    // BACK button
                    const float backBtnW = 200.f;
                    const float backBtnX = (fullW - backBtnW) / 2.f;
                    if (mousePos.x >= backBtnX && mousePos.x <= backBtnX + backBtnW &&
                        mousePos.y >= 380 && mousePos.y <= 430) {
                        Audio::playCloseSettings();
                        saveSettings();
                        state = previousState;  // Return to where we came from
                        if (previousState == GameState::PLAYING || previousState == GameState::PAUSED) {
                            timer.restart();
                        }
                    }
                }
                // --- HOW TO PLAY CLICKS ---
                else if (state == GameState::HOWTOPLAY) {
                    UI::handleHowToPlayClick(mousePos, state);
                }
            }

            // --- PLAYING INPUT ---
            if (state == GameState::PLAYING && !isGameOver) {
                if (auto* key = event.getIf<Event::KeyPressed>()) {
                    // Block input after hard drop until piece locks
                    if (!blockInput) {
                        if (key->code == Keyboard::Key::Left) {
                            if (canMove(-1, 0)) {
                                x--;
                                if (onGround) resetLockDelay();
                            }
                            leftHeld = true;
                            dasTimer = 0.f;
                            arrTimer = 0.f;
                            lastMoveWasRotate = false;
                        }
                        if (key->code == Keyboard::Key::Right) {
                            if (canMove(1, 0)) {
                                x++;
                                if (onGround) resetLockDelay();
                            }
                            rightHeld = true;
                            dasTimer = 0.f;
                            arrTimer = 0.f;
                            lastMoveWasRotate = false;
                        }
                        if (key->code == Keyboard::Key::Down) {
                            if (canMove(0, 1)) {
                                y++;
                                gScore += 1; // Soft drop scoring
                            }
                            downHeld = true;
                            dasTimer = 0.f;
                            arrTimer = 0.f;
                            lastMoveWasRotate = false;
                        }
                        if (key->code == Keyboard::Key::Up && currentPiece) {
                            currentPiece->rotate(x, y);
                            if (onGround) resetLockDelay();
                            lastMoveWasRotate = true;
                        }
                        
                        // Hard drop with scoring
                        if (key->code == Keyboard::Key::Space) {
                            int dropDist = getGhostY() - y;
                            y = getGhostY();
                            gScore += dropDist * 2; // Hard drop scoring
                            blockInput = true; // Block further input until piece locks
                        }
                    }
                    
                    // Hold piece (allowed even during blockInput)
                    if (key->code == Keyboard::Key::C) {
                        swapHold();
                        blockInput = false; // Reset after hold
                    }
                    
                    // Pause
                    if (key->code == Keyboard::Key::P || key->code == Keyboard::Key::Escape) {
                        state = GameState::PAUSED;
                    }
                }
                
                // Key release events
                if (auto* key = event.getIf<Event::KeyReleased>()) {
                    if (key->code == Keyboard::Key::Left) leftHeld = false;
                    if (key->code == Keyboard::Key::Right) rightHeld = false;
                    if (key->code == Keyboard::Key::Down) downHeld = false;
                }
            }
            // Unpause
            else if (state == GameState::PAUSED) {
                if (auto* key = event.getIf<Event::KeyPressed>()) {
                    if (key->code == Keyboard::Key::P || key->code == Keyboard::Key::Escape) {
                        state = GameState::PLAYING;
                        timer.restart();
                    }
                }
            }
        }

        // --- PLAYING LOGIC ---
        if (state == GameState::PLAYING && !isGameOver) {
            playTime += dt;
            UI::updateLineClearAnim(dt);
            UI::updateParticles(dt);
            
            // DAS & ARR auto-repeat logic
            if (!blockInput) {
                if (leftHeld || rightHeld || downHeld) {
                    dasTimer += dt;
                    if (dasTimer >= DAS_DELAY) {
                        arrTimer += dt;
                        if (arrTimer >= ARR_DELAY) {
                            if (leftHeld && canMove(-1, 0)) {
                                x--;
                                if (onGround) resetLockDelay();
                                lastMoveWasRotate = false;
                            }
                            if (rightHeld && canMove(1, 0)) {
                                x++;
                                if (onGround) resetLockDelay();
                                lastMoveWasRotate = false;
                            }
                            if (downHeld && canMove(0, 1)) {
                                y++;
                                gScore += 1;
                                lastMoveWasRotate = false;
                            }
                            arrTimer = 0.f;
                        }
                    }
                }
            }
            
            // Lock delay system
            bool canMoveDown = canMove(0, 1);
            if (!canMoveDown) {
                if (!onGround) {
                    onGround = true;
                    lockTimer = 0.f;
                    lockMoves = 0;
                }
                lockTimer += dt;
                
                // Force lock after delay or max moves
                if (lockTimer >= LOCK_DELAY || lockMoves >= MAX_LOCK_MOVES) {
                    blockInput = false;
                    
                    // Add particles on lock
                    if (currentPiece) {
                        for (int i = 0; i < 4; i++) {
                            for (int j = 0; j < 4; j++) {
                                if (currentPiece->shape[i][j] != ' ') {
                                    sf::Color c = getColor(currentPiece->shape[i][j]);
                                    UI::addParticles(fieldOffsetX + (x + j) * TILE_SIZE + TILE_SIZE/2,
                                                    (y + i) * TILE_SIZE + TILE_SIZE/2, c, 3);
                                }
                            }
                        }
                    }
                    
                    block2Board();
                    Audio::playLand();
                    totalPieces++;
                    
                    // Count piece type
                    if (currentPiece) {
                        for (int i = 0; i < 4; i++) {
                            for (int j = 0; j < 4; j++) {
                                if (currentPiece->shape[i][j] != ' ') {
                                    int idx = getPieceIndex(currentPiece->shape[i][j]);
                                    if (idx >= 0) pieceCount[idx]++;
                                    goto counted;
                                }
                            }
                        }
                        counted:;
                    }
                    
                    int cleared = removeLine();
                    applyLineClearScore(cleared);
                    
                    delete currentPiece;
                    currentPiece = nextPiece;
                    nextPiece = nextQueue[0];
                    // Shift queue and add new piece at end
                    for (int i = 0; i < 3; i++) {
                        nextQueue[i] = nextQueue[i + 1];
                    }
                    nextQueue[3] = createRandomPiece();
                    
                    x = 4;
                    y = 0;
                    canHold = true;
                    onGround = false;
                    lockTimer = 0.f;
                    lockMoves = 0;
                    
                    if (!canMove(0, 0)) {
                        isGameOver = true;
                        saveHighScore();
                        Audio::playGameOver();
                    }
                }
            } else {
                onGround = false;
                lockTimer = 0.f;
            }
            
            if (timer.getElapsedTime().asSeconds() >= gameDelay) {
                if (canMoveDown) {
                    y++;
                }
                timer.restart();
            }
        }

        // ================ RENDERING ================
        window.clear(Color::Black);

        if (state == GameState::MENU) {
            UI::drawMenu(window, font);
        }
        else if (state == GameState::PLAYING || state == GameState::PAUSED) {
            // Draw piece statistics panel (left side)
            UI::drawPieceStats(window, font);
            
            // Draw game board with 3D tiles
            for (int i = 0; i < H; i++) {
                for (int j = 0; j < W; j++) {
                    UI::drawTile3D(window, fieldOffsetX + (float)(j * TILE_SIZE), (float)(i * TILE_SIZE), 
                                   TILE_SIZE, board[i][j]);
                }
            }

            // Draw ghost piece
            if (currentPiece && ghostPieceEnabled && state == GameState::PLAYING) {
                int ghostY = getGhostY();
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (currentPiece->shape[i][j] != ' ') {
                            RectangleShape ghost({TILE_SIZE - 1.f, TILE_SIZE - 1.f});
                            ghost.setPosition({fieldOffsetX + (float)((x + j) * TILE_SIZE), (float)((ghostY + i) * TILE_SIZE)});
                            Color c = getColor(currentPiece->shape[i][j]);
                            c.a = 60;
                            ghost.setFillColor(c);
                            ghost.setOutlineThickness(1.f);
                            ghost.setOutlineColor(Color(c.r, c.g, c.b, 120));
                            window.draw(ghost);
                        }
                    }
                }
            }

            // Draw current piece with 3D effect
            if (currentPiece) {
                // Draw soft drop trail
                UI::drawSoftDropTrail(window, currentPiece, x, y, downHeld);
                
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (currentPiece->shape[i][j] != ' ') {
                            UI::drawTile3D(window, fieldOffsetX + (float)((x + j) * TILE_SIZE), 
                                          (float)((y + i) * TILE_SIZE), 
                                          TILE_SIZE, currentPiece->shape[i][j]);
                        }
                    }
                }
            }

            // Draw particles
            UI::drawParticles(window);
            
            // Draw line clear animation
            UI::drawLineClearAnim(window);
            
            // Draw combo
            UI::drawCombo(window, font);

            // Draw sidebar
            UI::drawSidebar(window, sidebarUI, font, gScore, gLevel, gLines, nextPiece, nextQueue, holdPiece);

            // Draw game over screen
            if (isGameOver) {
                UI::drawGameOverScreen(window, font);
            }
            
            // Draw pause screen
            if (state == GameState::PAUSED) {
                UI::drawPauseScreen(window, font);
            }
        }
        else if (state == GameState::SETTINGS) {
            UI::drawSettingsScreen(window, font);
        }
        else if (state == GameState::HOWTOPLAY) {
            UI::drawHowToPlay(window, font);
        }

        // Draw brightness overlay
        UI::drawBrightnessOverlay(window);

        // Update cursor
        bool onButton = false;
        Vector2i mousePos = Mouse::getPosition(window);
        
        if (state == GameState::MENU) {
            const float btnW = 200.f;
            const float btnX = (fullW - btnW) / 2.f;
            // Difficulty buttons
            float diffBtnW = 80.f;
            float diffStartX = (fullW - 3 * diffBtnW - 20.f) / 2.f;
            for (int i = 0; i < 3; i++) {
                float bx = diffStartX + i * (diffBtnW + 10.f);
                if (mousePos.x >= bx && mousePos.x <= bx + diffBtnW &&
                    mousePos.y >= 210 && mousePos.y <= 245) {
                    onButton = true;
                }
            }
            // Main buttons (START, SETTINGS, HOW TO PLAY, EXIT)
            if ((mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 280 && mousePos.y <= 325) ||
                (mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 340 && mousePos.y <= 385) ||
                (mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 400 && mousePos.y <= 445) ||
                (mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 460 && mousePos.y <= 505)) {
                onButton = true;
            }
        }
        else if (state == GameState::HOWTOPLAY) {
            const float btnW = 150.f;
            const float btnX = (fullW - btnW) / 2.f;
            const float btnY = WINDOW_H - 60.f;
            if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
                mousePos.y >= btnY && mousePos.y <= btnY + 40.f) {
                onButton = true;
            }
        }
        else if (state == GameState::SETTINGS) {
            const float backBtnW = 200.f;
            const float backBtnX = (fullW - backBtnW) / 2.f;
            bool onArrow = (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 125 && mousePos.y <= 155) ||
                           (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 125 && mousePos.y <= 155) ||
                           (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 185 && mousePos.y <= 215) ||
                           (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 185 && mousePos.y <= 215) ||
                           (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 245 && mousePos.y <= 275) ||
                           (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 245 && mousePos.y <= 275);
            bool onSlider = (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 127 && mousePos.y <= 157) ||
                            (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 187 && mousePos.y <= 217) ||
                            (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 247 && mousePos.y <= 277);
            bool onCheckbox = (mousePos.x >= 280 && mousePos.x <= 315 && mousePos.y >= 303 && mousePos.y <= 337);
            bool onBackBtn = (mousePos.x >= backBtnX && mousePos.x <= backBtnX + backBtnW &&
                              mousePos.y >= 380 && mousePos.y <= 430);
            if (onArrow || onSlider || onCheckbox || onBackBtn) {
                onButton = true;
            }
        }
        else if (state == GameState::PAUSED) {
            if ((mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 330 && mousePos.y <= 380) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 400 && mousePos.y <= 450)) {
                onButton = true;
            }
        }
        else if (state == GameState::PLAYING && isGameOver) {
            if ((mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 230 && mousePos.y <= 280) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 300 && mousePos.y <= 350) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 370 && mousePos.y <= 420)) {
                onButton = true;
            }
        }
        
        window.setMouseCursor(onButton ? Cursor(Cursor::Type::Hand) : Cursor(Cursor::Type::Arrow));

        window.display();
    }

    // --- CLEANUP ---
    saveHighScore();
    saveSettings();
    Audio::cleanup();
    delete currentPiece;
    delete nextPiece;
    for (int i = 0; i < 4; i++) {
        if (nextQueue[i]) delete nextQueue[i];
    }
    if (holdPiece) delete holdPiece;

    return 0;
}
