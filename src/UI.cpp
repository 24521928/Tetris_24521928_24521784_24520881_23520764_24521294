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

static void drawPanel(sf::RenderWindow& window, const sf::FloatRect& r) {
    const float outline = 3.f;
    const float inset = outline;
/** Render panel */
    sf::RectangleShape box({ r.size.x - 2.f*inset, r.size.y - 2.f*inset });
    box.setPosition({ r.position.x + inset, r.position.y + inset });
    box.setFillColor(sf::Color(15, 15, 25));
    box.setOutlineThickness(outline);
    box.setOutlineColor(sf::Color(80, 80, 120));
    window.draw(box);
}

namespace UI {

LineClearAnim lineClearAnim;

std::vector<Particle> particles;

/** Render tile3d */
void drawTile3D(sf::RenderWindow& window, float px, float py, float size, char c) {
    if (c == ' ') {
        sf::RectangleShape bg({size - 3.f, size - 3.f});
        bg.setPosition({px + 1.5f, py + 1.5f});
        bg.setFillColor(sf::Color(20, 20, 30));
        window.draw(bg);
        return;
    }

    const float bevel = 3.f;
    const float gap = 3.f;
    sf::Color main = getColor(c);
    sf::Color highlight = getHighlightColor(c);
    sf::Color shadow = getShadowColor(c);

/** Process getColor */
    sf::RectangleShape body({size - gap, size - gap});
    body.setPosition({px + gap/2.f, py + gap/2.f});
    body.setFillColor(main);
    window.draw(body);

/** Process body */
    sf::RectangleShape top({size - gap, bevel});
    top.setPosition({px + gap/2.f, py + gap/2.f});
    top.setFillColor(highlight);
    window.draw(top);

/** Process top */
    sf::RectangleShape left({bevel, size - gap});
    left.setPosition({px + gap/2.f, py + gap/2.f});
    left.setFillColor(highlight);
    window.draw(left);

/** Process left */
    sf::RectangleShape bottom({size - gap, bevel});
    bottom.setPosition({px + gap/2.f, py + size - gap/2.f - bevel});
    bottom.setFillColor(shadow);
    window.draw(bottom);

/** Process bottom */
    sf::RectangleShape right({bevel, size - gap});
    right.setPosition({px + size - gap/2.f - bevel, py + gap/2.f});
    right.setFillColor(shadow);
    window.draw(right);

/** Process right */
    sf::RectangleShape shine({(size - gap) * 0.3f, (size - gap) * 0.15f});
    shine.setPosition({px + gap/2.f + (size - gap) * 0.2f, py + gap/2.f + (size - gap) * 0.15f});
    sf::Color shineColor = highlight;
    shineColor.a = 100;
    shine.setFillColor(shineColor);
    window.draw(shine);
}

/** Process setFillColor */
void drawPieceStats(sf::RenderWindow& window, const sf::Font& font) {

    const char pieces[7] = {'I', 'O', 'T', 'S', 'Z', 'J', 'L'};
    const int shapes[7][4][4] = {

        {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},

        {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},

        {{0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},

        {{0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0}},

        {{1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0}},

        {{1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}},

        {{0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0}}
    };

    float panelX = 8.f;
    float panelY = 12.f;
    float panelW = STATS_W - 16.f;
    float panelH = WINDOW_H - 24.f;

/** Process bg */
    sf::RectangleShape bg({panelW, panelH});
    bg.setPosition({panelX, panelY});
    bg.setFillColor(sf::Color(15, 15, 25));
    bg.setOutlineThickness(3.f);
    bg.setOutlineColor(sf::Color(80, 80, 120));
    window.draw(bg);

    sf::Text title(font, "STATS", 26);
    title.setFillColor(sf::Color::White);
    float titleW = title.getLocalBounds().size.x;
    title.setPosition({panelX + (panelW - titleW) / 2.f, panelY + 12.f});
    window.draw(title);

    float rowY = panelY + 56.f;
    int mini = 15;
    float rowH = (panelH - 60.f) / 7.f;

    for (int p = 0; p < 7; p++) {

        int minC = 4, maxC = -1, minR = 4, maxR = -1;
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (shapes[p][r][c]) {
                    minC = std::min(minC, c);
                    maxC = std::max(maxC, c);
                    minR = std::min(minR, r);
                    maxR = std::max(maxR, r);
                }
            }
        }

        int brickW = (maxC - minC + 1) * mini;
        int brickH = (maxR - minR + 1) * mini;
        float pieceX = panelX + (panelW - brickW) / 2.f;

        float pieceY = rowY + (p == 0 ? 8.f : 0.f);

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (shapes[p][r][c]) {
                    drawTile3D(window, pieceX + (c - minC) * mini, pieceY + (r - minR) * mini, mini, pieces[p]);
                }
            }
        }

        char countStr[8];
        snprintf(countStr, 8, "%05d", pieceCount[p]);
        sf::Text countText(font, countStr, 26);
        countText.setFillColor(getColor(pieces[p]));
        float countW = countText.getLocalBounds().size.x;
        countText.setPosition({panelX + (panelW - countW) / 2.f, pieceY + brickH + 4.f});
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
    ui.pad  = 12.f;
    ui.boxW = ui.w - 2.f * ui.pad;

    const float left = ui.x + ui.pad;

    const float gridH = 200.f;

    ui.holdBox  = sf::FloatRect({left, 12.f}, {(ui.boxW - 10.f) / 2.f, (gridH - 10.f) / 2.f});

    ui.nextBox  = sf::FloatRect({left, 12.f}, {ui.boxW, gridH});

    float midY = 224.f;
    const float boxH = 72.f;
    const float boxGap = 10.f;
    ui.topScoreBox = sf::FloatRect({left, midY},                          {ui.boxW, boxH});
    ui.scoreBox    = sf::FloatRect({left, midY + (boxH + boxGap)},        {ui.boxW, boxH});
    ui.levelBox    = sf::FloatRect({left, midY + 2*(boxH + boxGap)},      {ui.boxW, boxH});
    ui.linesBox    = sf::FloatRect({left, midY + 3*(boxH + boxGap)},      {ui.boxW, boxH});

    float gameInfoY = midY + 4*(boxH + boxGap);
    ui.statsBox = sf::FloatRect({left, gameInfoY}, {ui.boxW, WINDOW_H - gameInfoY - 12.f});
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
    int mini = 18;

    float startX = ui.holdBox.position.x + (ui.holdBox.size.x - cellsW * mini) * 0.5f;
    float startY = ui.holdBox.position.y + 42.f + (ui.holdBox.size.y - 52.f - cellsH * mini) * 0.5f;

    for (int r = minR; r <= maxR; r++) {
        for (int c = minC; c <= maxC; c++) {
            if (p->shape[r][c] != ' ') {

                char tileChar = canHold ? p->shape[r][c] : '#';
                drawTile3D(window, startX + (c - minC) * mini, startY + (r - minR) * mini,
                          mini, tileChar);
            }
        }
    }
}

