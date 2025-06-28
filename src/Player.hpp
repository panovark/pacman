#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <functional>
#include <vector>

#include "Constants.hpp"
#include "Level.hpp"

class Player {
public:
    using ScoreFn = std::function<void(unsigned)>;

    explicit Player(ScoreFn);
    void handleInput();
    void update(Level&);
    void draw(sf::RenderTarget&) const;
    void reset();
    sf::Vector2f position() const { return sprite_.getPosition(); }

private:
    enum class Dir { None, Left, Right, Up, Down };
    sf::Vector2f dirVec(Dir) const;
    bool canMove(const Level&, const sf::Vector2f&) const;

    // sprite and moving
    sf::CircleShape sprite_;
    Dir  curDir_{Dir::None};
    Dir  nextDir_{Dir::None};
    Dir  lastDir_{Dir::Right};
    float mouthPhase_{0.f};

    // munch1/munch2 sound
    static const sf::SoundBuffer& munchBuf(int id);
    int  nextId_{0};
    static constexpr int SLOTS = 8;
    std::vector<sf::Sound> pool_;

    // score
    ScoreFn addScore_;
    bool onTeleport_ = false;
};
