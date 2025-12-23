#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cstdio>

using namespace std;
using namespace sf;

// --- GAME CONFIGURATION ---
const int TILE_SIZE = 30;
const int H = 20;
const int W = 15;
const int SIDEBAR_W = 6 * TILE_SIZE;
const int PLAY_W_PX = W * TILE_SIZE;
const int PLAY_H_PX = H * TILE_SIZE;

// --- GLOBAL VARIABLES ---
char board[H][W] = {};
int x = 4, y = 0;
float gameDelay = 0.8f;
bool isGameOver = false;

// --- SIDEBAR UI VARIABLES ---
int gScore = 0;
int gLines = 0;
int gLevel = 0;
int currentLevel = 0;

// --- SETTINGS ---
float musicVolume = 50.f;
float sfxVolume = 50.f;
float brightness = 255.f;
bool ghostPieceEnabled = true;

// --- AUDIO RESOURCES ---
sf::SoundBuffer clearBuffer;
sf::Sound* clearSound = nullptr;

sf::SoundBuffer landBuffer;
sf::Sound* landSound = nullptr;

sf::SoundBuffer gameOverBuffer;
sf::Sound* gameOverSound = nullptr;

sf::SoundBuffer settingClickBuffer;
sf::Sound* settingClickSound = nullptr;

sf::Music bgMusic;

// --- game state ---
enum class GameState {
    MENU,
    PLAYING,
    SETTINGS,
};
GameState gameState = GameState::MENU;

// --- PIECE CLASSES ---
class Piece {
public:
    char shape[4][4];

    Piece() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                shape[i][j] = ' ';
            }
        }
    }

    virtual ~Piece() {}

    virtual void rotate(int currentX, int currentY) {
        char temp[4][4];

        // 1. Rotate to temp matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[j][3 - i] = shape[i][j];
            }
        }

        // 2. Try wall kicks: original, left 1, right 1, left 2, right 2
        int kicks[] = {0, -1, 1, -2, 2};
        for (int kick : kicks) {
            bool canPlace = true;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (temp[i][j] != ' ') {
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
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        shape[i][j] = temp[i][j];
                    }
                }
                ::x += kick;
                return;
            }
        }
    }
};

class IPiece : public Piece {
public:
    IPiece() {
        shape[0][1] = 'I';
        shape[1][1] = 'I';
        shape[2][1] = 'I';
        shape[3][1] = 'I';
    }
};

class OPiece : public Piece {
public:
    OPiece() {
        shape[1][1] = 'O'; shape[1][2] = 'O';
        shape[2][1] = 'O'; shape[2][2] = 'O';
    }
    void rotate(int, int) override {}
};

class TPiece : public Piece {
public:
    TPiece() {
        shape[1][1] = 'T';
        shape[2][0] = 'T'; shape[2][1] = 'T'; shape[2][2] = 'T';
    }
};

class SPiece : public Piece {
public:
    SPiece() {
        shape[1][1] = 'S'; shape[1][2] = 'S';
        shape[2][0] = 'S'; shape[2][1] = 'S';
    }
};

class ZPiece : public Piece {
public:
    ZPiece() {
        shape[1][0] = 'Z'; shape[1][1] = 'Z';
        shape[2][1] = 'Z'; shape[2][2] = 'Z';
    }
};

class JPiece : public Piece {
public:
    JPiece() {
        shape[1][0] = 'J';
        shape[2][0] = 'J'; shape[2][1] = 'J'; shape[2][2] = 'J';
    }
};

class LPiece : public Piece {
public:
    LPiece() {
        shape[1][2] = 'L';
        shape[2][0] = 'L'; shape[2][1] = 'L'; shape[2][2] = 'L';
    }
};

Color getColor(char c) {
    switch (c) {
        case 'I': return Color::Cyan;
        case 'J': return Color::Blue;
        case 'L': return Color(255, 165, 0);
        case 'O': return Color::Yellow;
        case 'S': return Color::Green;
        case 'T': return Color(128, 0, 128);
        case 'Z': return Color::Red;
        case '#': return Color(100, 100, 100);
        default:  return Color::Black;
    }
}

// --- SIDEBAR UI ---
struct SidebarUI {
    float x, y, w, h;
    float pad;
    float boxW;
    sf::FloatRect scoreBox;
    sf::FloatRect levelBox;
    sf::FloatRect linesBox;
    sf::FloatRect nextBox;
};

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

static void drawText(sf::RenderWindow& window, const sf::Font& font, const std::string& s, float x, float y, unsigned size) {
    sf::Text t(font, s, size);
    t.setFillColor(sf::Color::White);
    t.setPosition({x, y});
    window.draw(t);
}

