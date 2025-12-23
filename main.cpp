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

// --- AUDIO RESOURCES ---
sf::SoundBuffer clearBuffer;
sf::Sound clearSound(clearBuffer);

sf::SoundBuffer landBuffer;
sf::Sound landSound(landBuffer);

sf::Music bgMusic;

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
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                temp[j][3 - i] = shape[i][j];
            }
        }
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (temp[i][j] != ' ') {
                    int tx = currentX + j;
                    int ty = currentY + i;
                    if (tx < 1 || tx >= W - 1 || ty >= H - 1) return;
                    if (board[ty][tx] != ' ') return;
                }
            }
        }
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
            if ((i == H - 1) || (j == 0) || (j == W - 1)) board[i][j] = '#';
            else board[i][j] = ' ';
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
            if (board[i][j] == ' ') { isFull = false; break; }
        }
        if (isFull) {
            cleared++;
            clearSound.play();
            for (int k = i; k > 0; k--) {
                for (int j = 1; j < W - 1; j++) {
                    if (k != 1) board[k][j] = board[k - 1][j];
                    else board[k][j] = ' ';
                }
            }
            i++; 
            SpeedIncrement();
        }
    }
    return cleared;
}

// --- MAIN FUNCTION ---
int main() {
    RenderWindow window(VideoMode(Vector2u(PLAY_W_PX + SIDEBAR_W, PLAY_H_PX)), "SS008 - Tetris");
    window.setFramerateLimit(60);
    SidebarUI ui = makeSidebarUI();
    // add font
    sf::Font font;
    if (!font.openFromFile("assets/fonts/DejaVuSans.ttf")) {
        return -1;
    }
    // Load Audio
    if (!bgMusic.openFromFile("loop_theme.wav")) return -1;
    if (!clearBuffer.loadFromFile("line_clear.wav")) return -1;
    if (!landBuffer.loadFromFile("bumper_end.wav")) return -1;

    bgMusic.setLooping(true);
    bgMusic.setVolume(50);
    bgMusic.play();

    // Init Game
    srand((unsigned)time(0));
    currentPiece = createRandomPiece();
    Piece* nextPiece = createRandomPiece(); // next piece to preview on side bar
    initBoard();

    Clock clock;
    float timer = 0, inputTimer = 0;
    const float inputDelay = 0.1f;

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        if (!isGameOver) {
            timer += time;
            inputTimer += time;
        }

        // --- EVENTS ---
        while (const auto event = window.pollEvent()) {
            if (event->is<Event::Closed>()) window.close();

            // Logic Click chuột khi Game Over
            if (isGameOver && event->is<Event::MouseButtonPressed>()) {
                Vector2i m = Mouse::getPosition(window);
                if (m.x > 125 && m.x < 325 && m.y > 250 && m.y < 300) restartGame();
                if (m.x > 125 && m.x < 325 && m.y > 330 && m.y < 380) window.close();
            }

            if (!isGameOver) {
                if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                    if (keyPressed->code == Keyboard::Key::W) currentPiece->rotate(x, y);
                    else if (keyPressed->code == Keyboard::Key::Space) {
                        while (canMove(0, 1)) y++;
                        timer = gameDelay + 10.0f; // force lock
                    }
                }
            }
        }

        // --- INPUT (Continuous) ---
        if (!isGameOver) {
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

        // Draw Sidebar 
        drawSidebar(window, ui, font, gScore, gLevel, gLines, nextPiece);
        window.display();
    }
    // cleanup
    delete currentPiece;
    delete nextPiece;
    return 0;
}
