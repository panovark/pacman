#pragma once
#include <SFML/Graphics.hpp>
#include <random>
#include "Level.hpp"
#include "Constants.hpp"

class Ghost {
public:
    Ghost(sf::Color color, sf::Vector2f start);
    void reset();
    void update(const Level& lvl);
    void draw(sf::RenderTarget& rt) const { rt.draw(sprite_); }
    sf::Vector2f position() const { return sprite_.getPosition(); }
private:
    enum class Dir {Left,Right,Up,Down};
    sf::Vector2f dirVec(Dir d) const;
    bool canMove(const Level& lvl, const sf::Vector2f& pos) const;
    static Dir opposite(Dir d);

    sf::CircleShape sprite_;
    sf::Vector2f start_;
    Dir curDir_{Dir::Left};
    bool onTeleport_ = false;
};

