#pragma once
#include <SFML/Graphics.hpp>
#include "Config.h"
#include "Piece.h"

// --- SIDEBAR UI ---
struct SidebarUI {
    float x, y, w, h;
    float pad;
    float boxW;
    sf::FloatRect scoreBox;
    sf::FloatRect levelBox;
    sf::FloatRect linesBox;
    sf::FloatRect nextBox;
    sf::FloatRect holdBox;
    sf::FloatRect statsBox;
};

// --- LINE CLEAR ANIMATION ---
struct LineClearAnim {
    bool active = false;
    float timer = 0.f;
    int lines[4] = {-1, -1, -1, -1};
    int count = 0;
};

extern LineClearAnim lineClearAnim;

namespace UI {
    // Tile drawing with 3D effect
    void drawTile3D(sf::RenderWindow& window, float px, float py, float size, char c);
    
    // Piece statistics panel (left side)
    void drawPieceStats(sf::RenderWindow& window, const sf::Font& font);
    
    // Sidebar
    SidebarUI makeSidebarUI();
    void drawSidebar(sf::RenderWindow& window, const SidebarUI& ui,
                     const sf::Font& font, int score, int level, int lines,
                     const Piece* next, Piece* const nextQueue[], const Piece* hold);
    
    // Settings screen
    void drawSettingsScreen(sf::RenderWindow& window, const sf::Font& font);
    void handleSettingsClick(sf::Vector2i mousePos);
    
    // Menu with difficulty
    void drawMenu(sf::RenderWindow& window, const sf::Font& font);
    void handleMenuClick(sf::Vector2i mousePos, GameState& state, GameState& previousState, bool& shouldClose);
    
    // Pause screen
    void drawPauseScreen(sf::RenderWindow& window, const sf::Font& font);
    
    // Game Over screen
    void drawGameOverScreen(sf::RenderWindow& window, const sf::Font& font);
    
    // Brightness overlay
    void drawBrightnessOverlay(sf::RenderWindow& window);
    
    // Line clear animation
    void startLineClearAnim(int* clearedLines, int count);
    void updateLineClearAnim(float dt);
    void drawLineClearAnim(sf::RenderWindow& window);
    
    // Particle system
    struct Particle {
        float x, y;
        float vx, vy;
        float life;
        sf::Color color;
    };
    
    void addParticles(float x, float y, sf::Color color, int count);
    void updateParticles(float dt);
    void drawParticles(sf::RenderWindow& window);
    
    // Soft drop trail effect
    void drawSoftDropTrail(sf::RenderWindow& window, const Piece* piece, int px, int py, bool isActive);
    
    // Combo display
    void drawCombo(sf::RenderWindow& window, const sf::Font& font);
}
