#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Constants.hpp"

class Level {
public:
    Level(const std::string& txtFile, const sf::Texture& tileset);

    bool isWalkable(int gx, int gy) const;

    // pellet API
    bool hasPellet(int gx,int gy) const { return pellets_[gy][gx] == '.'; }
    void eatPellet(int gx,int gy)       { pellets_[gy][gx] = ' '; }

    bool pelletsRemaining() const;
    void resetPellets();

    // teleport API
    bool isTeleport(int gx,int gy) const;
    sf::Vector2f teleportDestination(int gx,int gy) const;

    void draw(sf::RenderTarget& rt) const;
    int  width()  const { return static_cast<int>(grid_[0].size()); }
    int  height() const { return static_cast<int>(grid_.size());    }

private:
    std::vector<std::string> grid_;
    std::vector<std::string> pellets_;
    sf::VertexArray vertices_;
    const sf::Texture& tiles_;
    std::vector<sf::Vector2i> teleports_;

    static constexpr int TILE = 16;

};