static void drawNextPreview(sf::RenderWindow& window, const SidebarUI& ui, const Piece* p) {
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
    int mini = TILE_SIZE / 2;

    sf::Vector2f areaPos = { ui.nextBox.position.x + 16.f, ui.nextBox.position.y + 60.f };
    sf::Vector2f areaSize = { ui.nextBox.size.x - 32.f, ui.nextBox.size.y - 80.f };
    float startX = areaPos.x + (areaSize.x - cellsW * mini) * 0.5f;
    float startY = areaPos.y + (areaSize.y - cellsH * mini) * 0.5f;

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

static void drawSidebar(sf::RenderWindow& window, const SidebarUI& ui,
                        const sf::Font& font, int score, int level, int lines,
                        const Piece* next) {
    sf::RectangleShape bg({ui.w, ui.h});
    bg.setPosition({ui.x, ui.y});
    bg.setFillColor(sf::Color(30, 30, 30));
    window.draw(bg);

    drawPanel(window, ui.scoreBox);
    drawPanel(window, ui.levelBox);
    drawPanel(window, ui.linesBox);
    drawPanel(window, ui.nextBox);

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

// --- GAME LOGIC ---
Piece* currentPiece = nullptr;
Piece* nextPiece = nullptr;

Piece* createRandomPiece() {
    int r = rand() % 7;
    switch (r) {
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

void block2Board() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] != ' ') {
                board[y + i][x + j] = currentPiece->shape[i][j];
            }
        }
    }
}

void initBoard() {
    for (int i = 0; i < H; i++) {
        for (int j = 0; j < W; j++) {
            if ((i == H - 1) || (j == 0) || (j == W - 1)) {
                board[i][j] = '#';
            } else {
                board[i][j] = ' ';
            }
        }
    }
}

bool canMove(int dx, int dy) {
    if (!currentPiece) return false;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece->shape[i][j] != ' ') {
                int tx = x + j + dx;
                int ty = y + i + dy;
                if (tx < 1 || tx >= W - 1 || ty >= H - 1) return false;
                if (board[ty][tx] != ' ') return false;
            }
        }
    }
    return true;
}

int getGhostY() {
    int ghostY = y;
    while (true) {
        bool canGo = true;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentPiece->shape[i][j] != ' ') {
                    int tx = x + j;
                    int ty = ghostY + i + 1;
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

void SpeedIncrement() {
    if (gameDelay > 0.1f) {
        gameDelay -= 0.08f;
    }
}

void applyLineClearScore(int cleared) {
    if (cleared <= 0) return;
    gLines += cleared;
    gScore += 100 * cleared;
    gLevel = gLines / 10;
    if (gLevel > currentLevel) {
        SpeedIncrement();
        currentLevel = gLevel;
    }
}

int removeLine() {
    int cleared = 0;
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') { isFull = false; break; }
        }
        if (isFull) {
            cleared++;
            clearSound->play();
            for (int k = i; k > 0; k--) {
                for (int j = 1; j < W - 1; j++) {
                    board[k][j] = (k != 1) ? board[k - 1][j] : ' ';
                }
            }
            i++;
        }
    }
    return cleared;
}

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

