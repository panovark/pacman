#include "Ghost.hpp"
#include <array>
#include <algorithm>
#include <cmath>

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

void Ghost::update(const Level& lvl){
    auto pos = sprite_.getPosition();
    int gx=int(pos.x/TILE), gy=int(pos.y/TILE);
    sf::Vector2f center{TILE*(gx+0.5f),TILE*(gy+0.5f)};

    sf::Vector2f next = pos + dirVec(curDir_);
    bool atCenter = std::abs(center.x-pos.x)<1.f && std::abs(center.y-pos.y)<1.f;
    if(!canMove(lvl,next) || atCenter){
        static std::mt19937 rng(std::random_device{}());
        std::array<Dir,4> d{Dir::Left,Dir::Right,Dir::Up,Dir::Down};
        std::shuffle(d.begin(), d.end(), rng);
        for(auto nd:d){
            if(nd==opposite(curDir_)) continue;
            if(canMove(lvl,center+dirVec(nd))){ curDir_=nd; break; }
        }
    }

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