void drawSidebar(sf::RenderWindow& window, const SidebarUI& ui,
                 const sf::Font& font, int score, int level, int lines,
                 const Piece* next, Piece* const nextQueue[], const Piece* hold) {

/** Render sidebar */
    sf::RectangleShape bg({ui.w, ui.h});
    bg.setPosition({ui.x, ui.y});
    bg.setFillColor(sf::Color(25, 25, 40));
    window.draw(bg);

    float gridW = ui.boxW;
    float gridH = ui.nextBox.size.y;
    float cellW = (gridW - 10.f) / 2.f;
    float cellH = (gridH - 10.f) / 2.f;
    float gap = 10.f;
    float gridLeft = ui.nextBox.position.x;
    float gridTop = ui.nextBox.position.y;

    drawPanel(window, sf::FloatRect({gridLeft, gridTop}, {cellW, cellH}));

    sf::ConvexShape jShape(8);
    jShape.setPoint(0, {gridLeft + cellW + gap, gridTop});
    jShape.setPoint(1, {gridLeft + gridW, gridTop});
    jShape.setPoint(2, {gridLeft + gridW, gridTop + gridH});
    jShape.setPoint(3, {gridLeft, gridTop + gridH});
    jShape.setPoint(4, {gridLeft, gridTop + cellH + gap});
    jShape.setPoint(5, {gridLeft + cellW, gridTop + cellH + gap});
    jShape.setPoint(6, {gridLeft + cellW, gridTop + cellH});
    jShape.setPoint(7, {gridLeft + cellW + gap, gridTop + cellH});

    jShape.setFillColor(sf::Color(15, 15, 25));
    jShape.setOutlineThickness(3.f);
    jShape.setOutlineColor(sf::Color(80, 80, 120));
    window.draw(jShape);

    drawPanel(window, ui.topScoreBox);
    drawPanel(window, ui.scoreBox);
    drawPanel(window, ui.levelBox);
    drawPanel(window, ui.linesBox);
    drawPanel(window, ui.statsBox);

    const unsigned int labelSize = 26;
    const unsigned int valueSize = 26;
    const float pad = 10.f;

    sf::Text holdLabel(font, "HOLD", 28);
    holdLabel.setFillColor(sf::Color(100, 200, 255));
    float holdLabelW = holdLabel.getLocalBounds().size.x;
    float holdLabelX = gridLeft + (cellW - holdLabelW) * 0.5f;
    holdLabel.setPosition({holdLabelX, gridTop + pad});
    window.draw(holdLabel);
    drawHoldPreview(window, ui, hold);

    sf::Text nextLabel(font, "NEXT", 28);
    nextLabel.setFillColor(sf::Color(255, 100, 100));
    float nextLabelW = nextLabel.getLocalBounds().size.x;
    float nextLabelX = gridLeft + cellW + gap + (cellW - nextLabelW) * 0.5f;
    nextLabel.setPosition({nextLabelX, gridTop + pad});
    window.draw(nextLabel);

    const Piece* gridNext[3] = {next, nextQueue[0], nextQueue[1]};
/** Process setPosition */
    float nextX[3] = {
        gridLeft + cellW + gap,
        gridLeft + cellW + gap,
        gridLeft
    };
    float nextY[3] = {
        gridTop,
        gridTop + cellH + gap,
        gridTop + cellH + gap
    };

    for (int p = 0; p < 3; p++) {
        if (gridNext[p]) {
            int minR = 4, minC = 4, maxR = -1, maxC = -1;
            for (int r = 0; r < 4; r++) {
                for (int c = 0; c < 4; c++) {
                    if (gridNext[p]->shape[r][c] != ' ') {
                        minR = std::min(minR, r); minC = std::min(minC, c);
                        maxR = std::max(maxR, r); maxC = std::max(maxC, c);
                    }
                }
            }
            if (maxR != -1) {
                int cellsW = maxC - minC + 1;
                int cellsH = maxR - minR + 1;
                int mini = 16;

                float startX = nextX[p] + (cellW - cellsW * mini) * 0.5f;
                float labelOffset = (p == 0) ? 24.f : 0.f;
                float startY = nextY[p] + labelOffset + (cellH - labelOffset - cellsH * mini) * 0.5f;

                for (int r = minR; r <= maxR; r++) {
                    for (int c = minC; c <= maxC; c++) {
                        if (gridNext[p]->shape[r][c] != ' ') {
                            drawTile3D(window, startX + (c - minC) * mini, startY + (r - minR) * mini,
                                      mini, gridNext[p]->shape[r][c]);
                        }
                    }
                }
            }
        }
    }

    sf::Text topScoreLabel(font, "TOP SCORE", labelSize);
    topScoreLabel.setFillColor(sf::Color(255, 200, 100));
    topScoreLabel.setPosition({ui.topScoreBox.position.x + pad, ui.topScoreBox.position.y + pad});
    window.draw(topScoreLabel);
    char topScoreStr[16];
    snprintf(topScoreStr, 16, "%09d", highScore);
    sf::Text topScoreVal(font, topScoreStr, valueSize);
    topScoreVal.setFillColor(sf::Color(255, 200, 100));
    float topScoreValH = topScoreVal.getLocalBounds().size.y;
    topScoreVal.setPosition({ui.topScoreBox.position.x + pad, ui.topScoreBox.position.y + ui.topScoreBox.size.y - pad - topScoreValH - 4.f});
    window.draw(topScoreVal);

    sf::Text scoreLabel(font, "SCORE", labelSize);
    scoreLabel.setFillColor(sf::Color(255, 150, 100));
    scoreLabel.setPosition({ui.scoreBox.position.x + pad, ui.scoreBox.position.y + pad});
    window.draw(scoreLabel);
    char scoreStr[16];
    snprintf(scoreStr, 16, "%09d", score);
    sf::Text scoreVal(font, scoreStr, valueSize);
    scoreVal.setFillColor(sf::Color(200, 200, 200));
    float scoreValH = scoreVal.getLocalBounds().size.y;
    scoreVal.setPosition({ui.scoreBox.position.x + pad, ui.scoreBox.position.y + ui.scoreBox.size.y - pad - scoreValH - 4.f});
    window.draw(scoreVal);

    sf::Text levelLabel(font, "LEVEL", labelSize);
    levelLabel.setFillColor(sf::Color(100, 255, 100));
    levelLabel.setPosition({ui.levelBox.position.x + pad, ui.levelBox.position.y + pad});
    window.draw(levelLabel);
    char levelStr[16];
    snprintf(levelStr, 16, "%09d", level);
    sf::Text levelVal(font, levelStr, valueSize);
    levelVal.setFillColor(sf::Color(200, 200, 200));
    float levelValH = levelVal.getLocalBounds().size.y;
    levelVal.setPosition({ui.levelBox.position.x + pad, ui.levelBox.position.y + ui.levelBox.size.y - pad - levelValH - 4.f});
    window.draw(levelVal);

    sf::Text linesLabel(font, "LINES", labelSize);
    linesLabel.setFillColor(sf::Color(255, 100, 255));
    linesLabel.setPosition({ui.linesBox.position.x + pad, ui.linesBox.position.y + pad});
    window.draw(linesLabel);
    char linesStr[16];
    snprintf(linesStr, 16, "%09d", lines);
    sf::Text linesVal(font, linesStr, valueSize);
    linesVal.setFillColor(sf::Color(200, 200, 200));
    float linesValH = linesVal.getLocalBounds().size.y;
    linesVal.setPosition({ui.linesBox.position.x + pad, ui.linesBox.position.y + ui.linesBox.size.y - pad - linesValH - 4.f});
    window.draw(linesVal);

    float infoLabelX = ui.statsBox.position.x + pad;
    sf::Text statsLabel(font, "GAME INFO", 24);
    statsLabel.setFillColor(sf::Color(150, 200, 255));
    statsLabel.setPosition({infoLabelX, ui.statsBox.position.y + pad});
    window.draw(statsLabel);

    const unsigned int infoSize = 18;
    float infoY = ui.statsBox.position.y + pad + 30.f;
    float lineHeight = 22.f;

    int minutes = static_cast<int>(playTime) / 60;
    int seconds = static_cast<int>(playTime) % 60;
    char timeStr[20];
    snprintf(timeStr, 20, "Time: %02d:%02d", minutes, seconds);
    sf::Text timeText(font, timeStr, infoSize);
    timeText.setFillColor(sf::Color(200, 200, 200));
    timeText.setPosition({infoLabelX, infoY});
    window.draw(timeText);
    infoY += lineHeight;

    sf::Text piecesText(font, "Pieces: " + std::to_string(totalPieces), infoSize);
    piecesText.setFillColor(sf::Color(200, 200, 200));
    piecesText.setPosition({infoLabelX, infoY});
    window.draw(piecesText);
    infoY += lineHeight;

    float ppm = (playTime > 0) ? (totalPieces / playTime * 60.f) : 0.f;
    char ppmStr[20];
    snprintf(ppmStr, 20, "PPM: %.1f", ppm);
    sf::Text ppmText(font, ppmStr, infoSize);
    ppmText.setFillColor(sf::Color(200, 200, 200));
    ppmText.setPosition({infoLabelX, infoY});
    window.draw(ppmText);
    infoY += lineHeight;

    float lpm = (playTime > 0) ? (lines / playTime * 60.f) : 0.f;
    char lpmStr[20];
    snprintf(lpmStr, 20, "LPM: %.1f", lpm);
    sf::Text lpmText(font, lpmStr, infoSize);
    lpmText.setFillColor(sf::Color(200, 200, 200));
    lpmText.setPosition({infoLabelX, infoY});
    window.draw(lpmText);
    infoY += lineHeight;

    sf::Text tetrisText(font, "Tetris: " + std::to_string(tetrisCount), infoSize);
    tetrisText.setFillColor(sf::Color(0, 240, 240));
    tetrisText.setPosition({infoLabelX, infoY});
    window.draw(tetrisText);
    infoY += lineHeight;

    sf::Text tspinText(font, "T-Spin: " + std::to_string(tSpinCount), infoSize);
    tspinText.setFillColor(sf::Color(200, 100, 255));
    tspinText.setPosition({infoLabelX, infoY});
    window.draw(tspinText);
    infoY += lineHeight;

    static int maxCombo = 0;
    if (comboCount > maxCombo) maxCombo = comboCount;
    sf::Text maxComboText(font, "Max Combo: " + std::to_string(maxCombo), infoSize);
    maxComboText.setFillColor(sf::Color(255, 150, 100));
    maxComboText.setPosition({infoLabelX, infoY});
    window.draw(maxComboText);
    infoY += lineHeight;

    if (backToBackActive) {
        sf::Text b2bText(font, "B2B Active!", infoSize);
        b2bText.setFillColor(sf::Color(255, 255, 0));
        b2bText.setPosition({infoLabelX, infoY});
        window.draw(b2bText);
    }
}

