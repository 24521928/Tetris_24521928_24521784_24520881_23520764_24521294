/*
 * Tetris Game - UI implementation
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#include "UI.h"
#include "Game.h"
#include "Audio.h"
#include <algorithm>
#include <string>
#include <vector>
#include <cmath>

using namespace sf;

// --- HELPER FUNCTIONS ---
static void drawPanel(sf::RenderWindow& window, const sf::FloatRect& r) {
    const float outline = 3.f;
    const float inset = outline;
    sf::RectangleShape box({ r.size.x - 2.f*inset, r.size.y - 2.f*inset });
    box.setPosition({ r.position.x + inset, r.position.y + inset });
    box.setFillColor(sf::Color(15, 15, 25));
    box.setOutlineThickness(outline);
    box.setOutlineColor(sf::Color(80, 80, 120));
    window.draw(box);
}

namespace UI {

// Line clear animation state
LineClearAnim lineClearAnim;

// Particle system
std::vector<Particle> particles;

// Draw a single tile with 3D effect
void drawTile3D(sf::RenderWindow& window, float px, float py, float size, char c) {
    if (c == ' ') {
        // Empty cell - dark background
        sf::RectangleShape bg({size - 1.f, size - 1.f});
        bg.setPosition({px, py});
        bg.setFillColor(sf::Color(20, 20, 30));
        window.draw(bg);
        return;
    }
    
    const float bevel = 3.f;
    sf::Color main = getColor(c);
    sf::Color highlight = getHighlightColor(c);
    sf::Color shadow = getShadowColor(c);
    
    // Main body
    sf::RectangleShape body({size - 1.f, size - 1.f});
    body.setPosition({px, py});
    body.setFillColor(main);
    window.draw(body);
    
    // Top highlight
    sf::RectangleShape top({size - 1.f, bevel});
    top.setPosition({px, py});
    top.setFillColor(highlight);
    window.draw(top);
    
    // Left highlight
    sf::RectangleShape left({bevel, size - 1.f});
    left.setPosition({px, py});
    left.setFillColor(highlight);
    window.draw(left);
    
    // Bottom shadow
    sf::RectangleShape bottom({size - 1.f, bevel});
    bottom.setPosition({px, py + size - 1.f - bevel});
    bottom.setFillColor(shadow);
    window.draw(bottom);
    
    // Right shadow
    sf::RectangleShape right({bevel, size - 1.f});
    right.setPosition({px + size - 1.f - bevel, py});
    right.setFillColor(shadow);
    window.draw(right);
    
    // Inner shine (small highlight in center-top)
    sf::RectangleShape shine({size * 0.3f, size * 0.15f});
    shine.setPosition({px + size * 0.2f, py + size * 0.15f});
    sf::Color shineColor = highlight;
    shineColor.a = 100;
    shine.setFillColor(shineColor);
    window.draw(shine);
}

// Draw piece statistics panel (NES style - left side)
void drawPieceStats(sf::RenderWindow& window, const sf::Font& font) {
    // Panel on the left side of the window
    const char pieces[7] = {'I', 'O', 'T', 'S', 'Z', 'J', 'L'};
    const int shapes[7][4][4] = {
        // I
        {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}},
        // O
        {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
        // T
        {{0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0}},
        // S
        {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
        // Z
        {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},
        // J
        {{0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0}},
        // L
        {{0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0}}
    };
    
    // Draw in left panel area
    float panelX = 5.f;
    float panelY = 10.f;
    float panelW = STATS_W - 10.f;
    float panelH = WINDOW_H - 20.f;
    
    // Panel background
    sf::RectangleShape bg({panelW, panelH});
    bg.setPosition({panelX, panelY});
    bg.setFillColor(sf::Color(15, 15, 25));
    bg.setOutlineThickness(3.f);
    bg.setOutlineColor(sf::Color(80, 80, 120));
    window.draw(bg);
    
    // Title
    sf::Text title(font, "STATISTICS", 16);
    title.setFillColor(sf::Color(100, 200, 255));
    float titleW = title.getLocalBounds().size.x;
    title.setPosition({panelX + (panelW - titleW) / 2.f, panelY + 10.f});
    window.draw(title);
    
    // Draw each piece with count
    float rowY = panelY + 45.f;
    int mini = 10;  // Mini tile size
    float rowH = (panelH - 60.f) / 7.f;  // Divide remaining space by 7 pieces
    
    for (int p = 0; p < 7; p++) {
        // Draw mini piece preview (centered)
        float pieceX = panelX + 12.f;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (shapes[p][r][c]) {
                    sf::RectangleShape tile({(float)mini - 1, (float)mini - 1});
                    tile.setPosition({pieceX + c * mini, rowY + r * mini});
                    tile.setFillColor(getColor(pieces[p]));
                    window.draw(tile);
                }
            }
        }
        
        // Draw count (3 digits, right-aligned)
        char countStr[8];
        snprintf(countStr, 8, "%03d", pieceCount[p]);
        sf::Text countText(font, countStr, 18);
        countText.setFillColor(getColor(pieces[p]));
        countText.setPosition({panelX + 60.f, rowY + 12.f});
        window.draw(countText);
        
        rowY += rowH;
    }
}

SidebarUI makeSidebarUI() {
    SidebarUI ui{};
    ui.x = (float)(STATS_W + PLAY_W_PX);
    ui.y = 0.f;
    ui.w = (float)SIDEBAR_W;
    ui.h = (float)WINDOW_H;
    ui.pad  = 10.f;
    ui.boxW = ui.w - 2.f * ui.pad;

    const float leftColW = 75.f;   // Narrow left column
    const float rightColW = ui.boxW - leftColW - 8.f;  // Right column for NEXT
    const float left = ui.x + ui.pad;
    const float rightX = left + leftColW + 8.f;

    // Left column: HOLD, SCORE, LEVEL, LINES stacked vertically (narrow)
    ui.holdBox  = sf::FloatRect({left, 10.f},   {leftColW, 95.f});
    ui.scoreBox = sf::FloatRect({left, 113.f},  {leftColW, 72.f});
    ui.levelBox = sf::FloatRect({left, 193.f},  {leftColW, 72.f});
    ui.linesBox = sf::FloatRect({left, 273.f},  {leftColW, 72.f});
    
    // Right column: NEXT with 5 pieces running vertically
    ui.nextBox  = sf::FloatRect({rightX, 10.f}, {rightColW, 335.f});
    
    // GAME INFO full width at bottom
    ui.statsBox = sf::FloatRect({left, 353.f},  {ui.boxW, 295.f});
    return ui;
}

static void drawHoldPreview(sf::RenderWindow& window, const SidebarUI& ui, const Piece* p) {
    if (!p) return;
    int minR = 4, minC = 4, maxR = -1, maxC = -1;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (p->shape[r][c] != ' ') {
                minR = std::min(minR, r); minC = std::min(minC, c);
                maxR = std::max(maxR, r); maxC = std::max(maxC, c);
            }
        }
    }
    if (maxR == -1) return;

    int cellsW = maxC - minC + 1;
    int cellsH = maxR - minR + 1;
    int mini = 11;  // Slightly larger tiles

    float startX = ui.holdBox.position.x + (ui.holdBox.size.x - cellsW * mini) * 0.5f;
    float startY = ui.holdBox.position.y + 42.f + (ui.holdBox.size.y - 52.f - cellsH * mini) * 0.5f;

    for (int r = minR; r <= maxR; r++) {
        for (int c = minC; c <= maxC; c++) {
            if (p->shape[r][c] != ' ') {
                sf::RectangleShape rect({(float)mini - 1, (float)mini - 1});
                rect.setPosition({ startX + (c - minC) * mini, startY + (r - minR) * mini });
                sf::Color col = canHold ? getColor(p->shape[r][c]) : sf::Color(100, 100, 100);
                rect.setFillColor(col);
                window.draw(rect);
            }
        }
    }
}

void drawSidebar(sf::RenderWindow& window, const SidebarUI& ui,
                 const sf::Font& font, int score, int level, int lines,
                 const Piece* next, Piece* const nextQueue[], const Piece* hold) {
    // Dark gradient-like background
    sf::RectangleShape bg({ui.w, ui.h});
    bg.setPosition({ui.x, ui.y});
    bg.setFillColor(sf::Color(25, 25, 40));
    window.draw(bg);

    drawPanel(window, ui.holdBox);
    drawPanel(window, ui.scoreBox);
    drawPanel(window, ui.levelBox);
    drawPanel(window, ui.linesBox);
    drawPanel(window, ui.nextBox);
    drawPanel(window, ui.statsBox);

    const unsigned int labelSize = 18;  // Increased to be more prominent
    const unsigned int valueSize = 20;  // Larger values
    const float pad = 8.f;  // Consistent padding
    
    // ===== LEFT COLUMN (narrow) =====
    
    // HOLD box
    float leftLabelX = ui.holdBox.position.x + pad;
    sf::Text holdLabel(font, "HOLD", labelSize);
    holdLabel.setFillColor(sf::Color(100, 200, 255));
    holdLabel.setPosition({leftLabelX, ui.holdBox.position.y + pad});
    window.draw(holdLabel);
    sf::Text holdKey(font, "[C]", 12);
    holdKey.setFillColor(sf::Color(150, 150, 150));
    holdKey.setPosition({leftLabelX, ui.holdBox.position.y + pad + 18.f});
    window.draw(holdKey);
    drawHoldPreview(window, ui, hold);
    
    // SCORE box (vertical layout: label on top, value below)
    sf::Text scoreLabel(font, "SCORE", labelSize);
    scoreLabel.setFillColor(sf::Color(255, 200, 100));
    scoreLabel.setPosition({leftLabelX, ui.scoreBox.position.y + pad});
    window.draw(scoreLabel);
    sf::Text scoreVal(font, std::to_string(score), valueSize);
    scoreVal.setFillColor(sf::Color::White);
    scoreVal.setPosition({leftLabelX, ui.scoreBox.position.y + pad + 22.f});
    window.draw(scoreVal);
    
    // LEVEL box
    sf::Text levelLabel(font, "LEVEL", labelSize);
    levelLabel.setFillColor(sf::Color(100, 255, 100));
    levelLabel.setPosition({leftLabelX, ui.levelBox.position.y + pad});
    window.draw(levelLabel);
    sf::Text levelVal(font, std::to_string(level), valueSize);
    levelVal.setFillColor(sf::Color::White);
    levelVal.setPosition({leftLabelX, ui.levelBox.position.y + pad + 22.f});
    window.draw(levelVal);
    
    // LINES box
    sf::Text linesLabel(font, "LINES", labelSize);
    linesLabel.setFillColor(sf::Color(255, 100, 255));
    linesLabel.setPosition({leftLabelX, ui.linesBox.position.y + pad});
    window.draw(linesLabel);
    sf::Text linesVal(font, std::to_string(lines), valueSize);
    linesVal.setFillColor(sf::Color::White);
    linesVal.setPosition({leftLabelX, ui.linesBox.position.y + pad + 22.f});
    window.draw(linesVal);
    
    // ===== RIGHT COLUMN (NEXT pieces) =====
    
    float nextLabelX = ui.nextBox.position.x + pad;
    sf::Text nextLabel(font, "NEXT", labelSize);
    nextLabel.setFillColor(sf::Color(255, 150, 150));
    nextLabel.setPosition({nextLabelX, ui.nextBox.position.y + pad});
    window.draw(nextLabel);
    
    // Draw 5 next pieces vertically
    float nextY = ui.nextBox.position.y + 32.f;
    const Piece* allNext[5] = {next, nextQueue[0], nextQueue[1], nextQueue[2], nextQueue[3]};
    for (int p = 0; p < 5; p++) {
        if (allNext[p]) {
            int minR = 4, minC = 4, maxR = -1, maxC = -1;
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    if (allNext[p]->shape[r][c] != ' ') {
                        minR = std::min(minR, r); minC = std::min(minC, c);
                        maxR = std::max(maxR, r); maxC = std::max(maxC, c);
                    }
                }
            }
            if (maxR != -1) {
                int cellsW = maxC - minC + 1;
                int cellsH = maxR - minR + 1;
                int mini = 12;
                
                float startX = ui.nextBox.position.x + (ui.nextBox.size.x - cellsW * mini) * 0.5f;
                float startY = nextY + (56.f - cellsH * mini) * 0.5f;
                
                for (int r = minR; r <= maxR; r++) {
                    for (int c = minC; c <= maxC; c++) {
                        if (allNext[p]->shape[r][c] != ' ') {
                            drawTile3D(window, startX + (c - minC) * mini, startY + (r - minR) * mini,
                                      mini, allNext[p]->shape[r][c]);
                        }
                    }
                }
            }
        }
        nextY += 59.f;
    }
    
    // ===== BOTTOM: GAME INFO (full width) =====
    
    float infoLabelX = ui.statsBox.position.x + pad;
    sf::Text statsLabel(font, "GAME INFO", labelSize);
    statsLabel.setFillColor(sf::Color(150, 200, 255));
    statsLabel.setPosition({infoLabelX, ui.statsBox.position.y + pad});
    window.draw(statsLabel);
    
    const unsigned int infoSize = 14;
    float infoY = ui.statsBox.position.y + pad + 24.f;
    float lineHeight = 20.f;
    
    // Time
    int minutes = static_cast<int>(playTime) / 60;
    int seconds = static_cast<int>(playTime) % 60;
    char timeStr[20];
    snprintf(timeStr, 20, "Time: %02d:%02d", minutes, seconds);
    sf::Text timeText(font, timeStr, infoSize);
    timeText.setFillColor(sf::Color(200, 200, 200));
    timeText.setPosition({infoLabelX, infoY});
    window.draw(timeText);
    infoY += lineHeight;
    
    // Best score
    sf::Text bestText(font, "Best: " + std::to_string(highScore), infoSize);
    bestText.setFillColor(sf::Color(255, 215, 0));
    bestText.setPosition({infoLabelX, infoY});
    window.draw(bestText);
    infoY += lineHeight;
    
    // Pieces count
    sf::Text piecesText(font, "Pieces: " + std::to_string(totalPieces), infoSize);
    piecesText.setFillColor(sf::Color(200, 200, 200));
    piecesText.setPosition({infoLabelX, infoY});
    window.draw(piecesText);
    infoY += lineHeight;
    
    // PPM (Pieces per Minute)
    float ppm = (playTime > 0) ? (totalPieces / playTime * 60.f) : 0.f;
    char ppmStr[20];
    snprintf(ppmStr, 20, "PPM: %.1f", ppm);
    sf::Text ppmText(font, ppmStr, infoSize);
    ppmText.setFillColor(sf::Color(200, 200, 200));
    ppmText.setPosition({infoLabelX, infoY});
    window.draw(ppmText);
    infoY += lineHeight;
    
    // Lines per Minute
    float lpm = (playTime > 0) ? (lines / playTime * 60.f) : 0.f;
    char lpmStr[20];
    snprintf(lpmStr, 20, "LPM: %.1f", lpm);
    sf::Text lpmText(font, lpmStr, infoSize);
    lpmText.setFillColor(sf::Color(200, 200, 200));
    lpmText.setPosition({infoLabelX, infoY});
    window.draw(lpmText);
    infoY += lineHeight;
    
    // Tetris count
    sf::Text tetrisText(font, "Tetris: " + std::to_string(tetrisCount), infoSize);
    tetrisText.setFillColor(sf::Color(0, 240, 240));
    tetrisText.setPosition({infoLabelX, infoY});
    window.draw(tetrisText);
    infoY += lineHeight;
    
    // T-Spin count
    sf::Text tspinText(font, "T-Spin: " + std::to_string(tSpinCount), infoSize);
    tspinText.setFillColor(sf::Color(200, 100, 255));
    tspinText.setPosition({infoLabelX, infoY});
    window.draw(tspinText);
    infoY += lineHeight;
    
    // Max combo
    static int maxCombo = 0;
    if (comboCount > maxCombo) maxCombo = comboCount;
    sf::Text maxComboText(font, "Max Combo: " + std::to_string(maxCombo), infoSize);
    maxComboText.setFillColor(sf::Color(255, 150, 100));
    maxComboText.setPosition({infoLabelX, infoY});
    window.draw(maxComboText);
    infoY += lineHeight;
    
    // Current combo (if active)
    if (comboCount > 1) {
        sf::Text comboText(font, "Combo: x" + std::to_string(comboCount), infoSize);
        comboText.setFillColor(sf::Color(255, 100, 100));
        comboText.setPosition({infoLabelX, infoY});
        window.draw(comboText);
        infoY += lineHeight;
    }
    
    // Back-to-Back indicator
    if (backToBackActive) {
        sf::Text b2bText(font, "B2B Active!", infoSize);
        b2bText.setFillColor(sf::Color(255, 255, 0));
        b2bText.setPosition({infoLabelX, infoY});
        window.draw(b2bText);
    }
}

void drawSettingsScreen(sf::RenderWindow& window, const sf::Font& font) {
    Text settingsTitle(font);
    settingsTitle.setString("SETTINGS");
    settingsTitle.setCharacterSize(40);
    settingsTitle.setFillColor(Color::Cyan);
    float settingsTitleW = settingsTitle.getLocalBounds().size.x;
    settingsTitle.setPosition(sf::Vector2f{(WINDOW_W - settingsTitleW) / 2.f, 50.f});
    window.draw(settingsTitle);

    // --- Music Volume --- (row Y = 130)
    Text musicLabel(font);
    musicLabel.setString("Music Volume");
    musicLabel.setCharacterSize(20);
    musicLabel.setFillColor(Color::White);
    musicLabel.setPosition(sf::Vector2f{100.f, 130.f});
    window.draw(musicLabel);

    Text musicLeftArrow(font);
    musicLeftArrow.setString("<");
    musicLeftArrow.setCharacterSize(20);
    musicLeftArrow.setFillColor(Color::Yellow);
    musicLeftArrow.setPosition(sf::Vector2f{260.f, 130.f});
    window.draw(musicLeftArrow);

    RectangleShape musicSliderBg(Vector2f(200, 20));
    musicSliderBg.setPosition(sf::Vector2f{285.f, 132.f});
    musicSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(musicSliderBg);

    RectangleShape musicSliderFill(Vector2f(musicVolume * 2, 20));
    musicSliderFill.setPosition(sf::Vector2f{285.f, 132.f});
    musicSliderFill.setFillColor(Color(0, 150, 255));
    window.draw(musicSliderFill);

    Text musicRightArrow(font);
    musicRightArrow.setString(">");
    musicRightArrow.setCharacterSize(20);
    musicRightArrow.setFillColor(Color::Yellow);
    musicRightArrow.setPosition(sf::Vector2f{490.f, 130.f});
    window.draw(musicRightArrow);

    Text musicValue(font);
    musicValue.setString(std::to_string((int)musicVolume) + "%");
    musicValue.setCharacterSize(18);
    musicValue.setFillColor(Color::White);
    musicValue.setPosition(sf::Vector2f{520.f, 132.f});
    window.draw(musicValue);

    // --- SFX Volume --- (row Y = 190)
    Text sfxLabel(font);
    sfxLabel.setString("SFX Volume");
    sfxLabel.setCharacterSize(20);
    sfxLabel.setFillColor(Color::White);
    sfxLabel.setPosition(sf::Vector2f{100.f, 190.f});
    window.draw(sfxLabel);

    Text sfxLeftArrow(font);
    sfxLeftArrow.setString("<");
    sfxLeftArrow.setCharacterSize(20);
    sfxLeftArrow.setFillColor(Color::Yellow);
    sfxLeftArrow.setPosition(sf::Vector2f{260.f, 190.f});
    window.draw(sfxLeftArrow);

    RectangleShape sfxSliderBg(Vector2f(200, 20));
    sfxSliderBg.setPosition(sf::Vector2f{285.f, 192.f});
    sfxSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(sfxSliderBg);

    RectangleShape sfxSliderFill(Vector2f(sfxVolume * 2, 20));
    sfxSliderFill.setPosition(sf::Vector2f{285.f, 192.f});
    sfxSliderFill.setFillColor(Color(0, 200, 100));
    window.draw(sfxSliderFill);

    Text sfxRightArrow(font);
    sfxRightArrow.setString(">");
    sfxRightArrow.setCharacterSize(20);
    sfxRightArrow.setFillColor(Color::Yellow);
    sfxRightArrow.setPosition(sf::Vector2f{490.f, 190.f});
    window.draw(sfxRightArrow);

    Text sfxValue(font);
    sfxValue.setString(std::to_string((int)sfxVolume) + "%");
    sfxValue.setCharacterSize(18);
    sfxValue.setFillColor(Color::White);
    sfxValue.setPosition(sf::Vector2f{520.f, 192.f});
    window.draw(sfxValue);

    // --- Brightness --- (row Y = 250)
    Text brightnessLabel(font);
    brightnessLabel.setString("Brightness");
    brightnessLabel.setCharacterSize(20);
    brightnessLabel.setFillColor(Color::White);
    brightnessLabel.setPosition(sf::Vector2f{100.f, 250.f});
    window.draw(brightnessLabel);

    Text brightLeftArrow(font);
    brightLeftArrow.setString("<");
    brightLeftArrow.setCharacterSize(20);
    brightLeftArrow.setFillColor(Color::Yellow);
    brightLeftArrow.setPosition(sf::Vector2f{260.f, 250.f});
    window.draw(brightLeftArrow);

    RectangleShape brightnessSliderBg(Vector2f(200, 20));
    brightnessSliderBg.setPosition(sf::Vector2f{285.f, 252.f});
    brightnessSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(brightnessSliderBg);

    RectangleShape brightnessSliderFill(Vector2f((brightness / 255.f) * 200.f, 20));
    brightnessSliderFill.setPosition(sf::Vector2f{285.f, 252.f});
    brightnessSliderFill.setFillColor(Color(255, 200, 50));
    window.draw(brightnessSliderFill);

    Text brightRightArrow(font);
    brightRightArrow.setString(">");
    brightRightArrow.setCharacterSize(20);
    brightRightArrow.setFillColor(Color::Yellow);
    brightRightArrow.setPosition(sf::Vector2f{490.f, 250.f});
    window.draw(brightRightArrow);

    Text brightnessValue(font);
    brightnessValue.setString(std::to_string((int)(brightness / 255.f * 100.f)) + "%");
    brightnessValue.setCharacterSize(18);
    brightnessValue.setFillColor(Color::White);
    brightnessValue.setPosition(sf::Vector2f{520.f, 252.f});
    window.draw(brightnessValue);

    // --- Ghost Piece Toggle --- (row Y = 310)
    Text ghostLabel(font);
    ghostLabel.setString("Ghost Piece");
    ghostLabel.setCharacterSize(20);
    ghostLabel.setFillColor(Color::White);
    ghostLabel.setPosition(sf::Vector2f{100.f, 310.f});
    window.draw(ghostLabel);

    RectangleShape checkBox(Vector2f(24, 24));
    checkBox.setPosition(sf::Vector2f{285.f, 308.f});
    checkBox.setFillColor(Color(80, 80, 80));
    checkBox.setOutlineThickness(2.f);
    checkBox.setOutlineColor(Color::White);
    window.draw(checkBox);

    if (ghostPieceEnabled) {
        Text checkMark(font);
        checkMark.setString("X");
        checkMark.setCharacterSize(18);
        checkMark.setFillColor(Color::Green);
        checkMark.setPosition(sf::Vector2f{290.f, 308.f});
        window.draw(checkMark);
    }

    Text ghostStatus(font);
    ghostStatus.setString(ghostPieceEnabled ? "ON" : "OFF");
    ghostStatus.setCharacterSize(18);
    ghostStatus.setFillColor(ghostPieceEnabled ? Color::Green : Color::Red);
    ghostStatus.setPosition(sf::Vector2f{320.f, 312.f});
    window.draw(ghostStatus);

    // Back button
    const float backBtnW = 200.f;
    const float backBtnX = (WINDOW_W - backBtnW) / 2.f;
    RectangleShape backBtn(Vector2f(backBtnW, 50));
    backBtn.setPosition(sf::Vector2f{backBtnX, 380.f});
    backBtn.setFillColor(Color(100, 100, 100));
    window.draw(backBtn);

    Text backText(font);
    backText.setString("BACK");
    backText.setCharacterSize(24);
    backText.setFillColor(Color::White);
    float backTxtW = backText.getLocalBounds().size.x;
    backText.setPosition(sf::Vector2f{backBtnX + (backBtnW - backTxtW) / 2.f, 390.f});
    window.draw(backText);
}

void handleSettingsClick(sf::Vector2i mousePos) {
    // Music row Y=130
    if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 125 && mousePos.y <= 155) {
        musicVolume = std::max(0.f, musicVolume - 5.f);
        Audio::setMusicVolume(musicVolume);
        Audio::playSettingClick();
    }
    if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 125 && mousePos.y <= 155) {
        musicVolume = std::min(100.f, musicVolume + 5.f);
        Audio::setMusicVolume(musicVolume);
        Audio::playSettingClick();
    }
    if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 127 && mousePos.y <= 157) {
        musicVolume = static_cast<float>(mousePos.x - 285) / 2.f;
        musicVolume = std::max(0.f, std::min(100.f, musicVolume));
        Audio::setMusicVolume(musicVolume);
        Audio::playSettingClick();
    }

    // SFX row Y=190
    if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 185 && mousePos.y <= 215) {
        sfxVolume = std::max(0.f, sfxVolume - 5.f);
        Audio::setSfxVolume(sfxVolume);
        Audio::playSettingClick();
    }
    if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 185 && mousePos.y <= 215) {
        sfxVolume = std::min(100.f, sfxVolume + 5.f);
        Audio::setSfxVolume(sfxVolume);
        Audio::playSettingClick();
    }
    if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 187 && mousePos.y <= 217) {
        sfxVolume = static_cast<float>(mousePos.x - 285) / 2.f;
        sfxVolume = std::max(0.f, std::min(100.f, sfxVolume));
        Audio::setSfxVolume(sfxVolume);
        Audio::playSettingClick();
    }

    // Brightness row Y=250
    if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 245 && mousePos.y <= 275) {
        brightness = std::max(50.f, brightness - 10.f);
        Audio::playSettingClick();
    }
    if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 245 && mousePos.y <= 275) {
        brightness = std::min(255.f, brightness + 10.f);
        Audio::playSettingClick();
    }
    if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 247 && mousePos.y <= 277) {
        brightness = static_cast<float>(mousePos.x - 285) / 200.f * 255.f;
        brightness = std::max(50.f, std::min(255.f, brightness));
        Audio::playSettingClick();
    }

    // Ghost Piece checkbox
    if (mousePos.x >= 280 && mousePos.x <= 315 && mousePos.y >= 303 && mousePos.y <= 337) {
        ghostPieceEnabled = !ghostPieceEnabled;
        if (ghostPieceEnabled) {
            Audio::playToggleOn();
        } else {
            Audio::playToggleOff();
        }
    }
}

void drawGameOverScreen(sf::RenderWindow& window, const sf::Font& font) {
    // Full screen overlay
    RectangleShape overlay(Vector2f(WINDOW_W, WINDOW_H));
    overlay.setFillColor(Color(0, 0, 0, 200));
    window.draw(overlay);

    // Game Over text
    const float fullW = WINDOW_W;
    Text gameOverText(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(Color::Red);
    float goWidth = gameOverText.getLocalBounds().size.x;
    gameOverText.setPosition(sf::Vector2f{(fullW - goWidth) / 2.f, 150.f});
    window.draw(gameOverText);

    // Buttons
    const float goBtnW = 200.f;
    const float goBtnX = (fullW - goBtnW) / 2.f;

    RectangleShape restartBtn(Vector2f(goBtnW, 50));
    restartBtn.setPosition(sf::Vector2f{goBtnX, 230.f});
    restartBtn.setFillColor(Color(0, 100, 255));
    window.draw(restartBtn);
    Text restartText(font);
    restartText.setString("RESTART");
    restartText.setCharacterSize(24);
    restartText.setFillColor(Color::White);
    float restartTxtW = restartText.getLocalBounds().size.x;
    restartText.setPosition(sf::Vector2f{goBtnX + (goBtnW - restartTxtW) / 2.f, 240.f});
    window.draw(restartText);

    RectangleShape menuBtn(Vector2f(goBtnW, 50));
    menuBtn.setPosition(sf::Vector2f{goBtnX, 300.f});
    menuBtn.setFillColor(Color(100, 100, 100));
    window.draw(menuBtn);
    Text menuText(font);
    menuText.setString("MENU");
    menuText.setCharacterSize(24);
    menuText.setFillColor(Color::White);
    float menuTxtW = menuText.getLocalBounds().size.x;
    menuText.setPosition(sf::Vector2f{goBtnX + (goBtnW - menuTxtW) / 2.f, 310.f});
    window.draw(menuText);

    RectangleShape exitBtn(Vector2f(goBtnW, 50));
    exitBtn.setPosition(sf::Vector2f{goBtnX, 370.f});
    exitBtn.setFillColor(Color(255, 50, 50));
    window.draw(exitBtn);
    Text exitText(font);
    exitText.setString("EXIT");
    exitText.setCharacterSize(24);
    exitText.setFillColor(Color::White);
    float exitTxtW = exitText.getLocalBounds().size.x;
    exitText.setPosition(sf::Vector2f{goBtnX + (goBtnW - exitTxtW) / 2.f, 380.f});
    window.draw(exitText);
}

void drawBrightnessOverlay(sf::RenderWindow& window) {
    if (brightness < 255.f) {
        RectangleShape darkenOverlay(Vector2f(WINDOW_W, WINDOW_H));
        darkenOverlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(255 - brightness)));
        window.draw(darkenOverlay);
    }
}

void drawMenu(sf::RenderWindow& window, const sf::Font& font) {
    const float fullW = WINDOW_W;
    
    // Title
    Text title(font, "TETRIS", 60);
    title.setFillColor(Color::Cyan);
    title.setStyle(Text::Bold);
    float titleW = title.getLocalBounds().size.x;
    title.setPosition({(fullW - titleW) / 2.f, 80.f});
    window.draw(title);
    
    const float btnW = 200.f;
    const float btnH = 45.f;
    const float btnX = (fullW - btnW) / 2.f;
    
    // Difficulty selector
    Text diffLabel(font, "DIFFICULTY", 18);
    diffLabel.setFillColor(Color::White);
    float diffLabelW = diffLabel.getLocalBounds().size.x;
    diffLabel.setPosition({(fullW - diffLabelW) / 2.f, 180.f});
    window.draw(diffLabel);
    
    const char* diffNames[] = {"EASY", "NORMAL", "HARD"};
    Color diffColors[] = {Color(0, 150, 0), Color(180, 140, 0), Color(180, 0, 0)};  // Darker colors
    float diffBtnW = 80.f;
    float diffStartX = (fullW - 3 * diffBtnW - 20.f) / 2.f;
    
    for (int i = 0; i < 3; i++) {
        RectangleShape diffBtn({diffBtnW, 35.f});
        diffBtn.setPosition({diffStartX + i * (diffBtnW + 10.f), 210.f});
        bool selected = (static_cast<int>(difficulty) == i);
        diffBtn.setFillColor(selected ? diffColors[i] : Color(60, 60, 60));
        diffBtn.setOutlineThickness(selected ? 3.f : 1.f);
        diffBtn.setOutlineColor(selected ? Color::White : Color(100, 100, 100));
        window.draw(diffBtn);
        
        Text diffText(font, diffNames[i], 14);
        diffText.setFillColor(Color::White);
        float dtW = diffText.getLocalBounds().size.x;
        diffText.setPosition({diffStartX + i * (diffBtnW + 10.f) + (diffBtnW - dtW) / 2.f, 218.f});
        window.draw(diffText);
    }
    
    // START button
    RectangleShape startBtn({btnW, btnH});
    startBtn.setPosition({btnX, 280.f});
    startBtn.setFillColor(Color(0, 150, 0));
    window.draw(startBtn);
    Text startText(font, "START", 24);
    startText.setFillColor(Color::White);
    float startTxtW = startText.getLocalBounds().size.x;
    startText.setPosition({btnX + (btnW - startTxtW) / 2.f, 288.f});
    window.draw(startText);
    
    // SETTINGS button
    RectangleShape settingBtn({btnW, btnH});
    settingBtn.setPosition({btnX, 340.f});
    settingBtn.setFillColor(Color(100, 100, 100));
    window.draw(settingBtn);
    Text settingText(font, "SETTINGS", 24);
    settingText.setFillColor(Color::White);
    float settingTxtW = settingText.getLocalBounds().size.x;
    settingText.setPosition({btnX + (btnW - settingTxtW) / 2.f, 348.f});
    window.draw(settingText);
    
    // HOW TO PLAY button
    RectangleShape howToBtn({btnW, btnH});
    howToBtn.setPosition({btnX, 400.f});
    howToBtn.setFillColor(Color(0, 100, 180));
    window.draw(howToBtn);
    Text howToText(font, "HOW TO PLAY", 24);
    howToText.setFillColor(Color::White);
    float howToTxtW = howToText.getLocalBounds().size.x;
    howToText.setPosition({btnX + (btnW - howToTxtW) / 2.f, 408.f});
    window.draw(howToText);
    
    // EXIT button
    RectangleShape exitBtn({btnW, btnH});
    exitBtn.setPosition({btnX, 460.f});
    exitBtn.setFillColor(Color(200, 0, 0));
    window.draw(exitBtn);
    Text exitText(font, "EXIT", 24);
    exitText.setFillColor(Color::White);
    float exitTxtW = exitText.getLocalBounds().size.x;
    exitText.setPosition({btnX + (btnW - exitTxtW) / 2.f, 468.f});
    window.draw(exitText);
    
    // High Score
    Text hsText(font, "HIGH SCORE: " + std::to_string(highScore), 18);
    hsText.setFillColor(Color::Yellow);
    float hsW = hsText.getLocalBounds().size.x;
    hsText.setPosition({(fullW - hsW) / 2.f, 530.f});
    window.draw(hsText);
    
    // Controls hint
    Text controlsText(font, "Press F11 for Fullscreen", 12);
    controlsText.setFillColor(Color(150, 150, 150));
    float ctrlW = controlsText.getLocalBounds().size.x;
    controlsText.setPosition({(fullW - ctrlW) / 2.f, 560.f});
    window.draw(controlsText);
}

void handleMenuClick(sf::Vector2i mousePos, GameState& state, GameState& previousState, bool& shouldClose) {
    const float fullW = WINDOW_W;
    const float btnW = 200.f;
    const float btnX = (fullW - btnW) / 2.f;
    
    // Difficulty buttons
    float diffBtnW = 80.f;
    float diffStartX = (fullW - 3 * diffBtnW - 20.f) / 2.f;
    for (int i = 0; i < 3; i++) {
        float bx = diffStartX + i * (diffBtnW + 10.f);
        if (mousePos.x >= bx && mousePos.x <= bx + diffBtnW &&
            mousePos.y >= 210 && mousePos.y <= 245) {
            difficulty = static_cast<Difficulty>(i);
            Audio::playSettingClick();
        }
    }
    
    // START
    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 280 && mousePos.y <= 325) {
        Audio::playStartGame();
        resetGame();
        state = GameState::PLAYING;
    }
    
    // SETTINGS
    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 340 && mousePos.y <= 385) {
        Audio::playOpenSettings();
        previousState = GameState::MENU;  // Remember we came from menu
        state = GameState::SETTINGS;
    }
    
    // HOW TO PLAY
    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 400 && mousePos.y <= 445) {
        Audio::playOpenSettings();
        state = GameState::HOWTOPLAY;
    }
    
    // EXIT
    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 460 && mousePos.y <= 505) {
        shouldClose = true;
    }
}

void drawPauseScreen(sf::RenderWindow& window, const sf::Font& font) {
    const float fullW = WINDOW_W;
    
    // Dark overlay
    RectangleShape overlay(Vector2f(fullW, WINDOW_H));
    overlay.setFillColor(Color(0, 0, 0, 180));
    window.draw(overlay);
    
    // PAUSED text
    Text pauseText(font, "PAUSED", 50);
    pauseText.setFillColor(Color::Yellow);
    float pw = pauseText.getLocalBounds().size.x;
    pauseText.setPosition({(fullW - pw) / 2.f, 180.f});
    window.draw(pauseText);
    
    Text hintText(font, "Press P or ESC to resume", 18);
    hintText.setFillColor(Color::White);
    float hw = hintText.getLocalBounds().size.x;
    hintText.setPosition({(fullW - hw) / 2.f, 260.f});
    window.draw(hintText);
    
    // Buttons
    const float btnW = 200.f;
    const float btnX = (fullW - btnW) / 2.f;
    const float btnH = 50.f;
    
    // Resume button
    RectangleShape resumeBtn({btnW, btnH});
    resumeBtn.setPosition({btnX, 310.f});
    resumeBtn.setFillColor(Color(0, 150, 0));
    window.draw(resumeBtn);
    Text resumeText(font, "RESUME", 24);
    resumeText.setFillColor(Color::White);
    float rtW = resumeText.getLocalBounds().size.x;
    resumeText.setPosition({btnX + (btnW - rtW) / 2.f, 320.f});
    window.draw(resumeText);
    
    // Settings button
    RectangleShape settingsBtn({btnW, btnH});
    settingsBtn.setPosition({btnX, 380.f});
    settingsBtn.setFillColor(Color(0, 100, 200));
    window.draw(settingsBtn);
    Text settingsText(font, "SETTINGS", 24);
    settingsText.setFillColor(Color::White);
    float stW = settingsText.getLocalBounds().size.x;
    settingsText.setPosition({btnX + (btnW - stW) / 2.f, 390.f});
    window.draw(settingsText);
    
    // Menu button
    RectangleShape menuBtn({btnW, btnH});
    menuBtn.setPosition({btnX, 450.f});
    menuBtn.setFillColor(Color(100, 100, 100));
    window.draw(menuBtn);
    Text menuText(font, "MENU", 24);
    menuText.setFillColor(Color::White);
    float mtW = menuText.getLocalBounds().size.x;
    menuText.setPosition({btnX + (btnW - mtW) / 2.f, 460.f});
    window.draw(menuText);
}

// Line clear animation
void startLineClearAnim(int* clearedLines, int count) {
    lineClearAnim.active = true;
    lineClearAnim.timer = 0.f;
    lineClearAnim.count = count;
    for (int i = 0; i < 4; i++) {
        lineClearAnim.lines[i] = (i < count) ? clearedLines[i] : -1;
    }
}

void updateLineClearAnim(float dt) {
    if (!lineClearAnim.active) return;
    lineClearAnim.timer += dt;
    if (lineClearAnim.timer >= 0.3f) {
        lineClearAnim.active = false;
    }
}

void drawLineClearAnim(sf::RenderWindow& window) {
    if (!lineClearAnim.active) return;
    
    float alpha = (1.f - lineClearAnim.timer / 0.3f) * 255.f;
    
    for (int i = 0; i < lineClearAnim.count; i++) {
        int lineY = lineClearAnim.lines[i];
        if (lineY < 0) continue;
        
        RectangleShape flash(Vector2f(PLAY_W_PX - 2 * TILE_SIZE, TILE_SIZE));
        flash.setPosition({STATS_W + (float)TILE_SIZE, (float)(lineY * TILE_SIZE)});
        flash.setFillColor(Color(255, 255, 255, static_cast<uint8_t>(alpha)));
        window.draw(flash);
    }
}

void drawCombo(sf::RenderWindow& window, const sf::Font& font) {
    if (comboCount <= 1) return;
    
    Text comboText(font, "COMBO x" + std::to_string(comboCount), 30);
    comboText.setFillColor(Color::Yellow);
    float cw = comboText.getLocalBounds().size.x;
    comboText.setPosition({STATS_W + (PLAY_W_PX - cw) / 2.f, WINDOW_H / 2.f - 50.f});
    window.draw(comboText);
}

void addParticles(float x, float y, sf::Color color, int count) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = x;
        p.y = y;
        // Random velocity
        float angle = (rand() % 360) * 3.14159f / 180.f;
        float speed = 50.f + (rand() % 100);
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed - 50.f; // Upward bias
        p.life = 0.5f + (rand() % 100) / 200.f;
        p.color = color;
        particles.push_back(p);
    }
}

void updateParticles(float dt) {
    for (auto it = particles.begin(); it != particles.end();) {
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        it->vy += 200.f * dt; // Gravity
        it->life -= dt;
        
        if (it->life <= 0.f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void drawParticles(sf::RenderWindow& window) {
    for (const auto& p : particles) {
        sf::CircleShape circle(2.f);
        circle.setPosition({p.x, p.y});
        sf::Color c = p.color;
        c.a = static_cast<uint8_t>(p.life * 255.f);
        circle.setFillColor(c);
        window.draw(circle);
    }
}

void drawSoftDropTrail(sf::RenderWindow& window, const Piece* piece, int px, int py, bool isActive) {
    if (!isActive || !piece) return;
    
    // Draw fading trail above piece
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece->shape[i][j] != ' ') {
                float tileX = STATS_W + (px + j) * TILE_SIZE;
                float tileY = (py + i) * TILE_SIZE;
                
                // Draw 3 trail positions above
                for (int t = 1; t <= 3; t++) {
                    sf::RectangleShape trail({TILE_SIZE - 2.f, TILE_SIZE - 2.f});
                    trail.setPosition({tileX + 1.f, tileY - t * TILE_SIZE * 0.8f});
                    sf::Color c = getColor(piece->shape[i][j]);
                    c.a = static_cast<uint8_t>(80 / t); // Fade out
                    trail.setFillColor(c);
                    window.draw(trail);
                }
            }
        }
    }
}

void drawHowToPlay(sf::RenderWindow& window, const sf::Font& font) {
    const float fullW = WINDOW_W;
    
    // Background
    RectangleShape bg(Vector2f(fullW, WINDOW_H));
    bg.setFillColor(Color(20, 20, 30));
    window.draw(bg);
    
    // Title
    Text title(font, "HOW TO PLAY", 36);
    title.setFillColor(Color::Cyan);
    title.setStyle(Text::Bold);
    float titleW = title.getLocalBounds().size.x;
    title.setPosition({(fullW - titleW) / 2.f, 20.f});
    window.draw(title);
    
    // Content - Two columns
    float leftCol = 30.f;
    float rightCol = fullW / 2.f + 10.f;
    float y = 75.f;
    float lineH = 22.f;
    
    auto drawSection = [&](float x, float& yPos, const std::string& header, const std::vector<std::string>& lines) {
        Text headerText(font, header, 16);
        headerText.setFillColor(Color::Yellow);
        headerText.setStyle(Text::Bold);
        headerText.setPosition({x, yPos});
        window.draw(headerText);
        yPos += lineH;
        
        for (const auto& line : lines) {
            Text lineText(font, line, 13);
            lineText.setFillColor(Color::White);
            lineText.setPosition({x + 10.f, yPos});
            window.draw(lineText);
            yPos += lineH - 4.f;
        }
        yPos += 8.f;
    };
    
    // Left column - Controls
    float leftY = y;
    drawSection(leftCol, leftY, "CONTROLS", {
        "Left/Right Arrow - Move piece",
        "Down Arrow - Soft drop",
        "Up Arrow - Rotate clockwise",
        "Space - Hard drop (instant)",
        "C - Hold piece",
        "P or ESC - Pause game",
        "F11 - Toggle fullscreen"
    });
    
    drawSection(leftCol, leftY, "SCORING", {
        "Single line: 100 x Level",
        "Double: 300 x Level",
        "Triple: 500 x Level",
        "Tetris (4 lines): 800 x Level",
        "Combo bonus for chain clears"
    });
    
    drawSection(leftCol, leftY, "7-BAG SYSTEM", {
        "All 7 pieces appear once",
        "Then shuffle and repeat",
        "Guarantees fair distribution"
    });
    
    // Right column - Advanced mechanics
    float rightY = y;
    drawSection(rightCol, rightY, "T-SPIN", {
        "Rotate T piece into tight slot",
        "3 corners must be filled",
        "T-Spin Single: 800 x Level",
        "T-Spin Double: 1200 x Level",
        "T-Spin Triple: 1600 x Level"
    });
    
    drawSection(rightCol, rightY, "BACK-TO-BACK (B2B)", {
        "Consecutive Tetris or T-Spin",
        "1.5x score multiplier",
        "Chain breaks on non-special clear"
    });
    
    drawSection(rightCol, rightY, "PERFECT CLEAR", {
        "Clear ALL blocks on board",
        "+3000 bonus points",
        "Very rare and difficult!"
    });
    
    drawSection(rightCol, rightY, "LOCK DELAY", {
        "500ms before piece locks",
        "Move/rotate to reset timer",
        "Max 15 resets per piece"
    });
    
    drawSection(rightCol, rightY, "GHOST PIECE", {
        "Shows where piece will land",
        "Helps plan your drops"
    });
    
    // Back button
    const float btnW = 150.f;
    const float btnH = 40.f;
    const float btnX = (fullW - btnW) / 2.f;
    const float btnY = WINDOW_H - 60.f;
    
    RectangleShape backBtn({btnW, btnH});
    backBtn.setPosition({btnX, btnY});
    backBtn.setFillColor(Color(100, 100, 100));
    backBtn.setOutlineThickness(2.f);
    backBtn.setOutlineColor(Color::White);
    window.draw(backBtn);
    
    Text backText(font, "BACK", 24);
    backText.setFillColor(Color::White);
    float backTxtW = backText.getLocalBounds().size.x;
    backText.setPosition({btnX + (btnW - backTxtW) / 2.f, btnY + 6.f});
    window.draw(backText);
}

void handleHowToPlayClick(sf::Vector2i mousePos, GameState& state) {
    const float fullW = WINDOW_W;
    const float btnW = 150.f;
    const float btnH = 40.f;
    const float btnX = (fullW - btnW) / 2.f;
    const float btnY = WINDOW_H - 60.f;
    
    // Back button
    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= btnY && mousePos.y <= btnY + btnH) {
        Audio::playCloseSettings();
        state = GameState::MENU;
    }
}

} // namespace UI
