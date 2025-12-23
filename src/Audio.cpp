#include "Audio.h"
#include "Game.h"

namespace Audio {
    // Original buffers
    static sf::SoundBuffer clearBuffer;
    static sf::SoundBuffer landBuffer;
    static sf::SoundBuffer gameOverBuffer;
    static sf::SoundBuffer settingClickBuffer;
    
    // New SFX buffers
    static sf::SoundBuffer startGameBuffer;
    static sf::SoundBuffer levelUpBuffer;
    static sf::SoundBuffer openSettingsBuffer;
    static sf::SoundBuffer closeSettingsBuffer;
    static sf::SoundBuffer toggleOnBuffer;
    static sf::SoundBuffer toggleOffBuffer;
    
    // Original sounds
    static sf::Sound* clearSound = nullptr;
    static sf::Sound* landSound = nullptr;
    static sf::Sound* gameOverSound = nullptr;
    static sf::Sound* settingClickSound = nullptr;
    
    // New sounds
    static sf::Sound* startGameSound = nullptr;
    static sf::Sound* levelUpSound = nullptr;
    static sf::Sound* openSettingsSound = nullptr;
    static sf::Sound* closeSettingsSound = nullptr;
    static sf::Sound* toggleOnSound = nullptr;
    static sf::Sound* toggleOffSound = nullptr;
    
    static sf::Music bgMusic;

    bool init() {
        // Load original sounds
        if (!bgMusic.openFromFile("assets/audio/loop_theme.ogg")) return false;
        if (!clearBuffer.loadFromFile("assets/audio/line_clear.ogg")) return false;
        if (!landBuffer.loadFromFile("assets/audio/bumper_end.ogg")) return false;
        if (!gameOverBuffer.loadFromFile("assets/audio/game_over.ogg")) return false;
        if (!settingClickBuffer.loadFromFile("assets/audio/insetting_click.ogg")) return false;
        
        // Load new SFX (optional - game works without these)
        (void)startGameBuffer.loadFromFile("assets/audio/start_game.ogg");
        (void)levelUpBuffer.loadFromFile("assets/audio/level_up.ogg");
        (void)openSettingsBuffer.loadFromFile("assets/audio/open_settings.ogg");
        (void)closeSettingsBuffer.loadFromFile("assets/audio/close_settings.ogg");
        (void)toggleOnBuffer.loadFromFile("assets/audio/toggle_on.ogg");
        (void)toggleOffBuffer.loadFromFile("assets/audio/toggle_off.ogg");

        // Create original sounds
        clearSound = new sf::Sound(clearBuffer);
        landSound = new sf::Sound(landBuffer);
        gameOverSound = new sf::Sound(gameOverBuffer);
        settingClickSound = new sf::Sound(settingClickBuffer);
        
        // Create new sounds
        startGameSound = new sf::Sound(startGameBuffer);
        levelUpSound = new sf::Sound(levelUpBuffer);
        openSettingsSound = new sf::Sound(openSettingsBuffer);
        closeSettingsSound = new sf::Sound(closeSettingsBuffer);
        toggleOnSound = new sf::Sound(toggleOnBuffer);
        toggleOffSound = new sf::Sound(toggleOffBuffer);

        bgMusic.setLooping(true);
        bgMusic.setVolume(musicVolume);
        
        return true;
    }

    void cleanup() {
        delete clearSound;
        delete landSound;
        delete gameOverSound;
        delete settingClickSound;
        delete startGameSound;
        delete levelUpSound;
        delete openSettingsSound;
        delete closeSettingsSound;
        delete toggleOnSound;
        delete toggleOffSound;
    }

    void playClear() {
        if (clearSound) clearSound->play();
    }

    void playLand() {
        if (landSound) landSound->play();
    }

    void playGameOver() {
        if (gameOverSound) gameOverSound->play();
    }

    void playSettingClick() {
        if (settingClickSound) settingClickSound->play();
    }
    
    void playStartGame() {
        if (startGameSound) startGameSound->play();
    }
    
    void playLevelUp() {
        if (levelUpSound) levelUpSound->play();
    }
    
    void playOpenSettings() {
        if (openSettingsSound) openSettingsSound->play();
    }
    
    void playCloseSettings() {
        if (closeSettingsSound) closeSettingsSound->play();
    }
    
    void playToggleOn() {
        if (toggleOnSound) toggleOnSound->play();
    }
    
    void playToggleOff() {
        if (toggleOffSound) toggleOffSound->play();
    }

    void playMusic() {
        bgMusic.play();
    }

    void stopMusic() {
        bgMusic.stop();
    }

    void setMusicVolume(float volume) {
        bgMusic.setVolume(volume);
    }

    void setSfxVolume(float volume) {
        if (clearSound) clearSound->setVolume(volume);
        if (landSound) landSound->setVolume(volume);
        if (gameOverSound) gameOverSound->setVolume(volume);
        if (settingClickSound) settingClickSound->setVolume(volume);
        if (startGameSound) startGameSound->setVolume(volume);
        if (levelUpSound) levelUpSound->setVolume(volume);
        if (openSettingsSound) openSettingsSound->setVolume(volume);
        if (closeSettingsSound) closeSettingsSound->setVolume(volume);
        if (toggleOnSound) toggleOnSound->setVolume(volume);
        if (toggleOffSound) toggleOffSound->setVolume(volume);
    }

    sf::Music& getMusic() {
        return bgMusic;
    }
}
