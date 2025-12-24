// ╔════════════════════════════════════════════════════════════════╗
// ║         TETRIS GAME - SS008 Implementation                     ║
// ║  Features: 7-Bag Shuffle, Pause, Settings, Ghost Piece, Score  ║
// ╚════════════════════════════════════════════════════════════════╝

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cstdio>

using namespace std;
using namespace sf;

// ==================== GAME CONFIGURATION ====================
const int TILE_SIZE = 30;           // Size of each tetromino block in pixels
const int H = 20;                   // Board height (rows)
const int W = 15;                   // Board width (columns)
const int SIDEBAR_W = 6 * TILE_SIZE; // Sidebar width for score display
const int PLAY_W_PX = W * TILE_SIZE; // Playfield width in pixels
const int PLAY_H_PX = H * TILE_SIZE; // Playfield height in pixels

// ==================== GAME STATE ====================
char board[H][W] = {};              // Game board (H x W grid)
int x = 4, y = 0;                   // Current piece position
float gameDelay = 0.8f;             // Gravity speed (lower = faster)
bool isGameOver = false;            // Game over flag

// ==================== PLAYER STATISTICS ====================
int gScore = 0;                     // Total score
int gLines = 0;                     // Lines cleared
int gLevel = 0;                     // Current level (based on lines cleared)
int currentLevel = 0;               // Track level for speed increment

// ==================== GAME SETTINGS ====================
float musicVolume = 50.f;           // Music volume (0-100%)
float sfxVolume = 50.f;             // Sound effects volume (0-100%)
float brightness = 255.f;           // Screen brightness (0-255)
bool ghostPieceEnabled = true;      // Show ghost piece preview

// ==================== AUDIO SYSTEM ====================
// Line clear sound
sf::SoundBuffer clearBuffer;
sf::Sound* clearSound = nullptr;

// Piece landing sound
sf::SoundBuffer landBuffer;
sf::Sound* landSound = nullptr;

// Game over sound
sf::SoundBuffer gameOverBuffer;
sf::Sound* gameOverSound = nullptr;

// UI button click sound
sf::SoundBuffer settingClickBuffer;
sf::Sound* settingClickSound = nullptr;

// Background music
sf::Music bgMusic;

// ==================== GAME STATE MACHINE ====================
enum class GameState {
    MENU,       // Main menu
    PLAYING,    // Active gameplay
    PAUSE,      // Game paused
    SETTINGS,   // Settings menu
};
GameState gameState = GameState::MENU;
GameState stateBeforePause = GameState::MENU;  // Tracks where we came from before pause/settings

// ==================== TETROMINO PIECE CLASSES ====================
// Base Piece class with rotation logic and wall kick system
class Piece {
public:
    char shape[4][4];  // 4x4 grid representing piece shape

    Piece() {
        // Initialize empty piece
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                shape[i][j] = ' ';
            }
        }
    }

    virtual ~Piece() {}

    // Rotate piece with wall kick (allows rotation near walls)
    virtual void rotate(int currentX, int currentY) {
        char temp[4][4];

        // Rotate 90 degrees clockwise
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[j][3 - i] = shape[i][j];
            }
        }

        // Try multiple wall kick positions: center, left, right, left2, right2
        int kicks[] = {0, -1, 1, -2, 2};
        for (int kick : kicks) {
            bool canPlace = true;
            // Check if rotated piece can fit
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (temp[i][j] != ' ') {
                        int tx = currentX + j + kick;
                        int ty = currentY + i;
                        
                        // Check bounds
                        if (tx < 1 || tx >= W - 1 || ty >= H - 1) {
                            canPlace = false;
                            break;
                        }
                        // Check collision with board
                        if (board[ty][tx] != ' ') {
                            canPlace = false;
                            break;
                        }
                    }
                }
                if (!canPlace) break;
            }
            
            // If position is valid, apply rotation and wall kick
            if (canPlace) {
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        shape[i][j] = temp[i][j];
                    }
                }
                ::x += kick;  // Apply wall kick offset
                return;
            }
        }
    }
};

// I-Piece (cyan, straight line)
class IPiece : public Piece {
public:
    IPiece() {
        shape[0][1] = 'I';
        shape[1][1] = 'I';
        shape[2][1] = 'I';
        shape[3][1] = 'I';
    }
};

// O-Piece (yellow, square - cannot rotate)
class OPiece : public Piece {
public:
    OPiece() {
        shape[1][1] = 'O'; shape[1][2] = 'O';
        shape[2][1] = 'O'; shape[2][2] = 'O';
    }
    void rotate(int, int) override {}  // Override to prevent rotation
};

// T-Piece (purple, T shape) - with proper 90-degree rotation states
class TPiece : public Piece {
private:
    int rotationState = 0;  // 0=Up, 1=Right, 2=Down, 3=Left
    
    void applyRotationState(int state) {
        // Clear entire shape
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                shape[i][j] = ' ';
            }
        }
        
        // Apply shape based on rotation state - all pivot at (1,1)
        switch(state % 4) {
            case 0:  // Up (original) - spans rows 1-2
                shape[0][1] = 'T';
                shape[1][0] = 'T'; shape[1][1] = 'T'; shape[1][2] = 'T';
                break;
            case 1:  // Right (90° clockwise) - spans rows 0-2
                shape[0][1] = 'T';
                shape[1][1] = 'T'; shape[1][2] = 'T';
                shape[2][1] = 'T';
                break;
            case 2:  // Down (180°) - spans rows 0-1
                shape[1][0] = 'T'; shape[1][1] = 'T'; shape[1][2] = 'T';
                shape[2][1] = 'T';
                break;
            case 3:  // Left (270° clockwise) - spans rows 0-2
                shape[0][1] = 'T';
                shape[1][0] = 'T'; shape[1][1] = 'T';
                shape[2][1] = 'T';
                break;
        }
    }
    
