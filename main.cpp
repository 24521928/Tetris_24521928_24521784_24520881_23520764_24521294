#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <ctime>
#include <algorithm>
#include <optional>

using namespace std;
using namespace sf;

// --- GAME CONFIGURATION ---
const int H = 20;
const int W = 15;
const int TILE_SIZE = 30;

// --- GLOBAL VARIABLES ---
char board[H][W] = {};
int x = 4, y = 0;
float gameDelay = 0.5f;

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

void SpeedIncrement() {
    if (gameDelay > 0.1f) {
        gameDelay -= 0.03f;
    }
}

void removeLine() {
    for (int i = H - 2; i > 0; i--) {
        bool isFull = true;
        for (int j = 1; j < W - 1; j++) {
            if (board[i][j] == ' ') {
                isFull = false;
                break;
            }
        }

        if (isFull) {
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
}

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

// --- MAIN FUNCTION ---

int main() {
    RenderWindow window(VideoMode(Vector2u(W * TILE_SIZE, H * TILE_SIZE)), "SS008 - Tetris");
    window.setFramerateLimit(60);

    // Load Audio
    if (!bgMusic.openFromFile("loop_theme.wav")) return -1;
    if (!clearBuffer.loadFromFile("line_clear.wav")) return -1;
    if (!landBuffer.loadFromFile("bumper_end.wav")) return -1;

    bgMusic.setLooping(true);
    bgMusic.setVolume(50);
    bgMusic.play();

    // Init Game
    srand(time(0));
    currentPiece = createRandomPiece();
    initBoard();

    Clock clock;
    float timer = 0;
    float inputTimer = 0;
    const float inputDelay = 0.1f;

    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;
        inputTimer += time;

        // --- EVENTS ---
        while (const auto event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<Event::KeyPressed>()) {
                // Rotate
                if (keyPressed->code == Keyboard::Key::W) {
                    currentPiece->rotate(x, y);
                }
                // Hard Drop
                else if (keyPressed->code == Keyboard::Key::Space) {
                    while (canMove(0, 1)) {
                        y++;
                    }
                    timer = gameDelay + 10.0f; // Force lock immediately
                }
            }
        }

        // --- INPUT (Continuous) ---
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
            if (canMove(0, 1)) {
                y++;
            } else {
                landSound.play();

                block2Board();
                removeLine();

                delete currentPiece;
                currentPiece = createRandomPiece();
                x = 4; 
                y = 0;

                // Game Over Check
                if (!canMove(0, 0)) {
                    window.close();
                }
            }
            timer = 0;
        }

        // --- RENDER ---
        window.clear(Color::Black);

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

        window.display();
    }

    return 0;
}