// --- MAIN FUNCTION ---
int main() {
    RenderWindow window(VideoMode(Vector2u(PLAY_W_PX + SIDEBAR_W, PLAY_H_PX)), "SS008 - Tetris");
    window.setFramerateLimit(60);

    // Set window icon from logo.png
    sf::Image icon;
    if (icon.loadFromFile("logo.png")) {
        window.setIcon(icon);
    }

    SidebarUI ui = makeSidebarUI();

    // Load Audio
    if (!bgMusic.openFromFile("loop_theme.ogg")) return -1;
    if (!clearBuffer.loadFromFile("line_clear.ogg")) return -1;
    if (!landBuffer.loadFromFile("bumper_end.ogg")) return -1;
    if (!gameOverBuffer.loadFromFile("game_over.ogg")) return -1;
    if (!settingClickBuffer.loadFromFile("insetting_click.ogg")) return -1;

    clearSound = new sf::Sound(clearBuffer);
    landSound = new sf::Sound(landBuffer);
    gameOverSound = new sf::Sound(gameOverBuffer);
    settingClickSound = new sf::Sound(settingClickBuffer);

    bgMusic.setLooping(true);
    bgMusic.setVolume(musicVolume);
    bgMusic.play();

    // Init Game
    srand((unsigned)time(0));
    currentPiece = createRandomPiece();
    nextPiece = createRandomPiece();
    initBoard();

    Clock clock;
    float timer = 0.f;
    float inputTimer = 0.f;
    const float inputDelay = 0.1f;

    // UI menu
    Font font;
    if (!font.openFromFile("Monocraft.ttf")) return -1;

    // ===== TITLE ===== (centered in full window)
    Text title(font);
    title.setString("SS008 - TETRIS");
    title.setCharacterSize(36);
    title.setFillColor(Color::Cyan);
    float titleWidth = title.getLocalBounds().size.x;
    title.setPosition(sf::Vector2f{(PLAY_W_PX + SIDEBAR_W - titleWidth) / 2.f, 60.f});

    // ===== BUTTON FACTORY ===== (centered in full window)
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

    auto [startBtn, startText]     = createButton("START",    160);
    auto [settingBtn, settingText] = createButton("SETTINGS", 230);
    auto [exitBtn, exitText]       = createButton("EXIT",     300);

    auto isClicked = [&](RectangleShape& btn, Vector2i mousePos) {
        FloatRect bounds = btn.getGlobalBounds();
        return bounds.contains(Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)));
    };

    // Cursors for hover effect
    auto arrowCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow);
    auto handCursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand);

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (!isGameOver && gameState == GameState::PLAYING) {
            timer += dt;
            inputTimer += dt;
        }

        // --- EVENTS ---
        while (const auto event = window.pollEvent()) {
            // ===== MENU CLICK =====
            if (gameState == GameState::MENU) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i mousePos = Mouse::getPosition(window);
                        if (isClicked(startBtn, mousePos)) {
                            resetGame();
                            gameState = GameState::PLAYING;
                            continue; // Prevent click-through
                        }
                        if (isClicked(settingBtn, mousePos)) {
                            gameState = GameState::SETTINGS;
                            continue; // Prevent click-through
                        }
                        if (isClicked(exitBtn, mousePos)) {
                            window.close();
                        }
                    }
                }
            }

            if (event->is<Event::Closed>()) {
                window.close();
            }

            // ===== GAME OVER CLICK =====
            if (isGameOver && gameState == GameState::PLAYING) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i mousePos = Mouse::getPosition(window);
                        const float fullW = PLAY_W_PX + SIDEBAR_W;
                        const float goBtnW = 200.f;
                        const float goBtnX = (fullW - goBtnW) / 2.f;
                        // Restart button
                        if (mousePos.x > goBtnX && mousePos.x < goBtnX + goBtnW && mousePos.y > 230 && mousePos.y < 280) {
                            resetGame();
                            bgMusic.play();
                        }
                        // Menu button
                        if (mousePos.x > goBtnX && mousePos.x < goBtnX + goBtnW && mousePos.y > 300 && mousePos.y < 350) {
                            resetGame();
                            gameState = GameState::MENU;
                            bgMusic.play();
                        }
                        // Exit button
                        if (mousePos.x > goBtnX && mousePos.x < goBtnX + goBtnW && mousePos.y > 370 && mousePos.y < 420) {
                            window.close();
                        }
                    }
                }
            }

            // ===== PLAYING KEYS =====
            if (gameState == GameState::PLAYING && !isGameOver) {
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    if (keyPressed->code == Keyboard::Key::W) {
                        currentPiece->rotate(x, y);
                    }
                    else if (keyPressed->code == Keyboard::Key::Space) {
                        while (canMove(0, 1)) y++;
                        timer = gameDelay + 10.0f;
                    }
                    else if (keyPressed->code == Keyboard::Key::Escape) {
                        gameState = GameState::MENU;
                    }
                }
            }
            else if (gameState == GameState::MENU) {
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    if (keyPressed->code == Keyboard::Key::Enter) {
                        resetGame();
                        gameState = GameState::PLAYING;
                    }
                    if (keyPressed->code == Keyboard::Key::Escape) {
                        window.close();
                    }
                }
            }

            // ===== SETTINGS CLICK =====
            if (gameState == GameState::SETTINGS) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i mousePos = Mouse::getPosition(window);

                        // Music row Y=130: Left arrow at 260, slider 285-485, right arrow at 490
                        if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 125 && mousePos.y <= 155) {
                            musicVolume = max(0.f, musicVolume - 5.f);
                            bgMusic.setVolume(musicVolume);
                            settingClickSound->play();
                        }
                        if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 125 && mousePos.y <= 155) {
                            musicVolume = min(100.f, musicVolume + 5.f);
                            bgMusic.setVolume(musicVolume);
                            settingClickSound->play();
                        }
                        if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 127 && mousePos.y <= 157) {
                            musicVolume = static_cast<float>(mousePos.x - 285) / 2.f;
                            musicVolume = max(0.f, min(100.f, musicVolume));
                            bgMusic.setVolume(musicVolume);
                            settingClickSound->play();
                        }

                        // SFX row Y=190: Left arrow at 260, slider 285-485, right arrow at 490
                        if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 185 && mousePos.y <= 215) {
                            sfxVolume = max(0.f, sfxVolume - 5.f);
                            clearSound->setVolume(sfxVolume);
                            landSound->setVolume(sfxVolume);
                            gameOverSound->setVolume(sfxVolume);
                            settingClickSound->setVolume(sfxVolume);
                            settingClickSound->play();
                        }
                        if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 185 && mousePos.y <= 215) {
                            sfxVolume = min(100.f, sfxVolume + 5.f);
                            clearSound->setVolume(sfxVolume);
                            landSound->setVolume(sfxVolume);
                            gameOverSound->setVolume(sfxVolume);
                            settingClickSound->setVolume(sfxVolume);
                            settingClickSound->play();
                        }
                        if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 187 && mousePos.y <= 217) {
                            sfxVolume = static_cast<float>(mousePos.x - 285) / 2.f;
                            sfxVolume = max(0.f, min(100.f, sfxVolume));
                            clearSound->setVolume(sfxVolume);
                            landSound->setVolume(sfxVolume);
                            gameOverSound->setVolume(sfxVolume);
                            settingClickSound->setVolume(sfxVolume);
                            settingClickSound->play();
                        }

                        // Brightness row Y=250: Left arrow at 260, slider 285-485, right arrow at 490
                        if (mousePos.x >= 255 && mousePos.x <= 280 && mousePos.y >= 245 && mousePos.y <= 275) {
                            brightness = max(50.f, brightness - 10.f);
                            settingClickSound->play();
                        }
                        if (mousePos.x >= 485 && mousePos.x <= 510 && mousePos.y >= 245 && mousePos.y <= 275) {
                            brightness = min(255.f, brightness + 10.f);
                            settingClickSound->play();
                        }
                        if (mousePos.x >= 285 && mousePos.x <= 485 && mousePos.y >= 247 && mousePos.y <= 277) {
                            brightness = static_cast<float>(mousePos.x - 285) / 200.f * 255.f;
                            brightness = max(50.f, min(255.f, brightness));
                            settingClickSound->play();
                        }

                        // Ghost Piece checkbox at 285, 308 size 24x24
                        if (mousePos.x >= 280 && mousePos.x <= 315 && mousePos.y >= 303 && mousePos.y <= 337) {
                            ghostPieceEnabled = !ghostPieceEnabled;
                            settingClickSound->play();
                        }

                        // Back button centered at Y=380
                        const float backBtnW = 200.f;
                        const float backBtnX = (PLAY_W_PX + SIDEBAR_W - backBtnW) / 2.f;
                        if (mousePos.x > backBtnX && mousePos.x < backBtnX + backBtnW && mousePos.y > 375 && mousePos.y < 435) {
                            gameState = GameState::MENU;
                            settingClickSound->play();
                        }
                    }
                }
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    if (keyPressed->code == Keyboard::Key::Escape) {
                        gameState = GameState::MENU;
                    }
                }
            }
        }

        // --- INPUT (Continuous) ---
        if (gameState == GameState::PLAYING && !isGameOver) {
            if (inputTimer > inputDelay) {
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

            // --- GRAVITY ---
            if (timer > gameDelay) {
                if (canMove(0, 1)) {
                    y++;
                } else {
                    landSound->play();
                    block2Board();
                    int cleared = removeLine();
                    applyLineClearScore(cleared);
                    delete currentPiece;
                    currentPiece = nextPiece;
                    nextPiece = createRandomPiece();
                    x = 4;
                    y = 0;
                    if (!canMove(0, 0)) {
                        isGameOver = true;
                        gameOverSound->play();
                        bgMusic.stop();
                    }
                }
                timer = 0;
            }
        }

        // --- RENDER ---
        window.clear(Color::Black);

        // ===== MENU =====
        if (gameState == GameState::MENU) {
            window.draw(title);
            window.draw(startBtn);
            window.draw(startText);
            window.draw(settingBtn);
            window.draw(settingText);
            window.draw(exitBtn);
            window.draw(exitText);
        }

        // ===== GAME =====
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

        // ===== SETTINGS SCREEN =====
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
            brightnessValue.setString(to_string((int)(brightness / 255.f * 100.f)) + "%");
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

        // Apply brightness overlay
        if (brightness < 255.f) {
            RectangleShape darkenOverlay(Vector2f(PLAY_W_PX + SIDEBAR_W, PLAY_H_PX));
            darkenOverlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(255 - brightness)));
            window.draw(darkenOverlay);
        }

        // --- CURSOR HOVER LOGIC ---
        {
            Vector2i mp = Mouse::getPosition(window);
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