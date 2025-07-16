#include "Game.hpp"
#include "Constants.hpp"
#include <cmath>
#include <iostream>

#if SFML_VERSION_MAJOR >= 3
#   define TEXT_CTOR(font, str) font, str
#   define FONT_OPEN(font, file) font.openFromFile(file)
#   define SOUND_SET_LOOP(sound, v) sound.setLooping(v)
#   define RECT_W(r) r.size.x
#   define RECT_H(r) r.size.y
#else
#   define TEXT_CTOR(font, str) str, font
#   define FONT_OPEN(font, file) font.loadFromFile(file)
#   define SOUND_SET_LOOP(sound, v) sound.setLoop(v)
#   define RECT_W(r) r.width
#   define RECT_H(r) r.height
#endif

// texture singleton
static const sf::Texture& tiles()
{
    static sf::Texture* tex = nullptr;
    if (!tex)
    {
        tex = new sf::Texture;
        if (!tex->loadFromFile("resources/textures/tiles.png"))
            std::cerr << "ERROR: cannot load resources/textures/tiles.png\n";
    }
    return *tex;
}

// buffer singletons
static sf::SoundBuffer& startBuf()
{
    static sf::SoundBuffer* buf = nullptr;
    if (!buf) {
        buf = new sf::SoundBuffer;
        if (!buf->loadFromFile("resources/audio/start.wav"))
            std::cerr << "ERROR: cannot load resources/audio/start.wav\n";
    }
    return *buf;
}

static sf::SoundBuffer& sirenBuf()
{
    static sf::SoundBuffer* buf = nullptr;
    if (!buf) {
        buf = new sf::SoundBuffer;
        if (!buf->loadFromFile("resources/audio/siren.wav"))
            std::cerr << "ERROR: cannot load resources/audio/siren.wav\n";
    }
    return *buf;
}

static sf::SoundBuffer& deathBuf()
{
    static sf::SoundBuffer* buf = nullptr;
    if (!buf) {
        buf = new sf::SoundBuffer;
        if (!buf->loadFromFile("resources/audio/death.wav"))
            std::cerr << "ERROR: cannot load resources/audio/death.wav\n";
    }
    return *buf;
}

static sf::SoundBuffer& gameOverBuf()
{
    static sf::SoundBuffer* buf = nullptr;
    if (!buf) {
        buf = new sf::SoundBuffer;
        if (!buf->loadFromFile("resources/audio/gameover.wav"))
            std::cerr << "ERROR: cannot load resources/audio/gameover.wav\n";
    }
    return *buf;
}

static sf::SoundBuffer& winBuf()
{
    static sf::SoundBuffer* buf = nullptr;
    if (!buf) {
        buf = new sf::SoundBuffer;
        if (!buf->loadFromFile("resources/audio/win.wav"))
            std::cerr << "ERROR: cannot load resources/audio/win.wav\n";
    }
    return *buf;
}

// ctor
Game::Game()
: level_("resources/levels/level1.txt", tiles())
, window_(sf::VideoMode({unsigned(level_.width()  * TILE),
                         unsigned(level_.height() * TILE)}),
          "Pac-Man 3", sf::Style::Default)
