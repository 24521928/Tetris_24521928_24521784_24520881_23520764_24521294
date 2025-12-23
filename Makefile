# Tetris Makefile for MinGW/MSYS2
# Self-contained build - no system SFML installation required!
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g
LDFLAGS = -Llib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Source files
SOURCES = main.cpp src/Piece.cpp src/Game.cpp src/Audio.cpp src/UI.cpp
TARGET = Tetris.exe

# Default target
all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

# Release build (optimized)
release: CXXFLAGS = -std=c++17 -O2 -DNDEBUG
release: LDFLAGS = -Llib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
release: $(TARGET)

# Clean build files
clean:
	rm -f $(TARGET)

# Run the game
run: $(TARGET)
	./$(TARGET)

.PHONY: all release clean run
