#include "Ghost.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <vector>

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

void Ghost::update(const Level& lvl, const sf::Vector2f& target){
    auto pos = sprite_.getPosition();
    int gx=int(pos.x/TILE), gy=int(pos.y/TILE);
    sf::Vector2f center{TILE*(gx+0.5f),TILE*(gy+0.5f)};

    sf::Vector2f next = pos + dirVec(curDir_);
    bool atCenter = std::abs(center.x-pos.x)<1.f && std::abs(center.y-pos.y)<1.f;
    if(!canMove(lvl,next) || atCenter){
        static std::mt19937 rng(std::random_device{}());
        std::array<Dir,4> order{Dir::Left,Dir::Right,Dir::Up,Dir::Down};
        std::shuffle(order.begin(), order.end(), rng);

        int pgx = int(target.x / TILE);
        int pgy = int(target.y / TILE);

        // BFS from player to compute distances
        std::vector<std::vector<int>> dist(lvl.height(), std::vector<int>(lvl.width(), -1));
        std::queue<sf::Vector2i> q;
        dist[pgy][pgx] = 0;
        q.push({pgx,pgy});
        const int dx[4]={-1,1,0,0};
        const int dy[4]={0,0,-1,1};
        while(!q.empty()){
            auto c=q.front();q.pop();
            int cd = dist[c.y][c.x];
            for(int i=0;i<4;++i){
                int nx=c.x+dx[i], ny=c.y+dy[i];
                if(nx<0||ny<0||nx>=lvl.width()||ny>=lvl.height()) continue;
                if(!lvl.isWalkable(nx,ny)) continue;
                if(dist[ny][nx]!=-1) continue;
                dist[ny][nx]=cd+1;
                q.push({nx,ny});
            }
        }

        Dir bestDir = curDir_;
        int bestCost = std::numeric_limits<int>::max();
        for(auto nd:order){
            if(nd==opposite(curDir_)) continue;
            if(!canMove(lvl,center+dirVec(nd))) continue;

            int dxs=0,dys=0;
            switch(nd){
                case Dir::Left: dxs=-1; break;
                case Dir::Right: dxs=1; break;
                case Dir::Up: dys=-1; break;
                case Dir::Down: dys=1; break;
            }
            int nx=gx+dxs, ny=gy+dys;
            int cost = (nx>=0&&ny>=0&&ny<lvl.height()&&nx<lvl.width()) ? dist[ny][nx] : -1;
            if(cost>=0 && cost < bestCost){
                bestCost = cost;
                bestDir = nd;
            }
        }

        std::uniform_real_distribution<float> prob(0.f,1.f);
        if(prob(rng) < 0.3f || bestCost==std::numeric_limits<int>::max()){
            for(auto nd:order){
                if(nd==opposite(curDir_)) continue;
                if(canMove(lvl,center+dirVec(nd))){
                    bestDir = nd;
                    break;
                }
            }
        }

        curDir_ = bestDir;
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
