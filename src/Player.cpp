#include "Player.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

// two buffers cache
const sf::SoundBuffer& Player::munchBuf(int id)
{
    static sf::SoundBuffer buf[2];
    static bool ok[2]{false,false};
    const char* file = id ? "resources/audio/munch2.wav"
                          : "resources/audio/munch1.wav";
    if (!ok[id]) {
        if (!buf[id].loadFromFile(file))
            std::cerr << "ERROR: cannot load " << file << '\n';
        ok[id] = true;
    }
    return buf[id];
}

// constructor
Player::Player(ScoreFn cb) : addScore_(std::move(cb))
{
    sprite_.setRadius(PAC_RADIUS);
    sprite_.setFillColor(sf::Color::Yellow);
    sprite_.setOrigin({PAC_RADIUS, PAC_RADIUS});

    pool_.reserve(SLOTS);
    for (int i = 0; i < SLOTS; ++i) {
        pool_.emplace_back(munchBuf(0));
        pool_.back().setVolume(90.f);
    }
    reset();
}

// reset
void Player::reset()
{
    sprite_.setPosition({TILE*(12+0.5f), TILE*(23+0.5f)});
    curDir_ = nextDir_ = Dir::None;
    lastDir_ = Dir::Right;
    mouthPhase_ = 0.f;
    nextId_ = 0;
    onTeleport_ = false;
}

// input
void Player::handleInput()
{
    using K = sf::Keyboard::Key;
    if (sf::Keyboard::isKeyPressed(K::Left))  nextDir_ = Dir::Left;
    if (sf::Keyboard::isKeyPressed(K::Right)) nextDir_ = Dir::Right;
    if (sf::Keyboard::isKeyPressed(K::Up))    nextDir_ = Dir::Up;
    if (sf::Keyboard::isKeyPressed(K::Down))  nextDir_ = Dir::Down;
}

// additional
sf::Vector2f Player::dirVec(Dir d) const
{
    switch (d) {
        case Dir::Left:  return {-SPEED_PX,0};
        case Dir::Right: return { SPEED_PX,0};
        case Dir::Up:    return {0,-SPEED_PX};
        case Dir::Down:  return {0, SPEED_PX};
        default:         return {0,0};
    }
}

bool Player::canMove(const Level& lvl, const sf::Vector2f& p) const
{
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

// update
void Player::update(Level& lvl)
{
    mouthPhase_ += 0.15f;
    if (mouthPhase_ > 2.f * PI) mouthPhase_ -= 2.f * PI;

    auto pos=sprite_.getPosition();
    int gx=int(pos.x/TILE), gy=int(pos.y/TILE);
    sf::Vector2f center{TILE*(gx+0.5f),TILE*(gy+0.5f)};

    // pellet + sound
    if(lvl.hasPellet(gx,gy)){
        lvl.eatPellet(gx,gy);
        addScore_(10);

        // free or the old slot
        auto it=std::find_if(pool_.begin(),pool_.end(),[](auto& s){
            return s.getStatus()!=sf::SoundSource::Status::Playing;});
        if(it==pool_.end()) it=pool_.begin();     // recycle

        it->setBuffer(munchBuf(nextId_));
        nextId_^=1;
        it->stop(); it->play();
        std::rotate(pool_.begin(),it, it+1);
    }

    // rotating
    constexpr float tol=TILE*0.2f;
    if(nextDir_!=curDir_){
        sf::Vector2f toC=center-pos;
        if(std::abs(toC.x)<tol && std::abs(toC.y)<tol &&
           canMove(lvl,pos+dirVec(nextDir_))){
            sprite_.setPosition(center);
            curDir_=nextDir_;
        }
    }

    // moving
    sf::Vector2f step=dirVec(curDir_);
    if(canMove(lvl,pos+step)) sprite_.move(step);
    else curDir_=Dir::None;
    if(curDir_!=Dir::None) lastDir_=curDir_;

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

// draw
void Player::draw(sf::RenderTarget& rt) const
{
    rt.draw(sprite_);

    float deg=std::abs(std::sin(mouthPhase_))*40.f;
    if(deg<2.f) return;

    float dirDeg=0.f;
    switch(lastDir_){
        case Dir::Up: dirDeg=-90.f; break;
        case Dir::Down: dirDeg=90.f; break;
        case Dir::Left: dirDeg=180.f; break;
        default: break;
    }

    constexpr int seg=24;
    sf::ConvexShape mouth(seg+2);
    mouth.setFillColor(sf::Color::Black);
    sf::Vector2f c=sprite_.getPosition();
    mouth.setPoint(0,c);

    float start = (dirDeg - deg) * PI / 180.f;
    float end   = (dirDeg + deg) * PI / 180.f;
    float step=(end-start)/seg;
    float r=PAC_RADIUS+1.f;

    for(int i=0;i<=seg;++i){
        float a=start+i*step;
        mouth.setPoint(i+1,{c.x+std::cos(a)*r,c.y+std::sin(a)*r});
    }
    rt.draw(mouth);
}