public:
    TPiece() {
        rotationState = 0;
        applyRotationState(0);
    }
    
    void rotate(int currentX, int currentY) override {
        int nextState = (rotationState + 1) % 4;
        applyRotationState(nextState);
        
        // Test with wall kicks: center, left 1, right 1, left 2, right 2
        int kicks[] = {0, -1, 1, -2, 2};
        for (int kick : kicks) {
            bool canPlace = true;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (shape[i][j] != ' ') {
                        int tx = currentX + j + kick;
                        int ty = currentY + i;
                        
                        if (tx < 1 || tx >= W - 1 || ty >= H - 1) {
                            canPlace = false;
                            break;
                        }
                        if (board[ty][tx] != ' ') {
                            canPlace = false;
                            break;
                        }
                    }
                }
                if (!canPlace) break;
            }
            
            if (canPlace) {
                rotationState = nextState;
                ::x += kick;
                return;
            }
        }
        
        // Revert if no valid position found
        applyRotationState(rotationState);
    }
};

// S-Piece (green, S shape)
class SPiece : public Piece {
public:
    SPiece() {
        shape[1][1] = 'S'; shape[1][2] = 'S';
        shape[2][0] = 'S'; shape[2][1] = 'S';
    }
};

// Z-Piece (red, Z shape)
class ZPiece : public Piece {
public:
    ZPiece() {
        shape[1][0] = 'Z'; shape[1][1] = 'Z';
        shape[2][1] = 'Z'; shape[2][2] = 'Z';
    }
};

// J-Piece (blue, J shape)
class JPiece : public Piece {
public:
    JPiece() {
        shape[1][0] = 'J';
        shape[2][0] = 'J'; shape[2][1] = 'J'; shape[2][2] = 'J';
    }
};

// L-Piece (orange, L shape)
class LPiece : public Piece {
public:
    LPiece() {
        shape[1][2] = 'L';
        shape[2][0] = 'L'; shape[2][1] = 'L'; shape[2][2] = 'L';
    }
};

// ==================== UTILITY FUNCTIONS ====================
// Get color for tetromino type
Color getColor(char c) {
    switch (c) {
        case 'I': return Color::Cyan;
        case 'J': return Color::Blue;
        case 'L': return Color(255, 165, 0);  // Orange
        case 'O': return Color::Yellow;
        case 'S': return Color::Green;
        case 'T': return Color(128, 0, 128);  // Purple
        case 'Z': return Color::Red;
        case '#': return Color(100, 100, 100); // Wall
        default:  return Color::Black;
    }
}

// ==================== SIDEBAR UI STRUCTURE ====================
// Manages layout of score, level, lines, and next piece preview
struct SidebarUI {
    float x, y, w, h;       // Position and dimensions
    float pad;              // Padding
    float boxW;             // Box width
    sf::FloatRect scoreBox; // Score display area
    sf::FloatRect levelBox; // Level display area
    sf::FloatRect linesBox; // Lines cleared display area
    sf::FloatRect nextBox;  // Next piece preview area
};

// Initialize sidebar UI layout
static SidebarUI makeSidebarUI() {
    SidebarUI ui{};
    ui.x = (float)PLAY_W_PX;
    ui.y = 0.f;
    ui.w = (float)SIDEBAR_W;
    ui.h = (float)PLAY_H_PX;
    ui.pad  = 10.f;
    ui.boxW = ui.w - 2.f * ui.pad;

    const float boxH = 90.f;
    const float gap  = 30.f;
    const float left = ui.x + ui.pad;

    ui.scoreBox = sf::FloatRect({left, 20.f},               {ui.boxW, boxH});
    ui.levelBox = sf::FloatRect({left, 20.f + boxH + gap},  {ui.boxW, boxH});
    ui.linesBox = sf::FloatRect({left, 20.f + 2*(boxH+gap)},{ui.boxW, boxH});
    ui.nextBox  = sf::FloatRect({left, 20.f + 3*(boxH+gap)},{ui.boxW, 220.f});
    return ui;
}

// Draw a panel with border
static void drawPanel(sf::RenderWindow& window, const sf::FloatRect& r) {
    const float outline = 3.f;
    const float inset = outline;
    sf::RectangleShape box({ r.size.x - 2.f*inset, r.size.y - 2.f*inset });
    box.setPosition({ r.position.x + inset, r.position.y + inset });
    box.setFillColor(sf::Color::Black);
    box.setOutlineThickness(outline);
    box.setOutlineColor(sf::Color(200, 200, 200));
    window.draw(box);
}

// Draw text at specific position
static void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& s, float x, float y, unsigned size) {
    sf::Text t(font, s, size);
    t.setFillColor(sf::Color::White);
    t.setPosition({x, y});
    window.draw(t);
}

// Draw preview of next piece
static void drawNextPreview(sf::RenderWindow& window, const SidebarUI& ui, const Piece* p) {
    if (!p) return;
    
    // Find bounding box of piece
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

    // Calculate dimensions and center in preview box
    int cellsW = maxC - minC + 1;
    int cellsH = maxR - minR + 1;
    int mini = TILE_SIZE / 2;

    sf::Vector2f areaPos = { ui.nextBox.position.x + 16.f, ui.nextBox.position.y + 60.f };
    sf::Vector2f areaSize = { ui.nextBox.size.x - 32.f, ui.nextBox.size.y - 80.f };
    float startX = areaPos.x + (areaSize.x - cellsW * mini) * 0.5f;
    float startY = areaPos.y + (areaSize.y - cellsH * mini) * 0.5f;

    // Draw piece blocks
    for (int r = minR; r <= maxR; r++) {
        for (int c = minC; c <= maxC; c++) {
            if (p->shape[r][c] != ' ') {
                sf::RectangleShape rect({(float)mini - 1, (float)mini - 1});
                rect.setPosition({ startX + (c - minC) * mini, startY + (r - minR) * mini });
                rect.setFillColor(getColor(p->shape[r][c]));
                window.draw(rect);
            }
        }
    }
}

