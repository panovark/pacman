#include "Level.hpp"
#include <fstream>
#include <algorithm>

Level::Level(const std::string& txtFile, const sf::Texture& tileset)
: tiles_(tileset)
{
    std::ifstream in(txtFile);
    std::string line;
    while (std::getline(in, line))
        grid_.push_back(line);

    pellets_ = grid_;

    for(int y=0;y<(int)grid_.size();++y){
        int rowW = grid_[y].size()-1;
        if(grid_[y][0] != '#' && grid_[y][rowW] != '#'){
            teleports_.push_back({0,y});
            teleports_.push_back({rowW,y});
        }
    }

    const int w = grid_[0].size();
    const int h = grid_.size();

    vertices_.setPrimitiveType(sf::PrimitiveType::Triangles);
    vertices_.resize(w * h * 6);

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            if (grid_[y][x] != '#') continue;
            sf::Vertex* v = &vertices_[(y * w + x) * 6];

            float L =  x      * TILE, R = (x + 1) * TILE;
            float T =  y      * TILE, B = (y + 1) * TILE;

            const sf::Vector2f tl{0,0}, tr{16,0}, bl{0,16}, br{16,16};
            v[0].position={L,T}; v[0].texCoords=tl;
            v[1].position={R,T}; v[1].texCoords=tr;
            v[2].position={R,B}; v[2].texCoords=br;
            v[3].position={L,T}; v[3].texCoords=tl;
            v[4].position={R,B}; v[4].texCoords=br;
            v[5].position={L,B}; v[5].texCoords=bl;
        }
}

bool Level::isWalkable(int gx,int gy) const
{
    if (gy<0||gy>=height()||gx<0||gx>=width()) return false;
    return grid_[gy][gx] != '#';
}

void Level::draw(sf::RenderTarget& rt) const
{
    rt.draw(vertices_, &tiles_);

    sf::CircleShape dot(TILE * 0.15f);
    dot.setOrigin({dot.getRadius(), dot.getRadius()});
    dot.setFillColor({255,200,200});

    for (int y=0;y<height();++y)
        for (int x=0;x<width();++x)
            if (pellets_[y][x] == '.')
            {
                dot.setPosition({TILE*(x+0.5f), TILE*(y+0.5f)});
                rt.draw(dot);
            }
}

bool Level::pelletsRemaining() const
{
    for (auto& row : pellets_)
        for (char c : row)
            if (c == '.') return true;
    return false;
}

void Level::resetPellets()
{
    pellets_ = grid_;
}

bool Level::isTeleport(int gx,int gy) const
{
    return std::find(teleports_.begin(), teleports_.end(), sf::Vector2i{gx,gy}) != teleports_.end();
}

sf::Vector2f Level::teleportDestination(int gx,int gy) const
{
    for(auto t: teleports_)
        if(t.y==gy && t.x!=gx)
            return {TILE*(t.x+0.5f), TILE*(t.y+0.5f)};
    return {TILE*(gx+0.5f), TILE*(gy+0.5f)};
}
