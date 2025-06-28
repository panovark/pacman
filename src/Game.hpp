#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "Level.hpp"
#include "Player.hpp"
#include "Ghost.hpp"
#include <vector>

class Game {
public:
    Game();
    void run();

private:
    Level            level_;
    sf::RenderWindow window_;
    Player           player_;
    std::vector<Ghost> ghosts_;

    sf::Font hudFont_;
    unsigned score_{0};
    int      lives_{3};
    bool     gameOver_{false};

    bool    levelCleared_{false};
    sf::Text clearText_;
    sf::Text gameOverText_;

    sf::Sound startSnd_;

    sf::Sound siren_;
    sf::Sound deathSnd_;
    sf::Sound gameOverSnd_;
    sf::Sound winSnd_;

    void drawHud();
};
