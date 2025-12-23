# Tetris Game

A modern Tetris game with advanced mechanics built with SFML 3 and C++17.

## ✨ Features

### Core Mechanics
- 🎮 Classic Tetris gameplay with modern enhancements
- 🎲 **7-Bag Random System** - Fair piece distribution
- ⏱️ **Lock Delay** - 500ms grace period with 15-move infinity
- 🔄 **DAS & ARR** - Delayed Auto Shift (133ms) and Auto Repeat Rate
- 🎯 **Ghost Piece** - Shows landing position

### Advanced Techniques
- 🌀 **T-Spin Detection** - 3-corner algorithm with bonus scoring
- 🔥 **Back-to-Back (B2B)** - 1.5x multiplier for consecutive special clears
- ✨ **Perfect Clear** - +3000 bonus for clearing entire board
- 📊 **Combo System** - Chain multiple line clears

### Visual & Audio
- 🎨 NES-style 3D tile graphics with antialiasing
- 💫 Particle effects on line clears
- 🌈 Soft drop trail animation
- 🎵 Music & SFX with volume control
- 🖥️ **Fullscreen support** (F11 toggle with aspect ratio preservation)

### UI & UX
- 📋 **2x2 Grid: Hold (top-left) + NEXT queue (J-shaped)** - 3-piece preview
- 🔲 **Hold Piece** - Store piece for later (C key)
- ⏸️ Pause menu with settings access
- 📊 Live statistics (Score, Level, Lines, Tetris count, T-Spin count, Max Combo)
- 🎯 3 Difficulty levels (Easy/Normal/Hard)
- 💾 Settings persistence (config.ini)
  - Music Volume (0-100%)
  - SFX Volume (0-100%)
  - Brightness (20-100%)
  - **DAS** (100-200ms) - Delayed Auto Shift
  - **ARR** (0-50ms) - Auto Repeat Rate
  - Ghost Piece toggle
- 🏆 High score tracking
- 📖 **How To Play screen** - Complete tutorial with game mechanics

## 🎮 Controls
| Key | Action |
|-----|--------|
| ← → | Move left/right |
| ↓ | Soft drop (+1 pt/cell) |
| ↑ | Rotate clockwise |
| Space | Hard drop (+2 pts/cell) |
| C | Hold piece |
| P / ESC | Pause |
| F11 | Toggle fullscreen |

## 🏗️ Build Instructions

### Prerequisites
- **MinGW-w64** (GCC compiler)
- **Make** (optional, for using Makefile)

**No SFML installation required!** All libraries included in `lib/` folder.

### Quick Start
```bash
git clone <repo-url>
cd Tetris_24521928_24521784_24520881_23520764_24521294
make
./Tetris.exe
```

### Build Options

#### Option 1: Using Make (Recommended)
```bash
make         # Build debug version
make release # Build optimized version
make run     # Build and run
make clean   # Clean build files
```

#### Option 2: Direct compilation
```bash
g++ -std=c++17 main.cpp src/*.cpp -o Tetris.exe -Llib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

## 📁 Project Structure
```
├── main.cpp           # Entry point, game loop, DAS/ARR, fullscreen
├── src/
│   ├── Config.h       # Game constants, enums (GameState, Difficulty)
│   ├── Piece.h/cpp    # 7-bag randomizer, piece shapes
│   ├── Game.h/cpp     # T-Spin, B2B, lock delay, settings persistence
│   ├── Audio.h/cpp    # Volume control, SFX system
│   └── UI.h/cpp       # 2-column sidebar, particles, animations, menus
├── lib/
│   ├── *.dll          # SFML runtime libraries (19 files)
│   └── *.dll.a        # SFML import libraries for linking (4 files)
├── assets/
│   ├── audio/         # Sound effects & music
│   └── fonts/         # Game font (PressStart2P.ttf)
├── Makefile           # Self-contained build (no system SFML needed)
└── README.md          # This file
```

## 🎯 Scoring System

### Basic Scoring
- Single line: **100 × Level**
- Double: **300 × Level**
- Triple: **500 × Level**
- Tetris (4 lines): **800 × Level**

### T-Spin Bonuses
- T-Spin Single: **800 × Level**
- T-Spin Double: **1200 × Level**
- T-Spin Triple: **1600 × Level**

### Special Bonuses
- **Back-to-Back**: 1.5× multiplier for consecutive Tetris/T-Spin
- **Perfect Clear**: +3000 points
- **Combo**: Bonus for consecutive line clears
- **Soft Drop**: +1 point per cell
- **Hard Drop**: +2 points per cell

## 🔧 Technical Details

- **Language**: C++17
- **Graphics**: SFML 3.0
- **Antialiasing**: Level 8 for smooth rendering
- **Window**: 800×800 with fullscreen support
- **Architecture**: Modular design with separate Game, UI, Audio systems
- **Settings**: Persistent config.ini
  - Volume controls (Music, SFX)
  - Display brightness (51-255 internal, 20-100% display)
  - Input timing (DAS: 100-200ms, ARR: 0-50ms)
  - Visual toggles (Ghost Piece)
- **Code Style**: Uniform commenting for all source files with GPL v3 headers

## 📦 Distribution

The game is **fully portable**:
- All dependencies included in `lib/` folder
- No installation required
- Just copy the folder and run `Tetris.exe`

**Folder contents for distribution:**
```
Tetris/
├── Tetris.exe
├── lib/           (19 DLL files)
├── assets/        (audio + fonts)
├── config.ini     (auto-generated)
└── highscore.dat  (auto-generated)
```

## 🤝 Development

To modify the game:
1. Clone the repository
2. Edit source files in `src/`
3. Run `make` to rebuild
4. Test with `./Tetris.exe`

All SFML libraries are self-contained in `lib/`, so no system-wide SFML installation is needed for building or running.

## 📝 License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.
