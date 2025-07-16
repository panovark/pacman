#include "Ghost.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>

Ghost::Ghost(sf::Color color, sf::Vector2f start) : start_(start) {
    sprite_.setRadius(PAC_RADIUS);
    sprite_.setOrigin({PAC_RADIUS, PAC_RADIUS});
    sprite_.setFillColor(color);
    sprite_.setPosition(start_);
}

void Ghost::reset() {
    sprite_.setPosition(start_);
    curDir_ = Dir::Left;
    onTeleport_ = false;
    scatterSteps_ = 40;
}

sf::Vector2f Ghost::dirVec(Dir d) const {
    switch(d){
        case Dir::Left:  return {-SPEED_PX,0};
        case Dir::Right: return { SPEED_PX,0};
        case Dir::Up:    return {0,-SPEED_PX};
        default:         return {0, SPEED_PX};
    }
}

bool Ghost::canMove(const Level& lvl, const sf::Vector2f& p) const {
    const float d = COLL_RADIUS*0.70710678f;
    const sf::Vector2f v[8]={
        {p.x-COLL_RADIUS,p.y},{p.x+COLL_RADIUS,p.y},
        {p.x,p.y-COLL_RADIUS},{p.x,p.y+COLL_RADIUS},
        {p.x-d,p.y-d},{p.x+d,p.y-d},
        {p.x+d,p.y+d},{p.x-d,p.y+d}};
    for(auto q:v)
        if(!lvl.isWalkable(int(q.x/TILE),int(q.y/TILE))) return false;
    return true;
}

Ghost::Dir Ghost::opposite(Dir d){
    switch(d){
        case Dir::Left: return Dir::Right;
        case Dir::Right: return Dir::Left;
        case Dir::Up: return Dir::Down;
        default: return Dir::Up;
    }
}

void Ghost::update(const Level& lvl, const sf::Vector2f& target){
    auto pos = sprite_.getPosition();
    int gx=int(pos.x/TILE), gy=int(pos.y/TILE);
    sf::Vector2f center{TILE*(gx+0.5f),TILE*(gy+0.5f)};

    sf::Vector2f next = pos + dirVec(curDir_);
    bool atCenter = std::abs(center.x-pos.x)<1.f && std::abs(center.y-pos.y)<1.f;
    if(!canMove(lvl,next) || atCenter){
        static std::mt19937 rng(std::random_device{}());
        std::array<Dir,4> dirs{Dir::Left,Dir::Right,Dir::Up,Dir::Down};
        std::shuffle(dirs.begin(), dirs.end(), rng);

        int pgx = int(target.x / TILE);
        int pgy = int(target.y / TILE);

        bool chase = scatterSteps_ <= 0 && std::uniform_real_distribution<float>(0.f,1.f)(rng) < 0.6f;
        if(scatterSteps_ > 0) --scatterSteps_;

        Dir bestDir = curDir_;
        float bestDist = std::numeric_limits<float>::max();

        for(auto nd:dirs){
            if(nd == opposite(curDir_)) continue;
            if(!canMove(lvl, center + dirVec(nd))) continue;

            if(!chase){
                bestDir = nd;
                break;
            }

            int dx=0, dy=0;
            switch(nd){
                case Dir::Left: dx=-1; break;
                case Dir::Right: dx=1; break;
                case Dir::Up: dy=-1; break;
                case Dir::Down: dy=1; break;
            }
            float dist = std::abs((gx+dx)-pgx) + std::abs((gy+dy)-pgy);
            if(dist < bestDist){
                bestDist = dist;
                bestDir = nd;
            }
        }

        curDir_ = bestDir;
    }

    constexpr float tol = TILE * 0.2f;
    if(std::abs(center.x-pos.x)<tol && std::abs(center.y-pos.y)<tol)
        sprite_.setPosition(center);

    sprite_.move(dirVec(curDir_));
    pos = sprite_.getPosition();
    int w = lvl.width()*TILE;
    if(pos.x < 0) pos.x += w; else if(pos.x > w) pos.x -= w;
    sprite_.setPosition(pos);

    gx=int(pos.x/TILE); gy=int(pos.y/TILE);
    bool tp = lvl.isTeleport(gx,gy);
    if(tp && !onTeleport_){
        sprite_.setPosition(lvl.teleportDestination(gx,gy));
        pos = sprite_.getPosition();
        onTeleport_ = true;
    } else if(!tp){
        onTeleport_ = false;
    }
}

