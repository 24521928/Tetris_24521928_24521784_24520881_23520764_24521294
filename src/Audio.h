/*
 * Tetris Game - Audio system
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#pragma once
#include <SFML/Audio.hpp>

namespace Audio {
    bool init();

    void cleanup();

    void playClear();
    void playLand();


// Game functions
    void playGameOver();
    void playSettingClick();

    void playStartGame();
    void playLevelUp();
    void playOpenSettings();
    void playCloseSettings();
    void playToggleOn();
    void playToggleOff();



// Audio functions
    void playMusic();
    void stopMusic();
    void playTheme();
    void stopTheme();

    void setMusicVolume(float volume);
    void setSfxVolume(float volume);

    sf::Music& getMusic();
}
