# Tetris Game

A classic Tetris game built with SFML 3 and C++17.

## Features
- 🎮 Classic Tetris gameplay
- 🎨 NES-style 3D tile graphics
- 📊 Piece statistics panel
- 🔄 Hold piece (C key)
- ⏸️ Pause game (P/ESC key)
- 🏆 High score tracking
- 🎯 Combo system
- 📈 3 difficulty levels (Easy/Normal/Hard)
- 🎵 Music & SFX with volume control
- 👻 Ghost piece preview

## Controls
| Key | Action |
|-----|--------|
| ← → | Move left/right |
| ↓ | Soft drop (+1 pt/cell) |
| ↑ | Rotate |
| Space | Hard drop (+2 pts/cell) |
| C | Hold piece |
| P / ESC | Pause |

## Build Instructions

### Prerequisites (MSYS2)
```bash
pacman -S mingw-w64-x86_64-sfml
```

### Option 1: Using Make
```bash
make        # Build debug
make release # Build optimized
make run    # Build and run
make clean  # Clean build files
```

### Option 2: Using CMake
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

### Option 3: Direct compilation
```bash
g++ -std=c++17 main.cpp src/Piece.cpp src/Game.cpp src/Audio.cpp src/UI.cpp -o Tetris.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

## Project Structure
```
├── main.cpp           # Entry point & game loop
├── src/
│   ├── Config.h       # Game constants & enums
│   ├── Piece.h/cpp    # Tetromino classes
│   ├── Game.h/cpp     # Game logic & state
│   ├── Audio.h/cpp    # Sound system
│   └── UI.h/cpp       # UI rendering
├── assets/
│   ├── audio/         # Sound effects & music
│   └── fonts/         # Game font
├── CMakeLists.txt     # CMake build config
└── Makefile           # Make build config
```
