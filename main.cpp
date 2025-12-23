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

/** Process main */
int main() {
    srand(static_cast<unsigned>(time(nullptr)));
    
    loadHighScore();
    loadSettings();
    
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    bool isFullscreen = false;
    RenderWindow window(VideoMode({WINDOW_W, WINDOW_H}), "TETRIS", sf::Style::Default, sf::State::Windowed, settings);
    window.setFramerateLimit(60);
    
/** Process window */
    sf::View gameView(sf::FloatRect({0, 0}, {(float)WINDOW_W, (float)WINDOW_H}));
    window.setView(gameView);

    sf::Image icon;
    if (icon.loadFromFile("assets/logo.png")) {
        window.setIcon(icon);
    }

    Font font;
    if (!font.openFromFile("assets/fonts/Monocraft.ttf")) return -1;

    if (!Audio::init()) return -1;

    initBoard();
    currentPiece = createRandomPiece();
    nextPiece = createRandomPiece();
    for (int i = 0; i < 4; i++) {
        nextQueue[i] = createRandomPiece();
    }

    GameState state = GameState::MENU;
    GameState previousState = GameState::MENU;
    Clock timer;
    Clock frameClock;
    SidebarUI sidebarUI = UI::makeSidebarUI();
    bool shouldClose = false;
    bool blockInput = false;

    const float fieldOffsetX = STATS_W;

    const float fullW = WINDOW_W;
    const float goBtnW = 280.f;
    const float goBtnX = (fullW - goBtnW) / 2.f;

/** Play background music */
    Audio::playMusic();

    while (window.isOpen() && !shouldClose) {
        float dt = frameClock.restart().asSeconds();
        
        while (auto eventOpt = window.pollEvent()) {
            Event& event = *eventOpt;

            if (event.is<Event::Closed>()) {
                window.close();
            }
            
            if (auto* resized = event.getIf<Event::Resized>()) {
                float windowW = static_cast<float>(resized->size.x);
                float windowH = static_cast<float>(resized->size.y);
                
                float scaleX = windowW / WINDOW_W;
                float scaleY = windowH / WINDOW_H;
                float scale = std::min(scaleX, scaleY);
                
                float viewWidth = WINDOW_W * scale;
                float viewHeight = WINDOW_H * scale;
                float viewX = (windowW - viewWidth) / 2.f;
                float viewY = (windowH - viewHeight) / 2.f;
                
                gameView.setViewport(sf::FloatRect(
                    {viewX / windowW, viewY / windowH},
                    {viewWidth / windowW, viewHeight / windowH}
                ));
                window.setView(gameView);
            }
            
            if (auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::F11) {
                    isFullscreen = !isFullscreen;
                    window.close();
                    
                    if (isFullscreen) {
                        auto desktop = VideoMode::getDesktopMode();
                        window.create(desktop, "TETRIS", sf::Style::Default, sf::State::Fullscreen, settings);
                        
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
                    
                    if (icon.getSize().x > 0) {
                        window.setIcon(icon);
                    }
                    timer.restart();
                }
            }

            if (event.is<Event::MouseButtonPressed>()) {
                Vector2i pixelPos = Mouse::getPosition(window);
                Vector2f mousePos = window.mapPixelToCoords(pixelPos);

                if (state == GameState::MENU) {
                    UI::handleMenuClick(Vector2i(mousePos), state, previousState, shouldClose);
                    if (state == GameState::PLAYING) {
                        timer.restart();
                    }
                }
                else if (state == GameState::PAUSED) {
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 320 && mousePos.y <= 385) {
                        state = GameState::PLAYING;
                        timer.restart();
                    }
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 405 && mousePos.y <= 470) {
                        previousState = GameState::PAUSED;
                        state = GameState::HOWTOPLAY;
                    }
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 490 && mousePos.y <= 555) {
                        Audio::playOpenSettings();
                        previousState = GameState::PAUSED;
                        state = GameState::SETTINGS;
                    }
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 575 && mousePos.y <= 640) {
                        saveHighScore();
                        state = GameState::MENU;
                    }
                }
                else if (state == GameState::PLAYING && isGameOver) {
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 340 && mousePos.y <= 405) {
                        saveHighScore();
                        resetGame();
                        timer.restart();
                    }
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 430 && mousePos.y <= 495) {
                        saveHighScore();
/** Process restart */
                        Audio::playTheme();
                        state = GameState::MENU;
                    }
                    if (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW &&
                        mousePos.y >= 520 && mousePos.y <= 585) {
                        saveHighScore();
                        window.close();
                    }
                }
                else if (state == GameState::SETTINGS) {
/** Save high score to file */
                    UI::handleSettingsClick(Vector2i(mousePos));
                    
                    const float backBtnW = 280.f;
                    const float backBtnX = (fullW - backBtnW) / 2.f;
                    if (mousePos.x >= backBtnX && mousePos.x <= backBtnX + backBtnW &&
                        mousePos.y >= 700 && mousePos.y <= 765) {
                        Audio::playCloseSettings();
                        saveSettings();
                        state = previousState;
                        if (previousState == GameState::PLAYING || previousState == GameState::PAUSED) {
                            timer.restart();
                        }
                    }
                }
                else if (state == GameState::HOWTOPLAY) {
/** Process restart */
                    UI::handleHowToPlayClick(Vector2i(mousePos), state, previousState);
                }
            }

            if (state == GameState::PLAYING && !isGameOver) {
                if (auto* key = event.getIf<Event::KeyPressed>()) {
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
                                gScore += 1;
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
                        
                        if (key->code == Keyboard::Key::Space) {
                            int dropDist = getGhostY() - y;
                            y = getGhostY();
                            gScore += dropDist * 2;
                            blockInput = true;
                        }
                    }
                    
                    if (key->code == Keyboard::Key::C) {
                        swapHold();
                        blockInput = false;
                    }
                    
                    if (key->code == Keyboard::Key::P || key->code == Keyboard::Key::Escape) {
                        state = GameState::PAUSED;
                    }
                }
                
                if (auto* key = event.getIf<Event::KeyReleased>()) {
                    if (key->code == Keyboard::Key::Left) leftHeld = false;
                    if (key->code == Keyboard::Key::Right) rightHeld = false;
                    if (key->code == Keyboard::Key::Down) downHeld = false;
                }
            }
            else if (state == GameState::PAUSED) {
                if (auto* key = event.getIf<Event::KeyPressed>()) {
                    if (key->code == Keyboard::Key::P || key->code == Keyboard::Key::Escape) {
                        state = GameState::PLAYING;
                        timer.restart();
                    }
                }
            }
        }

        if (state == GameState::PLAYING && !isGameOver) {
            playTime += dt;
            UI::updateLineClearAnim(dt);
            UI::updateParticles(dt);
            
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
            
            bool canMoveDown = canMove(0, 1);
            if (!canMoveDown) {
                if (!onGround) {
                    onGround = true;
                    lockTimer = 0.f;
                    lockMoves = 0;
                }
                lockTimer += dt;
                
                if (lockTimer >= LOCK_DELAY || lockMoves >= MAX_LOCK_MOVES) {
                    blockInput = false;
                    
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
/** Transfer current piece to the game board */
                    Audio::playLand();
                    totalPieces++;
                    
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
                        Audio::stopTheme();
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

        window.clear(Color::Black);

        if (state == GameState::MENU) {
/** Process clear */
            UI::drawMenu(window, font);
        }
        else if (state == GameState::PLAYING || state == GameState::PAUSED) {
            UI::drawPieceStats(window, font);
            
            for (int i = 0; i < H; i++) {
                for (int j = 0; j < W; j++) {
                    UI::drawTile3D(window, fieldOffsetX + (float)(j * TILE_SIZE), (float)(i * TILE_SIZE), 
                                   TILE_SIZE, board[i][j]);
                }
            }

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

            if (currentPiece) {
                UI::drawSoftDropTrail(window, currentPiece, x, y, downHeld && canMove(0, 1));
                
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (currentPiece->shape[i][j] != ' ') {
/** Render softdroptrail */
                            UI::drawTile3D(window, fieldOffsetX + (float)((x + j) * TILE_SIZE), 
                                          (float)((y + i) * TILE_SIZE), 
                                          TILE_SIZE, currentPiece->shape[i][j]);
                        }
                    }
                }
            }

/** Render particles */
            UI::drawParticles(window);
            
/** Render particles */
            UI::drawLineClearAnim(window);
            
/** Render particles */
            UI::drawCombo(window, font);

/** Render lineclearanim */
            UI::drawSidebar(window, sidebarUI, font, gScore, gLevel, gLines, nextPiece, nextQueue, holdPiece);

            if (isGameOver) {
/** Render combo */
                UI::drawGameOverScreen(window, font);
            }
            
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

        UI::drawBrightnessOverlay(window);

        bool onButton = false;
        Vector2i pixelPos = Mouse::getPosition(window);
        Vector2f mousePos = window.mapPixelToCoords(pixelPos);
        
        if (state == GameState::MENU) {
            const float btnW = 280.f;
            const float btnX = (fullW - btnW) / 2.f;
            float diffBtnW = 100.f;
            float diffStartX = (fullW - 3 * diffBtnW - 24.f) / 2.f;
            for (int i = 0; i < 3; i++) {
                float bx = diffStartX + i * (diffBtnW + 12.f);
                if (mousePos.x >= bx && mousePos.x <= bx + diffBtnW &&
                    mousePos.y >= 270 && mousePos.y <= 315) {
                    onButton = true;
                }
            }
            if ((mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 360 && mousePos.y <= 425) ||
                (mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 445 && mousePos.y <= 510) ||
                (mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 530 && mousePos.y <= 595) ||
                (mousePos.x >= btnX && mousePos.x <= btnX + btnW && mousePos.y >= 615 && mousePos.y <= 680)) {
                onButton = true;
            }
        }
        else if (state == GameState::HOWTOPLAY) {
            const float btnW = 280.f;
            const float btnH = 65.f;
            const float btnX = (fullW - btnW) / 2.f;
            const float btnY = WINDOW_H - 90.f;
            if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
                mousePos.y >= btnY && mousePos.y <= btnY + btnH) {
                onButton = true;
            }
        }
        else if (state == GameState::SETTINGS) {
            const float backBtnW = 280.f;
            const float backBtnX = (fullW - backBtnW) / 2.f;
            const float arrowLeftX = 320.f;
            const float sliderX = 350.f;
            const float sliderW = 280.f;
            const float arrowRightX = 640.f;
            const float checkboxX = 330.f;
            const float row1Y = 140.f;
            const float row2Y = 225.f;
            const float row3Y = 310.f;
            const float dasSliderY = 413.f;
            const float arrSliderY = 498.f;
            const float row6Y = 565.f;
            
            bool onArrow = (mousePos.x >= arrowLeftX && mousePos.x <= arrowLeftX + 30 && mousePos.y >= row1Y - 10 && mousePos.y <= row1Y + 40) ||
                           (mousePos.x >= arrowRightX && mousePos.x <= arrowRightX + 30 && mousePos.y >= row1Y - 10 && mousePos.y <= row1Y + 40) ||
                           (mousePos.x >= arrowLeftX && mousePos.x <= arrowLeftX + 30 && mousePos.y >= row2Y - 10 && mousePos.y <= row2Y + 40) ||
                           (mousePos.x >= arrowRightX && mousePos.x <= arrowRightX + 30 && mousePos.y >= row2Y - 10 && mousePos.y <= row2Y + 40) ||
                           (mousePos.x >= arrowLeftX && mousePos.x <= arrowLeftX + 30 && mousePos.y >= row3Y - 10 && mousePos.y <= row3Y + 40) ||
                           (mousePos.x >= arrowRightX && mousePos.x <= arrowRightX + 30 && mousePos.y >= row3Y - 10 && mousePos.y <= row3Y + 40) ||
                           (mousePos.x >= arrowLeftX && mousePos.x <= arrowLeftX + 30 && mousePos.y >= dasSliderY - 10 && mousePos.y <= dasSliderY + 40) ||
                           (mousePos.x >= arrowRightX && mousePos.x <= arrowRightX + 30 && mousePos.y >= dasSliderY - 10 && mousePos.y <= dasSliderY + 40) ||
                           (mousePos.x >= arrowLeftX && mousePos.x <= arrowLeftX + 30 && mousePos.y >= arrSliderY - 10 && mousePos.y <= arrSliderY + 40) ||
                           (mousePos.x >= arrowRightX && mousePos.x <= arrowRightX + 30 && mousePos.y >= arrSliderY - 10 && mousePos.y <= arrSliderY + 40);
            bool onSlider = (mousePos.x >= sliderX && mousePos.x <= sliderX + sliderW && mousePos.y >= row1Y && mousePos.y <= row1Y + 35) ||
                            (mousePos.x >= sliderX && mousePos.x <= sliderX + sliderW && mousePos.y >= row2Y && mousePos.y <= row2Y + 35) ||
                            (mousePos.x >= sliderX && mousePos.x <= sliderX + sliderW && mousePos.y >= row3Y && mousePos.y <= row3Y + 35) ||
                            (mousePos.x >= sliderX && mousePos.x <= sliderX + sliderW && mousePos.y >= dasSliderY && mousePos.y <= dasSliderY + 35) ||
                            (mousePos.x >= sliderX && mousePos.x <= sliderX + sliderW && mousePos.y >= arrSliderY && mousePos.y <= arrSliderY + 35);
            bool onCheckbox = (mousePos.x >= checkboxX && mousePos.x <= checkboxX + 35 && mousePos.y >= row6Y - 5 && mousePos.y <= row6Y + 40);
            bool onBackBtn = (mousePos.x >= backBtnX && mousePos.x <= backBtnX + backBtnW &&
                              mousePos.y >= 700 && mousePos.y <= 765);
            if (onArrow || onSlider || onCheckbox || onBackBtn) {
                onButton = true;
            }
        }
        else if (state == GameState::PAUSED) {
            if ((mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 320 && mousePos.y <= 385) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 405 && mousePos.y <= 470) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 490 && mousePos.y <= 555) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 575 && mousePos.y <= 640)) {
                onButton = true;
            }
        }
        else if (state == GameState::PLAYING && isGameOver) {
            if ((mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 340 && mousePos.y <= 405) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 430 && mousePos.y <= 495) ||
                (mousePos.x >= goBtnX && mousePos.x <= goBtnX + goBtnW && mousePos.y >= 520 && mousePos.y <= 585)) {
                onButton = true;
            }
        }
        
        window.setMouseCursor(onButton ? Cursor(Cursor::Type::Hand) : Cursor(Cursor::Type::Arrow));

        window.display();
    }

    saveHighScore();
    saveSettings();
/** Process display */
    Audio::cleanup();
    delete currentPiece;
    delete nextPiece;
    for (int i = 0; i < 4; i++) {
        if (nextQueue[i]) delete nextQueue[i];
    }
    if (holdPiece) delete holdPiece;

    return 0;
}