/** Process setPosition */
void drawSettingsScreen(sf::RenderWindow& window, const sf::Font& font) {

    Text settingsTitle(font);
    settingsTitle.setString("SETTINGS");
    settingsTitle.setCharacterSize(70);
    settingsTitle.setFillColor(Color::Cyan);
    float settingsTitleW = settingsTitle.getLocalBounds().size.x;
    settingsTitle.setPosition(sf::Vector2f{(WINDOW_W - settingsTitleW) / 2.f, 40.f});
    window.draw(settingsTitle);

    const float labelX = 60.f;
    const float arrowLeftX = 320.f;
    const float sliderX = 350.f;
    const float sliderW = 280.f;
    const float sliderH = 30.f;
    const float arrowRightX = 640.f;
    const float valueX = 680.f;
    const float checkboxX = 330.f;

    const float row1Y = 140.f;
    const float row2Y = 225.f;
    const float row3Y = 310.f;
    const float row4Y = 395.f;
    const float row5Y = 480.f;
    const float row6Y = 565.f;
    const float backY = 700.f;

    Text musicLabel(font);
    musicLabel.setString("Music Volume");
    musicLabel.setCharacterSize(28);
    musicLabel.setFillColor(Color::White);
    musicLabel.setPosition(sf::Vector2f{labelX, row1Y});
    window.draw(musicLabel);

    Text musicLeftArrow(font);
    musicLeftArrow.setString("<");
    musicLeftArrow.setCharacterSize(32);
    musicLeftArrow.setFillColor(Color::Yellow);
    musicLeftArrow.setPosition(sf::Vector2f{arrowLeftX, row1Y - 2.f});
    window.draw(musicLeftArrow);

    RectangleShape musicSliderBg(Vector2f(sliderW, sliderH));
    musicSliderBg.setPosition(sf::Vector2f{sliderX, row1Y + 3.f});
    musicSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(musicSliderBg);

    RectangleShape musicSliderFill(Vector2f(musicVolume * 2.8f, sliderH));
    musicSliderFill.setPosition(sf::Vector2f{sliderX, row1Y + 3.f});
    musicSliderFill.setFillColor(Color(0, 150, 255));
    window.draw(musicSliderFill);

    Text musicRightArrow(font);
    musicRightArrow.setString(">");
    musicRightArrow.setCharacterSize(32);
    musicRightArrow.setFillColor(Color::Yellow);
    musicRightArrow.setPosition(sf::Vector2f{arrowRightX, row1Y - 2.f});
    window.draw(musicRightArrow);

    Text musicValue(font);
    musicValue.setString(std::to_string((int)musicVolume) + "%");
    musicValue.setCharacterSize(24);
    musicValue.setFillColor(Color::White);
    musicValue.setPosition(sf::Vector2f{valueX, row1Y + 3.f});
    window.draw(musicValue);

    Text sfxLabel(font);
    sfxLabel.setString("SFX Volume");
    sfxLabel.setCharacterSize(28);
    sfxLabel.setFillColor(Color::White);
    sfxLabel.setPosition(sf::Vector2f{labelX, row2Y});
    window.draw(sfxLabel);

    Text sfxLeftArrow(font);
    sfxLeftArrow.setString("<");
    sfxLeftArrow.setCharacterSize(32);
    sfxLeftArrow.setFillColor(Color::Yellow);
    sfxLeftArrow.setPosition(sf::Vector2f{arrowLeftX, row2Y - 2.f});
    window.draw(sfxLeftArrow);

    RectangleShape sfxSliderBg(Vector2f(sliderW, sliderH));
    sfxSliderBg.setPosition(sf::Vector2f{sliderX, row2Y + 3.f});
    sfxSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(sfxSliderBg);

    RectangleShape sfxSliderFill(Vector2f(sfxVolume * 2.8f, sliderH));
    sfxSliderFill.setPosition(sf::Vector2f{sliderX, row2Y + 3.f});
    sfxSliderFill.setFillColor(Color(0, 200, 100));
    window.draw(sfxSliderFill);

    Text sfxRightArrow(font);
    sfxRightArrow.setString(">");
    sfxRightArrow.setCharacterSize(32);
    sfxRightArrow.setFillColor(Color::Yellow);
    sfxRightArrow.setPosition(sf::Vector2f{arrowRightX, row2Y - 2.f});
    window.draw(sfxRightArrow);

    Text sfxValue(font);
    sfxValue.setString(std::to_string((int)sfxVolume) + "%");
    sfxValue.setCharacterSize(24);
    sfxValue.setFillColor(Color::White);
    sfxValue.setPosition(sf::Vector2f{valueX, row2Y + 3.f});
    window.draw(sfxValue);

    Text brightnessLabel(font);
    brightnessLabel.setString("Brightness");
    brightnessLabel.setCharacterSize(28);
    brightnessLabel.setFillColor(Color::White);
    brightnessLabel.setPosition(sf::Vector2f{labelX, row3Y});
    window.draw(brightnessLabel);

    Text brightLeftArrow(font);
    brightLeftArrow.setString("<");
    brightLeftArrow.setCharacterSize(32);
    brightLeftArrow.setFillColor(Color::Yellow);
    brightLeftArrow.setPosition(sf::Vector2f{arrowLeftX, row3Y - 2.f});
    window.draw(brightLeftArrow);

    RectangleShape brightnessSliderBg(Vector2f(sliderW, sliderH));
    brightnessSliderBg.setPosition(sf::Vector2f{sliderX, row3Y + 3.f});
    brightnessSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(brightnessSliderBg);

    RectangleShape brightnessSliderFill(Vector2f(((brightness - 51.f) / (255.f - 51.f)) * sliderW, sliderH));
    brightnessSliderFill.setPosition(sf::Vector2f{sliderX, row3Y + 3.f});
    brightnessSliderFill.setFillColor(Color(255, 200, 50));
    window.draw(brightnessSliderFill);

    Text brightRightArrow(font);
    brightRightArrow.setString(">");
    brightRightArrow.setCharacterSize(32);
    brightRightArrow.setFillColor(Color::Yellow);
    brightRightArrow.setPosition(sf::Vector2f{arrowRightX, row3Y - 2.f});
    window.draw(brightRightArrow);

    Text brightnessValue(font);
    float brightnessPercent = ((brightness - 51.f) / (255.f - 51.f)) * 80.f + 20.f;
    brightnessValue.setString(std::to_string((int)brightnessPercent) + "%");
    brightnessValue.setCharacterSize(24);
    brightnessValue.setFillColor(Color::White);
    brightnessValue.setPosition(sf::Vector2f{valueX, row3Y + 3.f});
    window.draw(brightnessValue);

    Text dasLabel1(font);
    dasLabel1.setString("Delayed Auto");
    dasLabel1.setCharacterSize(28);
    dasLabel1.setFillColor(Color::White);
    dasLabel1.setPosition(sf::Vector2f{labelX, row4Y});
    window.draw(dasLabel1);

    Text dasLabel2(font);
    dasLabel2.setString("Shift");
    dasLabel2.setCharacterSize(28);
    dasLabel2.setFillColor(Color::White);
    dasLabel2.setPosition(sf::Vector2f{labelX, row4Y + 30.f});
    window.draw(dasLabel2);

    float dasSliderY = row4Y + 18.f;
    Text dasLeftArrow(font);
    dasLeftArrow.setString("<");
    dasLeftArrow.setCharacterSize(32);
    dasLeftArrow.setFillColor(Color::Yellow);
    dasLeftArrow.setPosition(sf::Vector2f{arrowLeftX, dasSliderY - 2.f});
    window.draw(dasLeftArrow);

    RectangleShape dasSliderBg(Vector2f(sliderW, sliderH));
    dasSliderBg.setPosition(sf::Vector2f{sliderX, dasSliderY + 3.f});
    dasSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(dasSliderBg);

    float dasMs = DAS_DELAY * 1000.f;
    float dasRatio = (dasMs - 100.f) / 100.f;
    RectangleShape dasSliderFill(Vector2f(dasRatio * sliderW, sliderH));
    dasSliderFill.setPosition(sf::Vector2f{sliderX, dasSliderY + 3.f});
    dasSliderFill.setFillColor(Color(150, 100, 255));
    window.draw(dasSliderFill);

    Text dasRightArrow(font);
    dasRightArrow.setString(">");
    dasRightArrow.setCharacterSize(32);
    dasRightArrow.setFillColor(Color::Yellow);
    dasRightArrow.setPosition(sf::Vector2f{arrowRightX, dasSliderY - 2.f});
    window.draw(dasRightArrow);

    Text dasValue(font);
    dasValue.setString(std::to_string((int)dasMs) + "ms");
    dasValue.setCharacterSize(24);
    dasValue.setFillColor(Color::White);
    dasValue.setPosition(sf::Vector2f{valueX, dasSliderY + 3.f});
    window.draw(dasValue);

    Text arrLabel1(font);
    arrLabel1.setString("Auto Repeat");
    arrLabel1.setCharacterSize(28);
    arrLabel1.setFillColor(Color::White);
    arrLabel1.setPosition(sf::Vector2f{labelX, row5Y});
    window.draw(arrLabel1);

    Text arrLabel2(font);
    arrLabel2.setString("Rate");
    arrLabel2.setCharacterSize(28);
    arrLabel2.setFillColor(Color::White);
    arrLabel2.setPosition(sf::Vector2f{labelX, row5Y + 30.f});
    window.draw(arrLabel2);

    float arrSliderY = row5Y + 18.f;
    Text arrLeftArrow(font);
    arrLeftArrow.setString("<");
    arrLeftArrow.setCharacterSize(32);
    arrLeftArrow.setFillColor(Color::Yellow);
    arrLeftArrow.setPosition(sf::Vector2f{arrowLeftX, arrSliderY - 2.f});
    window.draw(arrLeftArrow);

    RectangleShape arrSliderBg(Vector2f(sliderW, sliderH));
    arrSliderBg.setPosition(sf::Vector2f{sliderX, arrSliderY + 3.f});
    arrSliderBg.setFillColor(Color(80, 80, 80));
    window.draw(arrSliderBg);

    float arrMs = ARR_DELAY * 1000.f;
    float arrRatio = arrMs / 50.f;
    RectangleShape arrSliderFill(Vector2f(arrRatio * sliderW, sliderH));
    arrSliderFill.setPosition(sf::Vector2f{sliderX, arrSliderY + 3.f});
    arrSliderFill.setFillColor(Color(255, 150, 100));
    window.draw(arrSliderFill);

    Text arrRightArrow(font);
    arrRightArrow.setString(">");
    arrRightArrow.setCharacterSize(32);
    arrRightArrow.setFillColor(Color::Yellow);
    arrRightArrow.setPosition(sf::Vector2f{arrowRightX, arrSliderY - 2.f});
    window.draw(arrRightArrow);

    Text arrValue(font);
    arrValue.setString(std::to_string((int)arrMs) + "ms");
    arrValue.setCharacterSize(24);
    arrValue.setFillColor(Color::White);
    arrValue.setPosition(sf::Vector2f{valueX, arrSliderY + 3.f});
    window.draw(arrValue);

    Text ghostLabel(font);
    ghostLabel.setString("Ghost Piece");
    ghostLabel.setCharacterSize(28);
    ghostLabel.setFillColor(Color::White);
    ghostLabel.setPosition(sf::Vector2f{labelX, row6Y});
    window.draw(ghostLabel);

    RectangleShape checkBox(Vector2f(35, 35));
    checkBox.setPosition(sf::Vector2f{checkboxX, row6Y - 2.f});
    checkBox.setFillColor(Color(80, 80, 80));
    checkBox.setOutlineThickness(3.f);
    checkBox.setOutlineColor(Color::White);
    window.draw(checkBox);

    if (ghostPieceEnabled) {
        Text checkMark(font);
        checkMark.setString("X");
        checkMark.setCharacterSize(28);
        checkMark.setFillColor(Color::Green);
        checkMark.setPosition(sf::Vector2f{checkboxX + 7.f, row6Y - 2.f});
        window.draw(checkMark);
    }

    Text ghostStatus(font);
    ghostStatus.setString(ghostPieceEnabled ? "ON" : "OFF");
    ghostStatus.setCharacterSize(24);
    ghostStatus.setFillColor(ghostPieceEnabled ? Color::Green : Color::Red);
    ghostStatus.setPosition(sf::Vector2f{checkboxX + 50.f, row6Y + 3.f});
    window.draw(ghostStatus);

    const float backBtnW = 280.f;
    const float backBtnH = 65.f;
    const float backBtnX = (WINDOW_W - backBtnW) / 2.f;
    RectangleShape backBtn(Vector2f(backBtnW, backBtnH));
    backBtn.setPosition(sf::Vector2f{backBtnX, backY});
    backBtn.setFillColor(Color(100, 100, 100));
    window.draw(backBtn);

    Text backText(font);
    backText.setString("BACK");
    backText.setCharacterSize(32);
    backText.setFillColor(Color::White);
    float backTxtW = backText.getLocalBounds().size.x;
    backText.setPosition(sf::Vector2f{backBtnX + (backBtnW - backTxtW) / 2.f, backY + 12.f});
    window.draw(backText);
}

