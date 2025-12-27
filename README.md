# SS008 - Tetris Game

A Tetris game developed in C++ using SFML.

## Features

- **7-Bag Shuffle**: Fair distribution of tetromino pieces
- **Pause/Resume**: Pause the game with P or Esc key
- **Settings Menu**: Adjust music/SFX volume, screen brightness, and ghost piece visibility
- **Ghost Piece**: Preview of where the piece will land
- **Score & Level**: Track your score and current level
- **Wall Kick**: Rotate pieces near walls
- **Line Clear**: Complete rows to earn points

## Controls

- **A/D**: Move left/right
- **W**: Rotate piece
- **Space**: Hard drop (instant fall to bottom)
- **P/Esc**: Pause game
- **Enter**: Start game (from main menu)

## Requirements

- Compiler: GCC (MinGW64 or similar)
- SFML 3.0 or higher

## Installation and Build

If using MingW64:
```bash
pacman -S mingw-w64-x86_64-sfml
```

Compile:
```bash
g++ main.cpp -o tetris.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
```

Run:
```bash
./tetris.exe
```

## Game Installation through Google Drive
[Link drive](https://drive.google.com/file/d/1soyxjdsicefQ4ZKw-8ln_HtNcpxS90Hm/view?usp=sharing!)
## License

MIT License - see LICENSE file
