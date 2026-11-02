#pragma once
#include <raylib.h>
#include "../rendering/SpriteSheet.h"

class Engine;
class Room;
class VirtualGamepad;

/**
 * Player
 * ------
 * Handles Sacchi's movement, animation, and collision response.
 *
 * State machine:
 *   IDLE → WALK → RUN → JUMP → FALL → CROUCH → DEAD
 *
 * Physics: simple velocity + gravity, no sub-pixel stuttering.
 * Collision: AABB against tile map provided by Room.
 */
class Player {
public:
    enum class State {
        IDLE,
        WALK,
        RUN,
        JUMP,
        FALL,
        CROUCH,
        DEAD,
        DEATH_ANIM   // playing death animation (blocks all input)
    };

    // Hitbox size (virtual pixels)
    static constexpr float WIDTH  = 20.0f;
    static constexpr float HEIGHT = 44.0f;

    // Physics constants (virtual-pixel units per second)
    static constexpr float WALK_SPEED   =  80.0f;
    static constexpr float RUN_SPEED    = 150.0f;
    static constexpr float JUMP_FORCE   = -340.0f;
    static constexpr float GRAVITY      =  700.0f;
    static constexpr float MAX_FALL_VEL =  500.0f;

    Player();
    ~Player();

    void init(Engine* engine);
    void setPosition(float x, float y);
    void update(Engine* engine, const Room& room, float dt);
    void render() const;

    void kill(int deathAnimIndex = 0);  // trigger death sequence
    bool isDead()        const { return m_state == State::DEAD; }
    bool isDeathAnimDone() const;

    float getX()    const { return m_x; }
    float getY()    const { return m_y; }
    float getCX()   const { return m_x + WIDTH  * 0.5f; }  // centre
    float getCY()   const { return m_y + HEIGHT * 0.5f; }

    Rectangle getHitbox() const {
        return {m_x, m_y, WIDTH, HEIGHT};
    }

    bool isFacingLeft() const { return m_facingLeft; }
    State getState()    const { return m_state; }

private:
    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_velX = 0.0f;
    float m_velY = 0.0f;
    bool  m_onGround    = false;
    bool  m_facingLeft  = false;
    State m_state       = State::IDLE;

    SpriteSheet m_sprite;

    void processInput(Engine* engine);
    void applyPhysics(const Room& room, float dt);
    void updateAnimation();
    void resolveCollision(const Room& room);
};