/** Process getLocalBounds */
void handleSettingsClick(sf::Vector2i mousePos) {

    const float arrowLeftX = 320.f;
    const float sliderX = 350.f;
    const float sliderW = 280.f;
    const float arrowRightX = 640.f;
    const float checkboxX = 330.f;

    const float row1Y = 140.f;
    const float row2Y = 225.f;
    const float row3Y = 310.f;
    const float row4Y = 395.f;
    const float row5Y = 480.f;
    const float row6Y = 565.f;

    auto inRange = [&](float minX, float maxX, float minY, float maxY) {
        return mousePos.x >= minX && mousePos.x <= maxX && mousePos.y >= minY && mousePos.y <= maxY;
    };

    if (inRange(arrowLeftX, arrowLeftX + 30, row1Y - 10, row1Y + 40)) {
        musicVolume = std::max(0.f, musicVolume - 5.f);
        Audio::setMusicVolume(musicVolume);
        Audio::playSettingClick();
    }
    if (inRange(arrowRightX, arrowRightX + 30, row1Y - 10, row1Y + 40)) {
        musicVolume = std::min(100.f, musicVolume + 5.f);
/** Process setMusicVolume */
        Audio::setMusicVolume(musicVolume);
/** Process playSettingClick */
        Audio::playSettingClick();
    }
    if (inRange(sliderX, sliderX + sliderW, row1Y, row1Y + 35)) {
        musicVolume = (mousePos.x - sliderX) / sliderW * 100.f;
        musicVolume = std::max(0.f, std::min(100.f, musicVolume));
/** Process playSettingClick */
        Audio::setMusicVolume(musicVolume);
        Audio::playSettingClick();
    }

    if (inRange(arrowLeftX, arrowLeftX + 30, row2Y - 10, row2Y + 40)) {
        sfxVolume = std::max(0.f, sfxVolume - 5.f);
/** Process playSettingClick */
        Audio::setSfxVolume(sfxVolume);
        Audio::playSettingClick();
    }
    if (inRange(arrowRightX, arrowRightX + 30, row2Y - 10, row2Y + 40)) {
        sfxVolume = std::min(100.f, sfxVolume + 5.f);
/** Process setSfxVolume */
        Audio::setSfxVolume(sfxVolume);
/** Process playSettingClick */
        Audio::playSettingClick();
    }
    if (inRange(sliderX, sliderX + sliderW, row2Y, row2Y + 35)) {
        sfxVolume = (mousePos.x - sliderX) / sliderW * 100.f;
        sfxVolume = std::max(0.f, std::min(100.f, sfxVolume));
/** Process playSettingClick */
        Audio::setSfxVolume(sfxVolume);
        Audio::playSettingClick();
    }

    if (inRange(arrowLeftX, arrowLeftX + 30, row3Y - 10, row3Y + 40)) {
        brightness = std::max(51.f, brightness - 10.f);
/** Process playSettingClick */
        Audio::playSettingClick();
    }
    if (inRange(arrowRightX, arrowRightX + 30, row3Y - 10, row3Y + 40)) {
        brightness = std::min(255.f, brightness + 10.f);
/** Process max */
        Audio::playSettingClick();
    }
    if (inRange(sliderX, sliderX + sliderW, row3Y, row3Y + 35)) {
        float ratio = (mousePos.x - sliderX) / sliderW;
        brightness = 51.f + ratio * (255.f - 51.f);
        brightness = std::max(51.f, std::min(255.f, brightness));
        Audio::playSettingClick();
    }

    float dasSliderY = row4Y + 18.f;
    if (inRange(arrowLeftX, arrowLeftX + 30, dasSliderY - 10, dasSliderY + 40)) {
        float dasMs = DAS_DELAY * 1000.f;
        dasMs = std::max(100.f, dasMs - 5.f);
        DAS_DELAY = dasMs / 1000.f;
        Audio::playSettingClick();
    }
    if (inRange(arrowRightX, arrowRightX + 30, dasSliderY - 10, dasSliderY + 40)) {
        float dasMs = DAS_DELAY * 1000.f;
        dasMs = std::min(200.f, dasMs + 5.f);
        DAS_DELAY = dasMs / 1000.f;
        Audio::playSettingClick();
    }
    if (inRange(sliderX, sliderX + sliderW, dasSliderY, dasSliderY + 35)) {
        float ratio = (mousePos.x - sliderX) / sliderW;
        float dasMs = 100.f + ratio * 100.f;
        dasMs = std::max(100.f, std::min(200.f, dasMs));
        DAS_DELAY = dasMs / 1000.f;
        Audio::playSettingClick();
    }

    float arrSliderY = row5Y + 18.f;
    if (inRange(arrowLeftX, arrowLeftX + 30, arrSliderY - 10, arrSliderY + 40)) {
        float arrMs = ARR_DELAY * 1000.f;
        arrMs = std::max(0.f, arrMs - 5.f);
        ARR_DELAY = arrMs / 1000.f;
        Audio::playSettingClick();
    }
    if (inRange(arrowRightX, arrowRightX + 30, arrSliderY - 10, arrSliderY + 40)) {
        float arrMs = ARR_DELAY * 1000.f;
        arrMs = std::min(50.f, arrMs + 5.f);
        ARR_DELAY = arrMs / 1000.f;
        Audio::playSettingClick();
    }
    if (inRange(sliderX, sliderX + sliderW, arrSliderY, arrSliderY + 35)) {
        float ratio = (mousePos.x - sliderX) / sliderW;
        float arrMs = ratio * 50.f;
        arrMs = std::max(0.f, std::min(50.f, arrMs));
        ARR_DELAY = arrMs / 1000.f;
        Audio::playSettingClick();
    }

    if (inRange(checkboxX, checkboxX + 35, row6Y - 5, row6Y + 40)) {
        ghostPieceEnabled = !ghostPieceEnabled;
        if (ghostPieceEnabled) {
            Audio::playToggleOn();
        } else {
            Audio::playToggleOff();
        }
    }
}

