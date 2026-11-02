#include "Player.h"
#include "Room.h"
#include "../engine/Engine.h"
#include "../input/VirtualGamepad.h"
#include <cmath>
#include <algorithm>

Player::Player() = default;
Player::~Player() = default;

void Player::init(Engine* engine) {
    // Load player sprite sheet from assets
    // Expected: 6 rows × 8 cols of 32×48 frames
    if (!m_sprite.load("sprites/player.png", 32, 48)) {
        // Fallback: no sprite (will render hitbox placeholder in debug)
    }

    // Define animations (frame index = row * cols + col, 8 cols per row)
    m_sprite.addAnimation({"idle",       0,  4,  8.0f,  true});
    m_sprite.addAnimation({"walk",       8,  8, 12.0f,  true});
    m_sprite.addAnimation({"run",       16,  8, 16.0f,  true});
    m_sprite.addAnimation({"jump",      24,  2,  8.0f, false});
    m_sprite.addAnimation({"fall",      26,  2,  8.0f, false});
    m_sprite.addAnimation({"crouch",    28,  4,  8.0f, false});
    // Death animations (indices 32–79, 6 distinct sequences of 8 frames each)
    m_sprite.addAnimation({"death_0",   32,  8,  12.0f, false});
    m_sprite.addAnimation({"death_1",   40,  8,  12.0f, false});
    m_sprite.addAnimation({"death_2",   48,  8,  12.0f, false});
    m_sprite.addAnimation({"death_3",   56,  8,  12.0f, false});
    m_sprite.addAnimation({"death_4",   64,  8,  12.0f, false});
    m_sprite.addAnimation({"death_5",   72,  8,  12.0f, false});

    m_sprite.setAnimation("idle");
}

void Player::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
    m_velX = 0.0f;
    m_velY = 0.0f;
    m_state = State::IDLE;
}

void Player::update(Engine* engine, const Room& room, float dt) {
    if (m_state == State::DEAD) return;
    if (m_state == State::DEATH_ANIM) {
        m_sprite.update(dt);
        if (m_sprite.isAnimationDone()) {
            m_state = State::DEAD;
        }
        return;
    }

    processInput(engine);
    applyPhysics(room, dt);
    updateAnimation();
    m_sprite.update(dt);
}

void Player::processInput(Engine* engine) {
    VirtualGamepad* pad = engine->getGamepad();
    if (!pad) return;

    bool left  = pad->isPressed(VirtualGamepad::BTN_LEFT);
    bool right = pad->isPressed(VirtualGamepad::BTN_RIGHT);
    bool jump  = pad->isJustDown(VirtualGamepad::BTN_JUMP);
    bool run   = pad->isPressed(VirtualGamepad::BTN_RUN);
    bool crouch= pad->isPressed(VirtualGamepad::BTN_CROUCH);

    float speed = run ? RUN_SPEED : WALK_SPEED;

    if (left && !right) {
        m_velX = -speed;
        m_facingLeft = true;
    } else if (right && !left) {
        m_velX = speed;
        m_facingLeft = false;
    } else {
        // Decelerate
        m_velX *= 0.75f;
        if (std::fabs(m_velX) < 1.0f) m_velX = 0.0f;
    }

    if (jump && m_onGround) {
        m_velY = JUMP_FORCE;
        m_onGround = false;
    }

    if (crouch && m_onGround) {
        m_velX = 0.0f;
        m_state = State::CROUCH;
        return;
    }

    // Determine horizontal state
    if (m_onGround) {
        if (std::fabs(m_velX) > 1.0f) {
            m_state = run ? State::RUN : State::WALK;
        } else {
            m_state = State::IDLE;
        }
    }
}

void Player::applyPhysics(const Room& room, float dt) {
    // Gravity
    m_velY = std::min(m_velY + GRAVITY * dt, MAX_FALL_VEL);

    // Move X
    m_x += m_velX * dt;
    resolveCollision(room);

    // Move Y
    m_y += m_velY * dt;
    resolveCollision(room);

    // Air state
    if (!m_onGround) {
        m_state = (m_velY < 0.0f) ? State::JUMP : State::FALL;
    }
}

void Player::resolveCollision(const Room& room) {
    Rectangle hb = getHitbox();
    m_onGround = false;

    // Query solid tiles overlapping with our hitbox
    // Room::getSolidTiles() returns a list of Rectangle tiles in world coords
    for (const Rectangle& tile : room.getSolidTiles(hb)) {
        // Compute overlap
        float overlapX = 0.0f, overlapY = 0.0f;
        float left  = std::max(hb.x,         tile.x);
        float right = std::min(hb.x + hb.width,  tile.x + tile.width);
        float top   = std::max(hb.y,         tile.y);
        float bot   = std::min(hb.y + hb.height, tile.y + tile.height);

        overlapX = right - left;
        overlapY = bot   - top;

        if (overlapX <= 0 || overlapY <= 0) continue;

        // Resolve along minimum overlap axis
        if (overlapX < overlapY) {
            // Horizontal push
            float cx = (hb.x + hb.width  * 0.5f);
            float tx = (tile.x + tile.width * 0.5f);
            m_x += (cx < tx) ? -overlapX : overlapX;
            m_velX = 0.0f;
        } else {
            // Vertical push
            float cy = (hb.y + hb.height * 0.5f);
            float ty = (tile.y + tile.height * 0.5f);
            if (cy < ty) {
                // Landing on top of tile
                m_y -= overlapY;
                m_velY = 0.0f;
                m_onGround = true;
            } else {
                // Hitting ceiling
                m_y += overlapY;
                m_velY = 0.0f;
            }
        }
        // Refresh hitbox after each resolution
        hb = getHitbox();
    }
}

void Player::updateAnimation() {
    switch (m_state) {
        case State::IDLE:   m_sprite.setAnimation("idle");  break;
        case State::WALK:   m_sprite.setAnimation("walk");  break;
        case State::RUN:    m_sprite.setAnimation("run");   break;
        case State::JUMP:   m_sprite.setAnimation("jump");  break;
        case State::FALL:   m_sprite.setAnimation("fall");  break;
        case State::CROUCH: m_sprite.setAnimation("crouch"); break;
        default: break;
    }
}

void Player::kill(int deathAnimIndex) {
    if (m_state == State::DEAD || m_state == State::DEATH_ANIM) return;
    m_state = State::DEATH_ANIM;
    m_velX = 0.0f;
    m_velY = 0.0f;

    char animName[16];
    snprintf(animName, sizeof(animName), "death_%d",
             std::clamp(deathAnimIndex, 0, 5));
    m_sprite.setAnimation(animName);
}

bool Player::isDeathAnimDone() const {
    return m_state == State::DEAD;
}

void Player::render() const {
    if (m_sprite.isLoaded()) {
        m_sprite.draw(m_x - 6.0f, m_y - 4.0f, m_facingLeft);
    } else {
        // Debug rectangle when no sprite is loaded
        DrawRectangle(
            static_cast<int>(m_x),
            static_cast<int>(m_y),
            static_cast<int>(WIDTH),
            static_cast<int>(HEIGHT),
            {0, 200, 255, 200}
        );
    }
}