// Draw entire sidebar with score, level, lines, and next piece
static void drawSidebar(sf::RenderWindow& window, const SidebarUI& ui,
                        const sf::Font& font, int score, int level, int lines,
                        const Piece* next) {
    // Sidebar background
    sf::RectangleShape bg({ui.w, ui.h});
    bg.setPosition({ui.x, ui.y});
    bg.setFillColor(sf::Color(30, 30, 30));
    window.draw(bg);

    // Draw all panels
    drawPanel(window, ui.scoreBox);
    drawPanel(window, ui.levelBox);
    drawPanel(window, ui.linesBox);
    drawPanel(window, ui.nextBox);

    // Draw text labels and values
    float labelX = ui.scoreBox.position.x + 12.f;
    drawText(window, font, "SCORE", labelX, ui.scoreBox.position.y + 10.f, 18);
    drawText(window, font, std::to_string(score), labelX, ui.scoreBox.position.y + 42.f, 24);
    drawText(window, font, "LEVEL", labelX, ui.levelBox.position.y + 10.f, 18);
    drawText(window, font, std::to_string(level), labelX, ui.levelBox.position.y + 42.f, 24);
    drawText(window, font, "LINES", labelX, ui.linesBox.position.y + 10.f, 18);
    drawText(window, font, std::to_string(lines), labelX, ui.linesBox.position.y + 42.f, 24);
    drawText(window, font, "NEXT", labelX, ui.nextBox.position.y + 10.f, 18);
    drawNextPreview(window, ui, next);
}

// ==================== GAME LOGIC ====================
Piece* currentPiece = nullptr;       // Currently falling piece
Piece* nextPiece = nullptr;          // Next piece to spawn

// ==================== 7-BAG SHUFFLE ALGORITHM ====================
// Ensures fair piece distribution - all 7 pieces appear before repeating
vector<int> pieceQueue;              // Queue of piece types
int queueIndex = 0;                  // Current position in queue

// Refill queue with randomized bag of 7 pieces
void refillPieceQueue() {
    vector<int> bag = {0, 1, 2, 3, 4, 5, 6};  // All 7 piece types
    
    // Fisher-Yates shuffle algorithm
    for (int i = 0; i < 7; i++) {
        int r = rand() % (7 - i);
        pieceQueue.push_back(bag[r]);
        swap(bag[r], bag[6 - i]);
    }
}

// Create piece from type ID
Piece* createPieceFromType(int type) {
    switch (type) {
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

// Get next random piece using 7-bag shuffle
Piece* createRandomPiece() {
    // Refill queue when empty
    if (queueIndex >= pieceQueue.size()) {
        refillPieceQueue();
        queueIndex = 0;
    }
    Piece* p = createPieceFromType(pieceQueue[queueIndex]);
    queueIndex++;
    return p;
}

// ==================== BOARD OPERATIONS ====================
// Commit current piece to board
void block2Board() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] != ' ') {
                board[y + i][x + j] = currentPiece->shape[i][j];
            }
        }
    }
}

// Initialize empty board with walls
void initBoard() {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            // Add borders (walls marked with '#')
            if ((i == H - 1) || (j == 0) || (j == W - 1)) {
                board[i][j] = '#';
            } else {
                board[i][j] = ' ';
            }
        }
    }
}

// Check if current piece can move in direction (dx, dy)
bool canMove(int dx, int dy) {
    if (!currentPiece) return false;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] != ' ') {
                int tx = x + j + dx;
                int ty = y + i + dy;
                
                // Check bounds and collision
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return false;
                if (board[ty][tx] != ' ') return false;
            }
        }
    }
    return true;
}

// Calculate Y position for ghost piece (preview of landing position)
int getGhostY() {
    int ghostY = y;
    while (true) {
        bool canGo = true;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentPiece->shape[i][j] != ' ') {
                    int tx = x + j;
                    int ty = ghostY + i + 1;
                    // Check if we hit bottom or collision
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

// ==================== GAME PROGRESSION ====================
// Increase game speed (gravity) when leveling up
void SpeedIncrement() {
    if (gameDelay > 0.1f) {
        gameDelay -= 0.08f;
    }
}

// Update score and level based on lines cleared
void applyLineClearScore(int cleared) {
    if (cleared <= 0) return;
    
    gLines += cleared;
    gScore += 100 * cleared;           // 100 points per line
    gLevel = gLines / 10;              // Level up every 10 lines
    
    // Increase game speed when leveling up
    if (gLevel > currentLevel) {
        SpeedIncrement();
        currentLevel = gLevel;
    }
}

// Detect and remove completed lines
int removeLine() {
    int cleared = 0;
    
    // Check each row from bottom up
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        
        // Check if row is completely filled
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') { 
                isFull = false; 
                break; 
            }
        }
        
        // If full, remove and drop lines above
        if (isFull) {
            cleared++;
            clearSound->play();
            
            // Move all rows above down by one
            for (int k = i; k > 0; k--) {
                for (int j = 1; j < W - 1; j++) {
                    board[k][j] = (k != 1) ? board[k - 1][j] : ' ';
                }
            }
            i++;  // Check same row again (shifted down)
        }
    }
    return cleared;
}

// Reset game to initial state
void resetGame() {
    initBoard();
    delete currentPiece;
    delete nextPiece;
    currentPiece = createRandomPiece();
    nextPiece = createRandomPiece();
    x = 4;
    y = 0;
    gameDelay = 0.8f;
    isGameOver = false;
    gScore = 0;
    gLines = 0;
    gLevel = 0;
    currentLevel = 0;
}