/** Process playToggleOff */
void drawGameOverScreen(sf::RenderWindow& window, const sf::Font& font) {

    RectangleShape overlay(Vector2f(WINDOW_W, WINDOW_H));
    overlay.setFillColor(Color(0, 0, 0, 200));
    window.draw(overlay);

    const float fullW = WINDOW_W;
    Text gameOverText(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(70);
    gameOverText.setFillColor(Color::Red);
    float goWidth = gameOverText.getLocalBounds().size.x;
    gameOverText.setPosition(sf::Vector2f{(fullW - goWidth) / 2.f, 220.f});
    window.draw(gameOverText);

    const float goBtnW = 280.f;
    const float goBtnH = 65.f;
    const float goBtnX = (fullW - goBtnW) / 2.f;

    RectangleShape restartBtn(Vector2f(goBtnW, goBtnH));
    restartBtn.setPosition(sf::Vector2f{goBtnX, 340.f});
    restartBtn.setFillColor(Color(0, 100, 255));
    window.draw(restartBtn);
    Text restartText(font);
    restartText.setString("RESTART");
    restartText.setCharacterSize(32);
    restartText.setFillColor(Color::White);
    float restartTxtW = restartText.getLocalBounds().size.x;
    restartText.setPosition(sf::Vector2f{goBtnX + (goBtnW - restartTxtW) / 2.f, 352.f});
    window.draw(restartText);

    RectangleShape menuBtn(Vector2f(goBtnW, goBtnH));
    menuBtn.setPosition(sf::Vector2f{goBtnX, 430.f});
    menuBtn.setFillColor(Color(100, 100, 100));
    window.draw(menuBtn);
    Text menuText(font);
    menuText.setString("MENU");
    menuText.setCharacterSize(32);
    menuText.setFillColor(Color::White);
    float menuTxtW = menuText.getLocalBounds().size.x;
    menuText.setPosition(sf::Vector2f{goBtnX + (goBtnW - menuTxtW) / 2.f, 442.f});
    window.draw(menuText);

    RectangleShape exitBtn(Vector2f(goBtnW, goBtnH));
    exitBtn.setPosition(sf::Vector2f{goBtnX, 520.f});
    exitBtn.setFillColor(Color(255, 50, 50));
    window.draw(exitBtn);
    Text exitText(font);
    exitText.setString("EXIT");
    exitText.setCharacterSize(32);
    exitText.setFillColor(Color::White);
    float exitTxtW = exitText.getLocalBounds().size.x;
    exitText.setPosition(sf::Vector2f{goBtnX + (goBtnW - exitTxtW) / 2.f, 532.f});
    window.draw(exitText);
}

/** Process getLocalBounds */
void drawBrightnessOverlay(sf::RenderWindow& window) {
    if (brightness < 255.f) {
        RectangleShape darkenOverlay(Vector2f(WINDOW_W, WINDOW_H));
        darkenOverlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(255 - brightness)));
        window.draw(darkenOverlay);
    }
}

