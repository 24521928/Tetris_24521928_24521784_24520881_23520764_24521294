#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <ctime>
#include <algorithm>
#include <optional>

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
float gameDelay = 0.5f;
bool isGameOver = false; // Trạng thái game

// --- SETTINGS ---
float musicVolume = 50.f;
float sfxVolume = 50.f;
float brightness = 255.f;

// --- AUDIO RESOURCES ---
sf::SoundBuffer clearBuffer;
sf::Sound clearSound(clearBuffer);

sf::SoundBuffer landBuffer;
sf::Sound landSound(landBuffer);

sf::SoundBuffer settingClickBuffer;
sf::Sound settingClickSound(settingClickBuffer);

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

        // 2. Check collision
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (temp[i][j] != ' ') {
                    int tx = currentX + j;
                    int ty = currentY + i;
                    
                    // Check boundaries and existing blocks
                    if (tx < 1 || tx >= W - 1 || ty >= H - 1) return;
                    if (board[ty][tx] != ' ') return;
                }
            }
        }

        // 3. Apply rotation
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                shape[i][j] = temp[i][j];
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
    void rotate(int, int) override {
        // O Piece does not rotate
    }
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
        case 'L': return Color(255, 165, 0); // Orange
        case 'O': return Color::Yellow;
        case 'S': return Color::Green;
        case 'T': return Color(128, 0, 128); // Purple
        case 'Z': return Color::Red;
        case '#': return Color(100, 100, 100); // Grey
        default:  return Color::Black;
    }
}
// --- SIDE BAR UI ---
// sidebar UI properties 
int gScore = 0;
int gLines = 0;
int gLevel = 0;
int currentLevel = 0;
//
// Draw UI
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
    // 1) background
    sf::RectangleShape bg({ui.w, ui.h});
    bg.setPosition({ui.x, ui.y});
    bg.setFillColor(sf::Color(30, 30, 30));
    window.draw(bg);

    // 2) panels
    drawPanel(window, ui.scoreBox);
    drawPanel(window, ui.levelBox);
    drawPanel(window, ui.linesBox);
    drawPanel(window, ui.nextBox);

    // 3) labels + values (use x,y floats to match your drawText signature)
    float labelX = ui.scoreBox.position.x + 12.f;

    drawText(window, font, "SCORE", labelX, ui.scoreBox.position.y + 10.f, 18);
    drawText(window, font, std::to_string(score), labelX, ui.scoreBox.position.y + 42.f, 24);

    drawText(window, font, "LEVEL", labelX, ui.levelBox.position.y + 10.f, 18);
    drawText(window, font, std::to_string(level), labelX, ui.levelBox.position.y + 42.f, 24);

    drawText(window, font, "LINES", labelX, ui.linesBox.position.y + 10.f, 18);
    drawText(window, font, std::to_string(lines), labelX, ui.linesBox.position.y + 42.f, 24);

    drawText(window, font, "NEXT", labelX, ui.nextBox.position.y + 10.f, 18);

    // 4) preview
    drawNextPreview(window, ui, next);
}
// update score, level, lines
void SpeedIncrement() {
    if (gameDelay > 0.1f) {
        gameDelay -= 0.08f;
    }
}
void applyLineClearScore(int cleared) {
    if (cleared <= 0) return;
    printf("Cleared Lines: %d\n", cleared);
    printf("Score: %d\n", gScore);
    printf("level: %d\n", gLevel);
    gLines += cleared;                  
    gScore += 100 * cleared;            // a cleared line give 100 points
    gLevel = 0 + (gLines / 10);         // a level costs 10 lines
    if (gLevel > currentLevel) {
        SpeedIncrement();
        currentLevel = gLevel;
    }
}

// --- GAME LOGIC ---

Piece* currentPiece = nullptr;

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

void restartGame() {
    initBoard();
    if (currentPiece) delete currentPiece;
    currentPiece = createRandomPiece();
    x = 4; y = 0;
    gameDelay = 0.5f;
    isGameOver = false;
}