// ==================== MAIN GAME LOOP ====================
int main() {
    // Window setup
    RenderWindow window(VideoMode(Vector2u(PLAY_W_PX + SIDEBAR_W, PLAY_H_PX)), "SS008 - Tetris");
    window.setFramerateLimit(60);  // 60 FPS cap

    // Load window icon
    sf::Image icon;
    if (icon.loadFromFile("assets/logo.png")) {
        window.setIcon(icon);
    }

    SidebarUI ui = makeSidebarUI();

    // ==================== AUDIO LOADING ====================
    // Load music and sound effects from assets folder
    if (!bgMusic.openFromFile("assets/loop_theme.ogg")) return -1;
    if (!clearBuffer.loadFromFile("assets/line_clear.ogg")) return -1;
    if (!landBuffer.loadFromFile("assets/bumper_end.ogg")) return -1;
    if (!gameOverBuffer.loadFromFile("assets/game_over.ogg")) return -1;
    if (!settingClickBuffer.loadFromFile("assets/insetting_click.ogg")) return -1;

    // Create sound objects from buffers
    clearSound = new sf::Sound(clearBuffer);
    landSound = new sf::Sound(landBuffer);
    gameOverSound = new sf::Sound(gameOverBuffer);
    settingClickSound = new sf::Sound(settingClickBuffer);

    // Setup audio
    bgMusic.setLooping(true);
    bgMusic.setVolume(musicVolume);
    bgMusic.play();

    // ==================== GAME INITIALIZATION ====================
    srand((unsigned)time(0));  // Random seed
    refillPieceQueue();         // Initialize 7-bag shuffle
    currentPiece = createRandomPiece();
    nextPiece = createRandomPiece();
    initBoard();

    Clock clock;
    float timer = 0.f;           // Gravity timer
    float inputTimer = 0.f;      // Input delay timer
    const float inputDelay = 0.1f;  // Minimum time between left/right moves

    // ==================== UI FONT LOADING ====================
    Font font;
    if (!font.openFromFile("assets/Monocraft.ttf")) return -1;

    // ===== MAIN MENU TITLE =====
    Text title(font);
    title.setString("SS008 - TETRIS");
    title.setCharacterSize(36);
    title.setFillColor(Color::Cyan);
    float titleWidth = title.getLocalBounds().size.x;
    title.setPosition(sf::Vector2f{(PLAY_W_PX + SIDEBAR_W - titleWidth) / 2.f, 60.f});

    // ===== BUTTON FACTORY =====
    const float btnWidth = 200.f;
    const float btnX = (PLAY_W_PX + SIDEBAR_W - btnWidth) / 2.f;
    auto createButton = [&](const string& label, float by) {
        RectangleShape btn(Vector2f(btnWidth, 50));
        btn.setPosition(sf::Vector2f{btnX, by});
        btn.setFillColor(Color(50, 50, 50));
        sf::Text txt(font);
        txt.setString(label);
        txt.setCharacterSize(24);
        txt.setFillColor(Color::White);
        float txtWidth = txt.getLocalBounds().size.x;
        txt.setPosition(sf::Vector2f{btnX + (btnWidth - txtWidth) / 2.f, by + 10.f});
        return pair<RectangleShape, Text>(btn, txt);
    };

    // Create menu buttons
    auto [startBtn, startText]     = createButton("START",    160);
    auto [settingBtn, settingText] = createButton("SETTINGS", 230);
    auto [exitBtn, exitText]       = createButton("EXIT",     300);

    auto isClicked = [&](RectangleShape& btn, Vector2f mousePos) {
        return btn.getGlobalBounds().contains(mousePos);    
    };

    // Cursors for hover effect
    auto arrowCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
    auto handCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);

    // ==================== VIEW SETUP (For Aspect Ratio) ====================
    float baseW = static_cast<float>(PLAY_W_PX + SIDEBAR_W);
    float baseH = static_cast<float>(PLAY_H_PX);
    View view(FloatRect({0.f, 0.f}, {baseW, baseH}));
    window.setView(view);

    // ==================== MAIN GAME LOOP ====================
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        // Only update timer during active gameplay
        if (!isGameOver && gameState == GameState::PLAYING) {
            timer += dt;
            inputTimer += dt;
        }

        // ==================== EVENT HANDLING ====================
        while (const auto event = window.pollEvent()) {
            // ===== PAUSE TOGGLE (press P or Esc from PLAYING to enter PAUSE) =====
            // This is checked first to intercept pause key before other handlers
            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                if ((keyPressed->code == Keyboard::Key::P || keyPressed->code == Keyboard::Key::Escape) && gameState == GameState::PLAYING && !isGameOver) {
                    stateBeforePause = GameState::PLAYING;
                    gameState = GameState::PAUSE;
                    bgMusic.pause();
                    continue;
                }
            }

            // ===== MENU CLICK HANDLING =====
            // Process mouse clicks on main menu buttons
            if (gameState == GameState::MENU) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i pixelPos = Mouse::getPosition(window);
                        Vector2f mousePos = window.mapPixelToCoords(pixelPos);

                        // START button - begin new game
                        if (isClicked(startBtn, mousePos)) {
                            resetGame();
                            gameState = GameState::PLAYING;
                            continue;
                        }
                        // SETTINGS button - open settings menu
                        if (isClicked(settingBtn, mousePos)) {
                            gameState = GameState::SETTINGS;
                            continue;
                        }
                        // EXIT button - close application
                        if (isClicked(exitBtn, mousePos)) {
                            window.close();
                        }
                    }
                }
            }

            if (event->is<Event::Closed>()) {
                window.close();
            }

            // ===== RESIZE LOGIC =====
            // Maintain aspect ratio when window is resized
            if (const auto* resized = event->getIf<Event::Resized>()) {
                float windowRatio = static_cast<float>(resized->size.x) / static_cast<float>(resized->size.y);
                float viewRatio = baseW / baseH;
                float sizeX = 1;
                float sizeY = 1;
                float posX = 0;
                float posY = 0;

                // Calculate letterbox or pillarbox dimensions
                if (windowRatio > viewRatio) {
                    sizeX = viewRatio / windowRatio;
                    posX = (1 - sizeX) / 2.f;
                }
                else {
                    sizeY = windowRatio / viewRatio;
                    posY = (1 - sizeY) / 2.f;
                }

                view.setViewport(FloatRect({posX, posY}, {sizeX, sizeY}));
                window.setView(view);
            }

            // ===== GAME OVER CLICK HANDLING =====
            // Process mouse clicks on game over menu buttons
            if (isGameOver && gameState == GameState::PLAYING) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i pixelPos = Mouse::getPosition(window);
                        Vector2f mousePos = window.mapPixelToCoords(pixelPos);

                        const float fullW = PLAY_W_PX + SIDEBAR_W;
                        const float goBtnW = 200.f;
                        const float goBtnX = (fullW - goBtnW) / 2.f;
                        
                        // RESTART button - reset and play again
                        if (mousePos.x > goBtnX && mousePos.x < goBtnX + goBtnW && mousePos.y > 230 && mousePos.y < 280) {
                            resetGame();
                            bgMusic.play();
                        }
                        // MENU button - return to main menu
                        if (mousePos.x > goBtnX && mousePos.x < goBtnX + goBtnW && mousePos.y > 300 && mousePos.y < 350) {
                            resetGame();
                            gameState = GameState::MENU;
                            bgMusic.play();
                        }
                        // EXIT button - close game
                        if (mousePos.x > goBtnX && mousePos.x < goBtnX + goBtnW && mousePos.y > 370 && mousePos.y < 420) {
                            window.close();
                        }
                    }
                }
            }

            // ===== PLAYING KEY EVENTS =====
            // Handle discrete key presses during gameplay
            if (gameState == GameState::PLAYING && !isGameOver) {
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    // W key - ROTATE piece
                    if (keyPressed->code == Keyboard::Key::W) {
                        currentPiece->rotate(x, y);
                    }
                    // SPACE - HARD DROP (instantly drop piece to bottom)
                    else if (keyPressed->code == Keyboard::Key::Space) {
                        while (canMove(0, 1)) y++;
                        timer = gameDelay + 10.0f;  // Force immediate landing
                    }
                }
            }
            // ===== MENU KEY EVENTS =====
            // Handle keyboard shortcuts in main menu
            else if (gameState == GameState::MENU) {
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    // ENTER - Start game
                    if (keyPressed->code == Keyboard::Key::Enter) {
                        resetGame();
                        gameState = GameState::PLAYING;
                    }
                    // ESC - Exit application
                    if (keyPressed->code == Keyboard::Key::Escape) {
                        window.close();
                    }
                }
            }

            // ===== SETTINGS CLICK HANDLING =====
            // Process mouse clicks and sliders in settings menu
            if (gameState == GameState::SETTINGS) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i pixelPos = Mouse::getPosition(window);
                        Vector2f mousePos = window.mapPixelToCoords(pixelPos);

                        // ===== MUSIC VOLUME SLIDER =====
                        // Left arrow (decrease volume)
                        if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 125 && mousePos.y <= 155) {
                            musicVolume = max(0.f, musicVolume - 5.f);
                            bgMusic.setVolume(musicVolume);
                            settingClickSound->play();
                        }
                        // Right arrow (increase volume)
                        if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 125 && mousePos.y <= 155) {
                            musicVolume = min(100.f, musicVolume + 5.f);
                            bgMusic.setVolume(musicVolume);
                            settingClickSound->play();
                        }
                        // Slider drag
                        if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 127 && mousePos.y <= 157) {
                            musicVolume = static_cast<float>(mousePos.x - 285) / 2.f;
                            musicVolume = max(0.f, min(100.f, musicVolume));
                            bgMusic.setVolume(musicVolume);
                            settingClickSound->play();
                        }

                        // ===== SFX VOLUME SLIDER =====
                        // Left arrow
                        if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 185 && mousePos.y <= 215) {
                            sfxVolume = max(0.f, sfxVolume - 5.f);
                            clearSound->setVolume(sfxVolume);
                            landSound->setVolume(sfxVolume);
                            gameOverSound->setVolume(sfxVolume);
                            settingClickSound->setVolume(sfxVolume);
                            settingClickSound->play();
                        }
                        // Right arrow
                        if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 185 && mousePos.y <= 215) {
                            sfxVolume = min(100.f, sfxVolume + 5.f);
                            clearSound->setVolume(sfxVolume);
                            landSound->setVolume(sfxVolume);
                            gameOverSound->setVolume(sfxVolume);
                            settingClickSound->setVolume(sfxVolume);
                            settingClickSound->play();
                        }
                        // Slider drag
                        if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 187 && mousePos.y <= 217) {
                            sfxVolume = static_cast<float>(mousePos.x - 285) / 2.f;
                            sfxVolume = max(0.f, min(100.f, sfxVolume));
                            clearSound->setVolume(sfxVolume);
                            landSound->setVolume(sfxVolume);
                            gameOverSound->setVolume(sfxVolume);
                            settingClickSound->setVolume(sfxVolume);
                            settingClickSound->play();
                        }

                        // ===== BRIGHTNESS SLIDER =====
                        // Range: 20% to 100% (51 to 255)
                        // Left arrow
                        if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 245 && mousePos.y <= 275) {
                            brightness = max(51.f, brightness - 10.f);
                            settingClickSound->play();
                        }
                        // Right arrow
                        if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 245 && mousePos.y <= 275) {
                            brightness = min(255.f, brightness + 10.f);
                            settingClickSound->play();
                        }
                        // Slider drag - map 0-200px to 51-255 brightness
                        if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 247 && mousePos.y <= 277) {
                            float normalized = static_cast<float>(mousePos.x - 285) / 200.f;
                            brightness = normalized * (255.f - 51.f) + 51.f;
                            settingClickSound->play();
                        }

                        // ===== GHOST PIECE TOGGLE =====
                        // Click checkbox to toggle ghost piece display
                        if (mousePos.x >= 280 && mousePos.x <= 315 && mousePos.y >= 303 && mousePos.y <= 337) {
                            ghostPieceEnabled = !ghostPieceEnabled;
                            settingClickSound->play();
                        }

                        // ===== BACK BUTTON =====
                        // Return to previous menu (Main Menu or Pause Menu)
                        const float backBtnW = 200.f;
                        const float backBtnX = (PLAY_W_PX + SIDEBAR_W - backBtnW) / 2.f;
                        if (mousePos.x > backBtnX && mousePos.x < backBtnX + backBtnW && mousePos.y > 375 && mousePos.y < 435) {
                            // Return to where we came from
                            if (stateBeforePause == GameState::PAUSE) {
                                gameState = GameState::PAUSE;
                                bgMusic.pause();
                            } else {
                                gameState = GameState::MENU;
                                bgMusic.play();
                            }
                            settingClickSound->play();
                        }
                    }
                }
                // ESC key also goes back
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    if (keyPressed->code == Keyboard::Key::Escape) {
                        // Return to where we came from
                        if (stateBeforePause == GameState::PAUSE) {
                            gameState = GameState::PAUSE;
                            bgMusic.pause();
                        } else {
                            gameState = GameState::MENU;
                            bgMusic.play();
                        }
                    }
                }
            }

            // ===== PAUSE MENU CLICK HANDLING =====
            // Process mouse clicks on pause menu buttons
            if (gameState == GameState::PAUSE) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i pixelPos = Mouse::getPosition(window);
                        Vector2f mousePos = window.mapPixelToCoords(pixelPos);

                        const float fullW = PLAY_W_PX + SIDEBAR_W;
                        const float pauseBtnW = 200.f;
                        const float pauseBtnX = (fullW - pauseBtnW) / 2.f;

                        // RESUME button - continue game
                        if (mousePos.x > pauseBtnX && mousePos.x < pauseBtnX + pauseBtnW && mousePos.y > 200 && mousePos.y < 250) {
                            gameState = GameState::PLAYING;
                            bgMusic.play();
                        }
                        // SETTINGS button - open settings from pause
                        if (mousePos.x > pauseBtnX && mousePos.x < pauseBtnX + pauseBtnW && mousePos.y > 270 && mousePos.y < 320) {
                            stateBeforePause = GameState::PAUSE;
                            gameState = GameState::SETTINGS;
                        }
                        // MENU button - return to main menu
                        if (mousePos.x > pauseBtnX && mousePos.x < pauseBtnX + pauseBtnW && mousePos.y > 340 && mousePos.y < 390) {
                            resetGame();
                            gameState = GameState::MENU;
                            bgMusic.play();
                        }
                    }
                }
                // P or ESC key also resumes game
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    if (keyPressed->code == Keyboard::Key::P || keyPressed->code == Keyboard::Key::Escape) {
                        gameState = GameState::PLAYING;
                        bgMusic.play();
                    }
                }
            }
        }

        // ==================== CONTINUOUS INPUT (Held Keys) ====================
        // Handle continuous input for movement (separate from discrete event input)
        if (gameState == GameState::PLAYING && !isGameOver) {
            if (inputTimer > inputDelay) {  // Check movement every inputDelay seconds
                // Handle LEFT movement
                if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
                    if (canMove(-1, 0)) x--;
                    inputTimer = 0;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
                    if (canMove(1, 0)) x++;
                    inputTimer = 0;
                }
                else if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
                    if (canMove(0, 1)) y++;
                    inputTimer = 0;
                }
            }

            // ===== GRAVITY (Piece Falling) =====
            // Apply gravity - move piece down if possible
            if (timer > gameDelay) {  // Check gravity every gameDelay seconds
                if (canMove(0, 1)) {
                    y++;  // Move piece down
                } else {
                    // Piece cannot move down - it lands
                    landSound->play();
                    block2Board();  // Commit piece to board
                    int cleared = removeLine();  // Check for completed lines
                    applyLineClearScore(cleared);  // Update score and level
                    
                    // Spawn next piece
                    delete currentPiece;
                    currentPiece = nextPiece;
                    nextPiece = createRandomPiece();
                    x = 4;  // Spawn at center top
                    y = 0;
                    
                    // Check if new piece can spawn (game over condition)
                    if (!canMove(0, 0)) {
                        isGameOver = true;
                        gameOverSound->play();
                        bgMusic.stop();
                    }
                }
                timer = 0;
            }
        }

        // ==================== RENDERING ====================
        window.clear(Color::Black);  // Clear screen for new frame

        // ===== MENU RENDERING =====
        // Draw main menu with title and buttons
        if (gameState == GameState::MENU) {
            window.draw(title);
            window.draw(startBtn);
            window.draw(startText);
            window.draw(settingBtn);
            window.draw(settingText);
            window.draw(exitBtn);
            window.draw(exitText);
        }

        // ===== GAME PLAYING STATE RENDERING =====
        // Draw the active game board, pieces, and sidebar
        if (gameState == GameState::PLAYING) {
            // Draw Board
            for (int i = 0; i < H; i++) {
                for (int j = 0; j < W; j++) {
                    if (board[i][j] != ' ') {
                        RectangleShape rect(Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                        rect.setPosition(Vector2f(j * TILE_SIZE, i * TILE_SIZE));
                        rect.setFillColor(getColor(board[i][j]));
                        window.draw(rect);
                    }
                }
            }

            // Draw Ghost Piece
            if (!isGameOver && ghostPieceEnabled) {
                int ghostY = getGhostY();
                if (ghostY != y) {
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            if (currentPiece->shape[i][j] != ' ') {
                                RectangleShape rect(Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                                rect.setPosition(Vector2f((x + j) * TILE_SIZE, (ghostY + i) * TILE_SIZE));
                                rect.setFillColor(Color::Transparent);
                                rect.setOutlineThickness(2.f);
                                rect.setOutlineColor(Color(255, 255, 255, 150));
                                window.draw(rect);
                            }
                        }
                    }
                }
            }

            // Draw Current Piece
            if (!isGameOver) {
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (currentPiece->shape[i][j] != ' ') {
                            RectangleShape rect(Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                            rect.setPosition(Vector2f((x + j) * TILE_SIZE, (y + i) * TILE_SIZE));
                            rect.setFillColor(getColor(currentPiece->shape[i][j]));
                            window.draw(rect);
                        }
                    }
                }
            }

            // Draw Sidebar
            drawSidebar(window, ui, font, gScore, gLevel, gLines, nextPiece);

            // ===== GAME OVER SCREEN =====
            if (isGameOver) {
                // Overlay for play area
                RectangleShape overlay(Vector2f(PLAY_W_PX, PLAY_H_PX));
                overlay.setFillColor(Color(0, 0, 0, 200));
                window.draw(overlay);

                // Overlay for sidebar
                RectangleShape sidebarOverlay(Vector2f(SIDEBAR_W, PLAY_H_PX));
                sidebarOverlay.setPosition(Vector2f(PLAY_W_PX, 0));
                sidebarOverlay.setFillColor(Color(0, 0, 0, 200));
                window.draw(sidebarOverlay);

                // Game Over text - centered in full window
                const float fullW = PLAY_W_PX + SIDEBAR_W;
                Text gameOverText(font);
                gameOverText.setString("GAME OVER");
                gameOverText.setCharacterSize(40);
                gameOverText.setFillColor(Color::Red);
                float goWidth = gameOverText.getLocalBounds().size.x;
                gameOverText.setPosition(sf::Vector2f{(fullW - goWidth) / 2.f, 150.f});
                window.draw(gameOverText);

                // Buttons centered in full window
                const float goBtnW = 200.f;
                const float goBtnX = (fullW - goBtnW) / 2.f;

                RectangleShape restartBtn2(Vector2f(goBtnW, 50));
                restartBtn2.setPosition(sf::Vector2f{goBtnX, 230.f});
                restartBtn2.setFillColor(Color(0, 100, 255));
                window.draw(restartBtn2);
                Text restartText2(font);
                restartText2.setString("RESTART");
                restartText2.setCharacterSize(24);
                restartText2.setFillColor(Color::White);
                float restartTxtW = restartText2.getLocalBounds().size.x;
                restartText2.setPosition(sf::Vector2f{goBtnX + (goBtnW - restartTxtW) / 2.f, 240.f});
                window.draw(restartText2);

                RectangleShape menuBtnGO(Vector2f(goBtnW, 50));
                menuBtnGO.setPosition(sf::Vector2f{goBtnX, 300.f});
                menuBtnGO.setFillColor(Color(100, 100, 100));
                window.draw(menuBtnGO);
                Text menuTextGO(font);
                menuTextGO.setString("MENU");
                menuTextGO.setCharacterSize(24);
                menuTextGO.setFillColor(Color::White);
                float menuTxtW = menuTextGO.getLocalBounds().size.x;
                menuTextGO.setPosition(sf::Vector2f{goBtnX + (goBtnW - menuTxtW) / 2.f, 310.f});
                window.draw(menuTextGO);

                RectangleShape exitBtnGO(Vector2f(goBtnW, 50));
                exitBtnGO.setPosition(sf::Vector2f{goBtnX, 370.f});
                exitBtnGO.setFillColor(Color(255, 50, 50));
                window.draw(exitBtnGO);
                Text exitTextGO(font);
                exitTextGO.setString("EXIT");
                exitTextGO.setCharacterSize(24);
                exitTextGO.setFillColor(Color::White);
                float exitTxtW = exitTextGO.getLocalBounds().size.x;
                exitTextGO.setPosition(sf::Vector2f{goBtnX + (goBtnW - exitTxtW) / 2.f, 380.f});
                window.draw(exitTextGO);
            }
        }

        // ===== PAUSE MENU RENDERING =====
        // Draw game in background with semi-transparent overlay and pause menu
        if (gameState == GameState::PAUSE) {
            // Draw Board (from game)
            for (int i = 0; i < H; i++) {
                for (int j = 0; j < W; j++) {
                    if (board[i][j] != ' ') {
                        RectangleShape rect(Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                        rect.setPosition(Vector2f(j * TILE_SIZE, i * TILE_SIZE));
                        rect.setFillColor(getColor(board[i][j]));
                        window.draw(rect);
                    }
                }
            }

            // Draw Ghost Piece
            if (ghostPieceEnabled) {
                int ghostY = getGhostY();
                if (ghostY != y) {
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            if (currentPiece->shape[i][j] != ' ') {
                                RectangleShape rect(Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                                rect.setPosition(Vector2f((x + j) * TILE_SIZE, (ghostY + i) * TILE_SIZE));
                                rect.setFillColor(Color::Transparent);
                                rect.setOutlineThickness(2.f);
                                rect.setOutlineColor(Color(255, 255, 255, 150));
                                window.draw(rect);
                            }
                        }
                    }
                }
            }

            // Draw Current Piece
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (currentPiece->shape[i][j] != ' ') {
                        RectangleShape rect(Vector2f(TILE_SIZE - 1, TILE_SIZE - 1));
                        rect.setPosition(Vector2f((x + j) * TILE_SIZE, (y + i) * TILE_SIZE));
                        rect.setFillColor(getColor(currentPiece->shape[i][j]));
                        window.draw(rect);
                    }
                }
            }

            // Draw Sidebar
            drawSidebar(window, ui, font, gScore, gLevel, gLines, nextPiece);

            // Draw semi-transparent overlay to dim the game
            RectangleShape pauseOverlay(Vector2f(PLAY_W_PX + SIDEBAR_W, PLAY_H_PX));
            pauseOverlay.setFillColor(Color(0, 0, 0, 100));
            window.draw(pauseOverlay);

            // Pause title - centered in full window
            const float fullW = PLAY_W_PX + SIDEBAR_W;
            Text pauseText(font);
            pauseText.setString("PAUSED");
            pauseText.setCharacterSize(48);
            pauseText.setFillColor(Color::Yellow);
            float pauseWidth = pauseText.getLocalBounds().size.x;
            pauseText.setPosition(sf::Vector2f{(fullW - pauseWidth) / 2.f, 80.f});
            window.draw(pauseText);

            // Buttons centered in full window
            const float pauseBtnW = 200.f;
            const float pauseBtnX = (fullW - pauseBtnW) / 2.f;

            // Resume button
            RectangleShape resumeBtn(Vector2f(pauseBtnW, 50));
            resumeBtn.setPosition(sf::Vector2f{pauseBtnX, 200.f});
            resumeBtn.setFillColor(Color(0, 150, 100));
            window.draw(resumeBtn);
            Text resumeText(font);
            resumeText.setString("RESUME");
            resumeText.setCharacterSize(24);
            resumeText.setFillColor(Color::White);
            float resumeTxtW = resumeText.getLocalBounds().size.x;
            resumeText.setPosition(sf::Vector2f{pauseBtnX + (pauseBtnW - resumeTxtW) / 2.f, 210.f});
            window.draw(resumeText);

            // Settings button
            RectangleShape settingsBtn2(Vector2f(pauseBtnW, 50));
            settingsBtn2.setPosition(sf::Vector2f{pauseBtnX, 270.f});
            settingsBtn2.setFillColor(Color(100, 100, 150));
            window.draw(settingsBtn2);
            Text settingsText2(font);
            settingsText2.setString("SETTINGS");
            settingsText2.setCharacterSize(24);
            settingsText2.setFillColor(Color::White);
            float settingsTxtW = settingsText2.getLocalBounds().size.x;
            settingsText2.setPosition(sf::Vector2f{pauseBtnX + (pauseBtnW - settingsTxtW) / 2.f, 280.f});
            window.draw(settingsText2);

            // Menu button
            RectangleShape menuBtnPause(Vector2f(pauseBtnW, 50));
            menuBtnPause.setPosition(sf::Vector2f{pauseBtnX, 340.f});
            menuBtnPause.setFillColor(Color(150, 100, 100));
            window.draw(menuBtnPause);
            Text menuTextPause(font);
            menuTextPause.setString("MENU");
            menuTextPause.setCharacterSize(24);
            menuTextPause.setFillColor(Color::White);
            float menuTxtPauseW = menuTextPause.getLocalBounds().size.x;
            menuTextPause.setPosition(sf::Vector2f{pauseBtnX + (pauseBtnW - menuTxtPauseW) / 2.f, 350.f});
            window.draw(menuTextPause);
        }

        // ===== SETTINGS MENU RENDERING =====
        // Draw settings interface with sliders and toggles
        if (gameState == GameState::SETTINGS) {
            Text settingsTitle(font);
            settingsTitle.setString("SETTINGS");
            settingsTitle.setCharacterSize(40);
            settingsTitle.setFillColor(Color::Cyan);
            float settingsTitleW = settingsTitle.getLocalBounds().size.x;
            settingsTitle.setPosition(sf::Vector2f{(PLAY_W_PX + SIDEBAR_W - settingsTitleW) / 2.f, 50.f});
            window.draw(settingsTitle);

            // --- Music Volume --- (row Y = 130)
            Text musicLabel(font);
            musicLabel.setString("Music Volume");
            musicLabel.setCharacterSize(20);
            musicLabel.setFillColor(Color::White);
            musicLabel.setPosition(sf::Vector2f{100.f, 130.f});
            window.draw(musicLabel);

            // Left arrow
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

            // Right arrow
            Text musicRightArrow(font);
            musicRightArrow.setString(">");
            musicRightArrow.setCharacterSize(20);
            musicRightArrow.setFillColor(Color::Yellow);
            musicRightArrow.setPosition(sf::Vector2f{490.f, 130.f});
            window.draw(musicRightArrow);

            Text musicValue(font);
            musicValue.setString(to_string((int)musicVolume) + "%");
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
            sfxValue.setString(to_string((int)sfxVolume) + "%");
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

            RectangleShape brightnessSliderFill(Vector2f(((brightness - 51.f) / (255.f - 51.f)) * 200.f, 20));
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
            brightnessValue.setString(to_string((int)(((brightness - 51.f) / 204.f) * 80.f + 20.f)) + "%");
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

            // Checkbox - aligned with slider position
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

            // Back button - centered (row Y = 380)
            const float backBtnW = 200.f;
            const float backBtnX = (PLAY_W_PX + SIDEBAR_W - backBtnW) / 2.f;
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

        // ===== BRIGHTNESS OVERLAY =====
        // Apply darkening overlay based on brightness setting
        if (brightness < 255.f) {
            RectangleShape darkenOverlay(Vector2f(PLAY_W_PX + SIDEBAR_W, PLAY_H_PX));
            darkenOverlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(255 - brightness)));
            window.draw(darkenOverlay);
        }

        // ===== MOUSE CURSOR HOVER EFFECTS =====
        // Change cursor to hand when hovering over buttons
        {
            Vector2i pixelPos = Mouse::getPosition(window);
            Vector2f mp = window.mapPixelToCoords(pixelPos);
            bool isHovering = false;

            if (gameState == GameState::MENU) {
                // Check menu buttons
                if (isClicked(startBtn, mp) || isClicked(settingBtn, mp) || isClicked(exitBtn, mp)) {
                    isHovering = true;
                }
            }
            else if (gameState == GameState::SETTINGS) {
                const float backBtnW = 200.f;
                const float backBtnX = (PLAY_W_PX + SIDEBAR_W - backBtnW) / 2.f;
                // Music row: arrows and slider
                if ((mp.x >= 255 && mp.x <= 280 && mp.y >= 125 && mp.y <= 155) ||
                    (mp.x >= 485 && mp.x <= 510 && mp.y >= 125 && mp.y <= 155) ||
                    (mp.x >= 285 && mp.x <= 485 && mp.y >= 127 && mp.y <= 157)) {
                    isHovering = true;
                }
                // SFX row
                if ((mp.x >= 255 && mp.x <= 280 && mp.y >= 185 && mp.y <= 215) ||
                    (mp.x >= 485 && mp.x <= 510 && mp.y >= 185 && mp.y <= 215) ||
                    (mp.x >= 285 && mp.x <= 485 && mp.y >= 187 && mp.y <= 217)) {
                    isHovering = true;
                }
                // Brightness row
                if ((mp.x >= 255 && mp.x <= 280 && mp.y >= 245 && mp.y <= 275) ||
                    (mp.x >= 485 && mp.x <= 510 && mp.y >= 245 && mp.y <= 275) ||
                    (mp.x >= 285 && mp.x <= 485 && mp.y >= 247 && mp.y <= 277)) {
                    isHovering = true;
                }
                // Ghost checkbox
                if (mp.x >= 280 && mp.x <= 315 && mp.y >= 303 && mp.y <= 337) {
                    isHovering = true;
                }
                // Back button
                if (mp.x > backBtnX && mp.x < backBtnX + backBtnW && mp.y > 375 && mp.y < 435) {
                    isHovering = true;
                }
            }
            else if (gameState == GameState::PAUSE) {
                const float fullW = PLAY_W_PX + SIDEBAR_W;
                const float pauseBtnW = 200.f;
                const float pauseBtnX = (fullW - pauseBtnW) / 2.f;
                // Pause menu buttons
                if ((mp.x > pauseBtnX && mp.x < pauseBtnX + pauseBtnW && mp.y > 200 && mp.y < 250) ||
                    (mp.x > pauseBtnX && mp.x < pauseBtnX + pauseBtnW && mp.y > 270 && mp.y < 320) ||
                    (mp.x > pauseBtnX && mp.x < pauseBtnX + pauseBtnW && mp.y > 340 && mp.y < 390)) {
                    isHovering = true;
                }
            }
            else if (gameState == GameState::PLAYING && isGameOver) {
                const float fullW = PLAY_W_PX + SIDEBAR_W;
                const float goBtnW = 200.f;
                const float goBtnX = (fullW - goBtnW) / 2.f;
                // Game over buttons
                if ((mp.x > goBtnX && mp.x < goBtnX + goBtnW && mp.y > 230 && mp.y < 280) ||
                    (mp.x > goBtnX && mp.x < goBtnX + goBtnW && mp.y > 300 && mp.y < 350) ||
                    (mp.x > goBtnX && mp.x < goBtnX + goBtnW && mp.y > 370 && mp.y < 420)) {
                    isHovering = true;
                }
            }

            if (arrowCursor && handCursor) {
                window.setMouseCursor(isHovering ? *handCursor : *arrowCursor);
            }
        }

        window.display();
    }

    // Cleanup
    delete currentPiece;
    delete nextPiece;
    delete clearSound;
    delete landSound;
    delete gameOverSound;
    delete settingClickSound;

    return 0;
}