/** Process setFillColor */
void drawMenu(sf::RenderWindow& window, const sf::Font& font) {
    const float fullW = WINDOW_W;

    Text title(font, "TETRIS", 80);
    title.setFillColor(Color::Cyan);
    title.setStyle(Text::Bold);
    float titleW = title.getLocalBounds().size.x;
    title.setPosition({(fullW - titleW) / 2.f, 100.f});
    window.draw(title);

    const float btnW = 280.f;
    const float btnH = 65.f;
    const float btnX = (fullW - btnW) / 2.f;

    Text diffLabel(font, "DIFFICULTY", 24);
    diffLabel.setFillColor(Color::White);
    float diffLabelW = diffLabel.getLocalBounds().size.x;
    diffLabel.setPosition({(fullW - diffLabelW) / 2.f, 230.f});
    window.draw(diffLabel);

    const char* diffNames[] = {"EASY", "NORMAL", "HARD"};
    Color diffColors[] = {Color(0, 150, 0), Color(255, 200, 50), Color(180, 0, 0)};
    float diffBtnW = 100.f;
    float diffStartX = (fullW - 3 * diffBtnW - 24.f) / 2.f;

    for (int i = 0; i < 3; i++) {
        RectangleShape diffBtn({diffBtnW, 45.f});
        diffBtn.setPosition({diffStartX + i * (diffBtnW + 12.f), 270.f});
        bool selected = (static_cast<int>(difficulty) == i);
        diffBtn.setFillColor(selected ? diffColors[i] : Color(60, 60, 60));
        diffBtn.setOutlineThickness(selected ? 3.f : 1.f);
        diffBtn.setOutlineColor(selected ? Color::White : Color(100, 100, 100));
        window.draw(diffBtn);

        Text diffText(font, diffNames[i], 18);
        diffText.setFillColor(Color::White);
        float dtW = diffText.getLocalBounds().size.x;
        diffText.setPosition({diffStartX + i * (diffBtnW + 12.f) + (diffBtnW - dtW) / 2.f, 280.f});
        window.draw(diffText);
    }

    RectangleShape startBtn({btnW, btnH});
    startBtn.setPosition({btnX, 360.f});
    startBtn.setFillColor(Color(0, 150, 0));
    window.draw(startBtn);
    Text startText(font, "START", 32);
    startText.setFillColor(Color::White);
    float startTxtW = startText.getLocalBounds().size.x;
    startText.setPosition({btnX + (btnW - startTxtW) / 2.f, 372.f});
    window.draw(startText);

    RectangleShape howToBtn({btnW, btnH});
    howToBtn.setPosition({btnX, 445.f});
    howToBtn.setFillColor(Color(0, 100, 180));
    window.draw(howToBtn);
    Text howToText(font, "HOW TO PLAY", 32);
    howToText.setFillColor(Color::White);
    float howToTxtW = howToText.getLocalBounds().size.x;
    howToText.setPosition({btnX + (btnW - howToTxtW) / 2.f, 457.f});
    window.draw(howToText);

    RectangleShape settingBtn({btnW, btnH});
    settingBtn.setPosition({btnX, 530.f});
    settingBtn.setFillColor(Color(100, 100, 100));
    window.draw(settingBtn);
    Text settingText(font, "SETTINGS", 32);
    settingText.setFillColor(Color::White);
    float settingTxtW = settingText.getLocalBounds().size.x;
    settingText.setPosition({btnX + (btnW - settingTxtW) / 2.f, 542.f});
    window.draw(settingText);

    RectangleShape exitBtn({btnW, btnH});
    exitBtn.setPosition({btnX, 615.f});
    exitBtn.setFillColor(Color(200, 0, 0));
    window.draw(exitBtn);
    Text exitText(font, "EXIT", 32);
    exitText.setFillColor(Color::White);
    float exitTxtW = exitText.getLocalBounds().size.x;
    exitText.setPosition({btnX + (btnW - exitTxtW) / 2.f, 627.f});
    window.draw(exitText);

    Text controlsText(font, "Press F11 for Fullscreen", 14);
    controlsText.setFillColor(Color(150, 150, 150));
    float ctrlW = controlsText.getLocalBounds().size.x;
    controlsText.setPosition({(fullW - ctrlW) / 2.f, 720.f});
    window.draw(controlsText);
}