, player_([this](unsigned p){ score_ += p; })
, clearText_(TEXT_CTOR(hudFont_, ""))
, gameOverText_(TEXT_CTOR(hudFont_, ""))
    , startSnd_(startBuf())
    , siren_   (sirenBuf())
    , deathSnd_(deathBuf())
    , gameOverSnd_(gameOverBuf())
    , winSnd_(winBuf())
{
    window_.setFramerateLimit(60);

    FONT_OPEN(hudFont_, "resources/fonts/PressStart2P.ttf");

    startSnd_.play();

    SOUND_SET_LOOP(siren_, true);
    siren_.setVolume(40.f);
    siren_.play();

    ghosts_.emplace_back(sf::Color::Red,
                         sf::Vector2f(TILE*(13+0.5f), TILE*(14+0.5f)));
    ghosts_.emplace_back(sf::Color::Cyan,
                         sf::Vector2f(TILE*(14+0.5f), TILE*(14+0.5f)));
    ghosts_.emplace_back(sf::Color::Magenta,
                         sf::Vector2f(TILE*(12+0.5f), TILE*(14+0.5f)));
    ghosts_.emplace_back(sf::Color(255,165,0),
                         sf::Vector2f(TILE*(15+0.5f), TILE*(14+0.5f)));

    clearText_.setCharacterSize(14);
    clearText_.setFillColor(sf::Color::Yellow);
    clearText_.setString("LEVEL CLEAR!\nPress Space");
    auto r = clearText_.getLocalBounds();
    clearText_.setOrigin({RECT_W(r) / 2.f, RECT_H(r) / 2.f});
    clearText_.setPosition({window_.getSize().x / 2.f,
                            window_.getSize().y / 2.f});

    gameOverText_.setFont(hudFont_);
    gameOverText_.setCharacterSize(14);
    gameOverText_.setFillColor(sf::Color::Yellow);
    gameOverText_.setString("GAME OVER\nPress Space");
    r = gameOverText_.getLocalBounds();
    gameOverText_.setOrigin({RECT_W(r) / 2.f, RECT_H(r) / 2.f});
    gameOverText_.setPosition({window_.getSize().x / 2.f,
                               window_.getSize().y / 2.f});
}

// HUD
void Game::drawHud()
{
    sf::Text t(TEXT_CTOR(hudFont_, ""));
    t.setCharacterSize(12);
    t.setFillColor(sf::Color::White);
    t.setString("SCORE  " + std::to_string(score_));
    t.setPosition({4.f, float(level_.height() * TILE - 14)});
    window_.draw(t);

    sf::Text l(TEXT_CTOR(hudFont_, ""));
    l.setCharacterSize(12);
    l.setFillColor(sf::Color::White);
    l.setString("LIVES " + std::to_string(lives_));
    l.setPosition({float(level_.width()*TILE - 96), float(level_.height()*TILE - 14)});
    window_.draw(l);
}

// main loop
void Game::run()
{
    while (window_.isOpen())
    {

#if SFML_VERSION_MAJOR >= 3
        while (auto evOpt = window_.pollEvent())
        {
            const sf::Event& ev = *evOpt;
            if (ev.is<sf::Event::Closed>()) {
                window_.close();
                return;
            }

            if ((levelCleared_ || gameOver_)
                && ev.is<sf::Event::KeyPressed>()
                && ev.getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Space)
            {
#else
        sf::Event ev;
        while (window_.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed) {
                window_.close();
                return;
            }

            if ((levelCleared_ || gameOver_)
                && ev.type == sf::Event::KeyPressed
                && ev.key.code == sf::Keyboard::Space)
            {
#endif
                level_.resetPellets();
                player_.reset();
                for(auto& g:ghosts_) g.reset();
                score_        = 0;
                lives_        = 3;
                levelCleared_ = false;
                gameOver_     = false;

                deathSnd_.stop();
                gameOverSnd_.stop();
                winSnd_.stop();
                startSnd_.play();
                siren_.play();
            }
        }

        if (!levelCleared_ && !gameOver_)
        {
            player_.handleInput();
            player_.update(level_);
            sf::Vector2f target = player_.position();
            for(auto& g:ghosts_) g.update(level_, target);

            for(auto& g:ghosts_){
                sf::Vector2f d = g.position() - player_.position();
                if(std::hypot(d.x,d.y) < COLL_RADIUS*2){
                    lives_--;
                    deathSnd_.play();
                    if(lives_ <= 0){
                        gameOver_ = true;
                        siren_.stop();
                        gameOverSnd_.play();
                    }
                    player_.reset();
                    for(auto& g:ghosts_) g.reset();
                    break;
                }
            }

            if (!level_.pelletsRemaining())
            {
                levelCleared_ = true;
                siren_.stop();
                winSnd_.play();
            }
        }

        window_.clear();
        level_.draw(window_);
        for(auto& g:ghosts_) g.draw(window_);
        player_.draw(window_);
        drawHud();
        if (levelCleared_) window_.draw(clearText_);
        if (gameOver_)    window_.draw(gameOverText_);
        window_.display();
    }
}