int removeLine() {
    int cleared = 0;
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') {
                isFull = false;
                break;
            }
        }

        if (isFull) {
            cleared++;
            clearSound.play();
            
            // Shift rows down
            for (int k = i; k > 0; k--) {
                for (int j = 1; j < W - 1; j++) {
                    if (k != 1) {
                        board[k][j] = board[k - 1][j];
                    } else {
                        board[k][j] = ' ';
                    }
                }
            }
            i++; // Re-check the current row
            SpeedIncrement();
        }
    }
    return cleared;
}

void resetGame() {
    initBoard();

    delete currentPiece;
    currentPiece = createRandomPiece();

    x = 4;
    y = 0;
    gameDelay = 0.5f;
}

// --- MAIN FUNCTION ---

int main() {
    RenderWindow window(VideoMode(Vector2u(W * TILE_SIZE, H * TILE_SIZE)), "SS008 - Tetris");
    window.setFramerateLimit(60);
    SidebarUI ui = makeSidebarUI();
    // add font
    sf::Font font;
    if (!font.openFromFile("Monocraft.ttf")) {
        return -1;
    }

    // Load Audio
    if (!bgMusic.openFromFile("loop_theme.wav")) return -1;
    if (!clearBuffer.loadFromFile("line_clear.wav")) return -1;
    if (!landBuffer.loadFromFile("bumper_end.wav")) return -1;
    if (!settingClickBuffer.loadFromFile("insetting_click.wav")) return -1;

    bgMusic.setLooping(true);
    bgMusic.setVolume(musicVolume);
    bgMusic.play();

    // Init Game
    srand(time(0));
    currentPiece = createRandomPiece();
    Piece* nextPiece = createRandomPiece(); // next piece to preview on side bar
    initBoard();

    Clock clock;
    float timer = 0;
    float inputTimer = 0;
    const float inputDelay = 0.1f;


    // ===== TITLE ===== (centered in full window)
    Text title(font);
    title.setString("SS008 - TETRIS");
    title.setCharacterSize(36);
    title.setFillColor(Color::Cyan);
    float titleWidth = title.getLocalBounds().size.x;
    title.setPosition(sf::Vector2f{(W * TILE_SIZE - titleWidth) / 2.f, 60.f});

    // ===== BUTTON FACTORY ===== (centered in full window)
    const float btnWidth = 200.f;
    const float btnX = (W * TILE_SIZE - btnWidth) / 2.f;
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

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        
        if (gameState == GameState::PLAYING) {
            timer += time;
            inputTimer += time;
        }

        // --- EVENTS ---
        while (const auto event = window.pollEvent()) {
            // ===== MENU CLICK =====
            if (gameState == GameState::MENU) {
                if (const auto* mouse = event->getIf<Event::MouseButtonPressed>()) {
                    if (mouse->button == Mouse::Button::Left) {
                        Vector2i mousePos = Mouse::getPosition(window);
                        if (isClicked(startBtn, mousePos)) {
                            gameState = GameState::PLAYING;

                            initBoard();
                            delete currentPiece;
                            currentPiece = createRandomPiece();
                            x = 4;
                            y = 0;
                            gameDelay = 0.5f;
                        }
                        if (isClicked(settingBtn, mousePos)) {
                            gameState = GameState::SETTINGS;
                            continue;
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

            // Logic Click chuột khi Game Over
            if (isGameOver && event->is<Event::MouseButtonPressed>()) {
                Vector2i m = Mouse::getPosition(window);
                if (m.x > 125 && m.x < 325 && m.y > 250 && m.y < 300) restartGame();
                if (m.x > 125 && m.x < 325 && m.y > 330 && m.y < 380) window.close();
            }

            // ===== PLAYING KEYS =====
            if (gameState == GameState::PLAYING) {
                if (!isGameOver) {
                    // ===== INPUT =====
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
            }
            else if (gameState == GameState::MENU) {
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    if (keyPressed->code == Keyboard::Key::Enter) {
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
                        const float sliderX = 70.f;  // Căn giữa hơn
                        const float sliderW = 200.f;
                        const float arrowGap = 20.f; // Khoảng cách mũi tên đến slider

                        // Music slider row Y=155
                        // Left arrow
                        if (mousePos.x >= sliderX && mousePos.x <= sliderX + 15 && mousePos.y >= 150 && mousePos.y <= 180) {
                            musicVolume = max(0.f, musicVolume - 5.f);
                            bgMusic.setVolume(musicVolume);
                            settingClickSound.play();
                        }
                        // Right arrow
                        if (mousePos.x >= sliderX + arrowGap + sliderW && mousePos.x <= sliderX + arrowGap + sliderW + 15 && mousePos.y >= 150 && mousePos.y <= 180) {
                            musicVolume = min(100.f, musicVolume + 5.f);
                            bgMusic.setVolume(musicVolume);
                            settingClickSound.play();
                        }
                        // Slider bar
                        if (mousePos.x >= sliderX + arrowGap && mousePos.x <= sliderX + arrowGap + sliderW && mousePos.y >= 152 && mousePos.y <= 178) {
                            musicVolume = static_cast<float>(mousePos.x - (sliderX + arrowGap)) / sliderW * 100.f;
                            musicVolume = max(0.f, min(100.f, musicVolume));
                            bgMusic.setVolume(musicVolume);
                            settingClickSound.play();
                        }

                        // SFX slider row Y=235
                        if (mousePos.x >= sliderX && mousePos.x <= sliderX + 15 && mousePos.y >= 230 && mousePos.y <= 260) {
                            sfxVolume = max(0.f, sfxVolume - 5.f);
                            clearSound.setVolume(sfxVolume);
                            landSound.setVolume(sfxVolume);
                            settingClickSound.setVolume(sfxVolume);
                            settingClickSound.play();
                        }
                        if (mousePos.x >= sliderX + arrowGap + sliderW && mousePos.x <= sliderX + arrowGap + sliderW + 15 && mousePos.y >= 230 && mousePos.y <= 260) {
                            sfxVolume = min(100.f, sfxVolume + 5.f);
                            clearSound.setVolume(sfxVolume);
                            landSound.setVolume(sfxVolume);
                            settingClickSound.setVolume(sfxVolume);
                            settingClickSound.play();
                        }
                        if (mousePos.x >= sliderX + arrowGap && mousePos.x <= sliderX + arrowGap + sliderW && mousePos.y >= 232 && mousePos.y <= 258) {
                            sfxVolume = static_cast<float>(mousePos.x - (sliderX + arrowGap)) / sliderW * 100.f;
                            sfxVolume = max(0.f, min(100.f, sfxVolume));
                            clearSound.setVolume(sfxVolume);
                            landSound.setVolume(sfxVolume);
                            settingClickSound.setVolume(sfxVolume);
                            settingClickSound.play();
                        }

                        // Brightness slider row Y=315
                        if (mousePos.x >= sliderX && mousePos.x <= sliderX + 15 && mousePos.y >= 310 && mousePos.y <= 340) {
                            brightness = max(50.f, brightness - 10.f);
                            settingClickSound.play();
                        }
                        if (mousePos.x >= sliderX + arrowGap + sliderW && mousePos.x <= sliderX + arrowGap + sliderW + 15 && mousePos.y >= 310 && mousePos.y <= 340) {
                            brightness = min(255.f, brightness + 10.f);
                            settingClickSound.play();
                        }
                        if (mousePos.x >= sliderX + arrowGap && mousePos.x <= sliderX + arrowGap + sliderW && mousePos.y >= 312 && mousePos.y <= 338) {
                            brightness = static_cast<float>(mousePos.x - (sliderX + arrowGap)) / sliderW * 255.f;
                            brightness = max(50.f, min(255.f, brightness));
                            settingClickSound.play();
                        }

                        // Back button centered at Y=400
                        const float backBtnW = 200.f;
                        const float backBtnX = (W * TILE_SIZE - backBtnW) / 2.f;
                        if (mousePos.x > backBtnX && mousePos.x < backBtnX + backBtnW && mousePos.y > 395 && mousePos.y < 455) {
                            gameState = GameState::MENU;
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
        if (gameState == GameState::PLAYING) {
            if (inputTimer > inputDelay) {
                if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
                    if (canMove(-1, 0)) x--;
                    inputTimer = 0;
                } else if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
                    if (canMove(1, 0)) x++;
                    inputTimer = 0;
                } else if (Keyboard::isKeyPressed(Keyboard::Key::S)) {
                    if (canMove(0, 1)) y++;
                    inputTimer = 0;
                }
            }

            // --- GRAVITY & LOGIC ---
            if (timer > gameDelay) {
                if (canMove(0, 1)) y++;
                else {
                    landSound.play();
                    block2Board(); // Gọi hàm của bạn
                    int cleared = removeLine(); // check if a line is removed 
                    applyLineClearScore(cleared);  // apply changes for a cleared line
                    delete currentPiece;
                    currentPiece = nextPiece;   // assign nextpiece for currentpiece
                    nextPiece = createRandomPiece();    // random next piece
                    x = 4; y = 0;
                    if (!canMove(0, 0)) isGameOver = true;
                }
                timer = 0;
            }
        }

        // --- RENDER (ONLY ONCE) ---
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
                        RectangleShape rect(Vector2f((float)TILE_SIZE - 1, (float)TILE_SIZE - 1));
                        rect.setPosition(Vector2f((float)j * TILE_SIZE, (float)i * TILE_SIZE));
                        rect.setFillColor(getColor(board[i][j]));
                        window.draw(rect);
                    }
                }
            }

            // Draw Current Piece
            if (!isGameOver) 
            {
                for (int i = 0; i < 4; i++) 
                    for (int j = 0; j < 4; j++)
                        if (currentPiece->shape[i][j] != ' ') 
                        {
                            RectangleShape rect(Vector2f((float)TILE_SIZE - 1, (float)TILE_SIZE - 1));
                            rect.setPosition(Vector2f((float)(x + j) * TILE_SIZE, (float)(y + i) * TILE_SIZE));
                            rect.setFillColor(getColor(currentPiece->shape[i][j]));
                            window.draw(rect);
                        }
            }
        }
        // Màn hình Game Over
        if (isGameOver) {
            RectangleShape overlay(Vector2f(W * TILE_SIZE, H * TILE_SIZE));
            overlay.setFillColor(Color(0, 0, 0, 200));
            window.draw(overlay);

            Text t1(font, "GAME OVER", 45);
            t1.setFillColor(Color::Red);
            t1.setPosition(Vector2f(85, 150));
            window.draw(t1);

            // Nút Restart
            RectangleShape b1(Vector2f(200, 50));
            b1.setPosition(Vector2f(125, 250));
            b1.setFillColor(Color(0, 0, 255));
            window.draw(b1);
            Text rT(font, "RESTART", 25);
            rT.setPosition(Vector2f(175, 260));
            window.draw(rT);

            // Nút Exit
            RectangleShape b2(Vector2f(200, 50));
            b2.setPosition(Vector2f(125, 330));
            b2.setFillColor(Color(255, 0, 0));
            window.draw(b2);
            Text eT(font, "EXIT", 25);
            eT.setPosition(Vector2f(200, 340));
            window.draw(eT);
        }

        // ===== SETTINGS SCREEN =====
        if (gameState == GameState::SETTINGS) {
            const float sliderX = 70.f;  // Căn giữa hơn
            const float sliderW = 200.f;
            const float arrowGap = 20.f; // Khoảng cách mũi tên đến slider

            Text settingsTitle(font);
            settingsTitle.setString("SETTINGS");
            settingsTitle.setCharacterSize(40);
            settingsTitle.setFillColor(Color::Cyan);
            float settingsTitleW = settingsTitle.getLocalBounds().size.x;
            settingsTitle.setPosition(sf::Vector2f{(W * TILE_SIZE - settingsTitleW) / 2.f, 30.f});
            window.draw(settingsTitle);

            // --- Music Volume ---
            Text musicLabel(font);
            musicLabel.setString("Music Volume");
            musicLabel.setCharacterSize(20);
            musicLabel.setFillColor(Color::White);
            musicLabel.setPosition(sf::Vector2f{sliderX, 120.f});
            window.draw(musicLabel);

            // Slider row Y=155
            Text musicLeftArrow(font);
            musicLeftArrow.setString("<");
            musicLeftArrow.setCharacterSize(22);
            musicLeftArrow.setFillColor(Color::Yellow);
            musicLeftArrow.setPosition(sf::Vector2f{sliderX, 155.f});
            window.draw(musicLeftArrow);

            RectangleShape musicSliderBg(Vector2f(sliderW, 22));
            musicSliderBg.setPosition(sf::Vector2f{sliderX + arrowGap, 157.f});
            musicSliderBg.setFillColor(Color(80, 80, 80));
            window.draw(musicSliderBg);

            RectangleShape musicSliderFill(Vector2f(musicVolume / 100.f * sliderW, 22));
            musicSliderFill.setPosition(sf::Vector2f{sliderX + arrowGap, 157.f});
            musicSliderFill.setFillColor(Color(0, 150, 255));
            window.draw(musicSliderFill);

            Text musicRightArrow(font);
            musicRightArrow.setString(">");
            musicRightArrow.setCharacterSize(22);
            musicRightArrow.setFillColor(Color::Yellow);
            musicRightArrow.setPosition(sf::Vector2f{sliderX + arrowGap + sliderW + 5.f, 155.f});
            window.draw(musicRightArrow);

            Text musicValue(font);
            musicValue.setString(to_string((int)musicVolume) + "%");
            musicValue.setCharacterSize(20);
            musicValue.setFillColor(Color::White);
            musicValue.setPosition(sf::Vector2f{sliderX + arrowGap + sliderW + 30.f, 157.f});
            window.draw(musicValue);

            // --- SFX Volume ---
            Text sfxLabel(font);
            sfxLabel.setString("SFX Volume");
            sfxLabel.setCharacterSize(20);
            sfxLabel.setFillColor(Color::White);
            sfxLabel.setPosition(sf::Vector2f{sliderX, 200.f});
            window.draw(sfxLabel);

            // Slider row Y=235
            Text sfxLeftArrow(font);
            sfxLeftArrow.setString("<");
            sfxLeftArrow.setCharacterSize(22);
            sfxLeftArrow.setFillColor(Color::Yellow);
            sfxLeftArrow.setPosition(sf::Vector2f{sliderX, 235.f});
            window.draw(sfxLeftArrow);

            RectangleShape sfxSliderBg(Vector2f(sliderW, 22));
            sfxSliderBg.setPosition(sf::Vector2f{sliderX + arrowGap, 237.f});
            sfxSliderBg.setFillColor(Color(80, 80, 80));
            window.draw(sfxSliderBg);

            RectangleShape sfxSliderFill(Vector2f(sfxVolume / 100.f * sliderW, 22));
            sfxSliderFill.setPosition(sf::Vector2f{sliderX + arrowGap, 237.f});
            sfxSliderFill.setFillColor(Color(0, 200, 100));
            window.draw(sfxSliderFill);

            Text sfxRightArrow(font);
            sfxRightArrow.setString(">");
            sfxRightArrow.setCharacterSize(22);
            sfxRightArrow.setFillColor(Color::Yellow);
            sfxRightArrow.setPosition(sf::Vector2f{sliderX + arrowGap + sliderW + 5.f, 235.f});
            window.draw(sfxRightArrow);

            Text sfxValue(font);
            sfxValue.setString(to_string((int)sfxVolume) + "%");
            sfxValue.setCharacterSize(20);
            sfxValue.setFillColor(Color::White);
            sfxValue.setPosition(sf::Vector2f{sliderX + arrowGap + sliderW + 30.f, 237.f});
            window.draw(sfxValue);

            // --- Brightness ---
            Text brightnessLabel(font);
            brightnessLabel.setString("Brightness");
            brightnessLabel.setCharacterSize(20);
            brightnessLabel.setFillColor(Color::White);
            brightnessLabel.setPosition(sf::Vector2f{sliderX, 280.f});
            window.draw(brightnessLabel);

            // Slider row Y=315
            Text brightLeftArrow(font);
            brightLeftArrow.setString("<");
            brightLeftArrow.setCharacterSize(22);
            brightLeftArrow.setFillColor(Color::Yellow);
            brightLeftArrow.setPosition(sf::Vector2f{sliderX, 315.f});
            window.draw(brightLeftArrow);

            RectangleShape brightnessSliderBg(Vector2f(sliderW, 22));
            brightnessSliderBg.setPosition(sf::Vector2f{sliderX + arrowGap, 317.f});
            brightnessSliderBg.setFillColor(Color(80, 80, 80));
            window.draw(brightnessSliderBg);

            RectangleShape brightnessSliderFill(Vector2f((brightness / 255.f) * sliderW, 22));
            brightnessSliderFill.setPosition(sf::Vector2f{sliderX + arrowGap, 317.f});
            brightnessSliderFill.setFillColor(Color(255, 200, 50));
            window.draw(brightnessSliderFill);

            Text brightRightArrow(font);
            brightRightArrow.setString(">");
            brightRightArrow.setCharacterSize(22);
            brightRightArrow.setFillColor(Color::Yellow);
            brightRightArrow.setPosition(sf::Vector2f{sliderX + arrowGap + sliderW + 5.f, 315.f});
            window.draw(brightRightArrow);

            Text brightnessValue(font);
            brightnessValue.setString(to_string((int)(brightness / 255.f * 100.f)) + "%");
            brightnessValue.setCharacterSize(20);
            brightnessValue.setFillColor(Color::White);
            brightnessValue.setPosition(sf::Vector2f{sliderX + arrowGap + sliderW + 30.f, 317.f});
            window.draw(brightnessValue);

            // Back button - centered (row Y = 400)
            const float backBtnW = 200.f;
            const float backBtnX = (W * TILE_SIZE - backBtnW) / 2.f;
            RectangleShape backBtn(Vector2f(backBtnW, 50));
            backBtn.setPosition(sf::Vector2f{backBtnX, 400.f});
            backBtn.setFillColor(Color(100, 100, 100));
            window.draw(backBtn);

            Text backText(font);
            backText.setString("BACK");
            backText.setCharacterSize(24);
            backText.setFillColor(Color::White);
            float backTxtW = backText.getLocalBounds().size.x;
            backText.setPosition(sf::Vector2f{backBtnX + (backBtnW - backTxtW) / 2.f, 410.f});
            window.draw(backText);
        }

        // Apply brightness overlay
        if (brightness < 255.f) {
            RectangleShape darkenOverlay(Vector2f(W * TILE_SIZE, H * TILE_SIZE));
            darkenOverlay.setFillColor(Color(0, 0, 0, static_cast<uint8_t>(255 - brightness)));
            window.draw(darkenOverlay);
        }

        // Draw Sidebar 
        drawSidebar(window, ui, font, gScore, gLevel, gLines, nextPiece);

        window.display();
    }
    // cleanup
    delete currentPiece;
    delete nextPiece;
    return 0;
}