/** Process getLocalBounds */
void handleMenuClick(sf::Vector2i mousePos, GameState& state, GameState& previousState, bool& shouldClose) {
    const float fullW = WINDOW_W;
    const float btnW = 280.f;
    const float btnX = (fullW - btnW) / 2.f;

    float diffBtnW = 100.f;
    float diffStartX = (fullW - 3 * diffBtnW - 24.f) / 2.f;
    for (int i = 0; i < 3; i++) {
        float bx = diffStartX + i * (diffBtnW + 12.f);
        if (mousePos.x >= bx && mousePos.x <= bx + diffBtnW &&
            mousePos.y >= 270 && mousePos.y <= 315) {
            difficulty = static_cast<Difficulty>(i);
            Audio::playSettingClick();
        }
    }

    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 360 && mousePos.y <= 425) {
        Audio::playStartGame();
        resetGame();
        state = GameState::PLAYING;
    }

    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 445 && mousePos.y <= 510) {
        Audio::playOpenSettings();
        previousState = GameState::MENU;
        state = GameState::HOWTOPLAY;
    }

    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 530 && mousePos.y <= 595) {
        Audio::playOpenSettings();
        previousState = GameState::MENU;
        state = GameState::SETTINGS;
    }

    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= 615 && mousePos.y <= 680) {
        shouldClose = true;
    }
}

