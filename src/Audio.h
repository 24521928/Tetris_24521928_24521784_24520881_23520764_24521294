/*
 * Tetris Game - Audio system
 * Copyright (C) 2025 Tetris Game Contributors
 * Licensed under GPL v3 - see LICENSE file
 */

#pragma once
#include <SFML/Audio.hpp>

namespace Audio {
    // Initialize audio system
    bool init();
    
    // Cleanup
    void cleanup();
    
    // Play sounds - Original
    void playClear();
    void playLand();
    void playGameOver();
    void playSettingClick();
    
    // Play sounds - New SFX
    void playStartGame();      // Nhấn START
    void playLevelUp();        // Lên level
    void playOpenSettings();   // Mở Settings
    void playCloseSettings();  // Đóng Settings
    void playToggleOn();       // Bật Ghost Piece
    void playToggleOff();      // Tắt Ghost Piece
    
    // Music control
    void playMusic();
    void stopMusic();
    
    // Volume control
    void setMusicVolume(float volume);
    void setSfxVolume(float volume);
    
    // Access to music for volume changes
    sf::Music& getMusic();
}