/** Render pausescreen */
void drawPauseScreen(sf::RenderWindow& window, const sf::Font& font) {
    const float fullW = WINDOW_W;

    RectangleShape overlay(Vector2f(fullW, WINDOW_H));
    overlay.setFillColor(Color(0, 0, 0, 180));
    window.draw(overlay);

    Text pauseText(font, "PAUSED", 70);
    pauseText.setFillColor(Color::Yellow);
    float pw = pauseText.getLocalBounds().size.x;
    pauseText.setPosition({(fullW - pw) / 2.f, 150.f});
    window.draw(pauseText);

    Text hintText(font, "Press P or ESC to resume", 22);
    hintText.setFillColor(Color::White);
    float hw = hintText.getLocalBounds().size.x;
    hintText.setPosition({(fullW - hw) / 2.f, 250.f});
    window.draw(hintText);

    const float btnW = 280.f;
    const float btnX = (fullW - btnW) / 2.f;
    const float btnH = 65.f;

    RectangleShape resumeBtn({btnW, btnH});
    resumeBtn.setPosition({btnX, 320.f});
    resumeBtn.setFillColor(Color(0, 150, 0));
    window.draw(resumeBtn);
    Text resumeText(font, "RESUME", 32);
    resumeText.setFillColor(Color::White);
    float rtW = resumeText.getLocalBounds().size.x;
    resumeText.setPosition({btnX + (btnW - rtW) / 2.f, 332.f});
    window.draw(resumeText);

    RectangleShape howToBtn({btnW, btnH});
    howToBtn.setPosition({btnX, 405.f});
    howToBtn.setFillColor(Color(0, 100, 180));
    window.draw(howToBtn);
    Text howToText(font, "HOW TO PLAY", 32);
    howToText.setFillColor(Color::White);
    float htW = howToText.getLocalBounds().size.x;
    howToText.setPosition({btnX + (btnW - htW) / 2.f, 417.f});
    window.draw(howToText);

    RectangleShape settingsBtn({btnW, btnH});
    settingsBtn.setPosition({btnX, 490.f});
    settingsBtn.setFillColor(Color(100, 100, 100));
    window.draw(settingsBtn);
    Text settingsText(font, "SETTINGS", 32);
    settingsText.setFillColor(Color::White);
    float stW = settingsText.getLocalBounds().size.x;
    settingsText.setPosition({btnX + (btnW - stW) / 2.f, 502.f});
    window.draw(settingsText);

    RectangleShape menuBtn({btnW, btnH});
    menuBtn.setPosition({btnX, 575.f});
    menuBtn.setFillColor(Color(200, 0, 0));
    window.draw(menuBtn);
    Text menuText(font, "MENU", 32);
    menuText.setFillColor(Color::White);
    float mtW = menuText.getLocalBounds().size.x;
    menuText.setPosition({btnX + (btnW - mtW) / 2.f, 587.f});
    window.draw(menuText);
}

/** Process getLocalBounds */
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

/** Process setFillColor */
void drawCombo(sf::RenderWindow& window, const sf::Font& font) {
    if (comboCount <= 1) return;

    Text comboText(font, "COMBO x" + std::to_string(comboCount), 30);
    comboText.setFillColor(Color::Yellow);
    float cw = comboText.getLocalBounds().size.x;
    comboText.setPosition({STATS_W + (PLAY_W_PX - cw) / 2.f, WINDOW_H / 2.f - 50.f});
    window.draw(comboText);
}

/** Process getLocalBounds */
void addParticles(float x, float y, sf::Color color, int count) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = x;
        p.y = y;

        float angle = (rand() % 360) * 3.14159f / 180.f;
        float speed = 50.f + (rand() % 100);
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed - 50.f;
        p.life = 0.5f + (rand() % 100) / 200.f;
        p.color = color;
        particles.push_back(p);
    }
}

/** Process push_back */
void updateParticles(float dt) {
    for (auto it = particles.begin(); it != particles.end();) {
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        it->vy += 200.f * dt;
        it->life -= dt;

        if (it->life <= 0.f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

/** Render particles */
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

/** Process setFillColor */
void drawSoftDropTrail(sf::RenderWindow& window, const Piece* piece, int px, int py, bool isActive) {
    if (!isActive || !piece) return;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece->shape[i][j] != ' ') {
                float tileX = STATS_W + (px + j) * TILE_SIZE;
                float tileY = (py + i) * TILE_SIZE;

                for (int t = 1; t <= 3; t++) {
                    sf::RectangleShape trail({TILE_SIZE - 2.f, TILE_SIZE - 2.f});
                    trail.setPosition({tileX + 1.f, tileY - t * TILE_SIZE * 0.8f});
                    sf::Color c = getColor(piece->shape[i][j]);
                    c.a = static_cast<uint8_t>(80 / t);
                    trail.setFillColor(c);
                    window.draw(trail);
                }
            }
        }
    }
}

/** Render howtoplay */
void drawHowToPlay(sf::RenderWindow& window, const sf::Font& font) {
    const float fullW = WINDOW_W;

    RectangleShape bg(Vector2f(fullW, WINDOW_H));
    bg.setFillColor(Color(20, 20, 30));
    window.draw(bg);

    Text title(font, "HOW TO PLAY", 70);
    title.setFillColor(Color::Cyan);
    title.setStyle(Text::Bold);
    float titleW = title.getLocalBounds().size.x;
    title.setPosition({(fullW - titleW) / 2.f, 20.f});
    window.draw(title);

    float leftCol = 30.f;
    float rightCol = fullW / 2.f + 10.f;
    float y = 130.f;
    float lineH = 28.f;

    auto drawSection = [&](float x, float& yPos, const std::string& header, const std::vector<std::string>& lines) {
        Text headerText(font, header, 28);
        headerText.setFillColor(Color(255, 200, 50));
        headerText.setStyle(Text::Bold);
        headerText.setPosition({x, yPos});
        window.draw(headerText);
        yPos += lineH + 4.f;

        for (const auto& line : lines) {
            Text lineText(font, line, 18);
            lineText.setFillColor(Color::White);
            lineText.setPosition({x + 10.f, yPos});
            window.draw(lineText);
            yPos += lineH - 6.f;
        }
        yPos += 10.f;
    };

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

    drawSection(leftCol, leftY, "GHOST PIECE", {
        "Shows where piece will land",
        "Helps plan your drops"
    });

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

    drawSection(rightCol, rightY, "NOTES", {
        "PPM = Pieces Per Minute",
        "LPM = Lines Per Minute"
    });

    const float btnW = 280.f;
    const float btnH = 65.f;
    const float btnX = (fullW - btnW) / 2.f;
    const float btnY = WINDOW_H - 90.f;

    RectangleShape backBtn({btnW, btnH});
    backBtn.setPosition({btnX, btnY});
    backBtn.setFillColor(Color(100, 100, 100));
    window.draw(backBtn);

    Text backText(font, "BACK", 32);
    backText.setFillColor(Color::White);
    float backTxtW = backText.getLocalBounds().size.x;
    backText.setPosition({btnX + (btnW - backTxtW) / 2.f, btnY + 12.f});
    window.draw(backText);
}

/** Process getLocalBounds */
void handleHowToPlayClick(sf::Vector2i mousePos, GameState& state, GameState& previousState) {
    const float fullW = WINDOW_W;
    const float btnW = 280.f;
    const float btnH = 65.f;
    const float btnX = (fullW - btnW) / 2.f;
    const float btnY = WINDOW_H - 90.f;

    if (mousePos.x >= btnX && mousePos.x <= btnX + btnW &&
        mousePos.y >= btnY && mousePos.y <= btnY + btnH) {
        Audio::playCloseSettings();
        state = previousState;
    }
}

}
