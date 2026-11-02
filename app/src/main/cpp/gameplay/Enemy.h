#pragma once
#include <raylib.h>
#include "../rendering/SpriteSheet.h"
#include <cstdint>

class Room;
class Player;

/**
 * Enemy
 * -----
 * Represents one enemy instance in a room.
 *
 * AI states:
 *   PATROL  — walk between patrol points, flip at wall/edge
 *   CHASE   — run toward player (triggered by line-of-sight or proximity)
 *   ATTACK  — play attack animation; kills player on contact
 *   STUNNED — brief stun (future: disarm opportunity)
 *   DEAD    — not rendered, no collision
 *
 * Each enemy type has different speeds, detection ranges, and death triggers.
 */
class Enemy {
public:
    enum class AIState { PATROL, CHASE, ATTACK, STUNNED, DEAD };
    enum class Type : uint8_t {
        GHOST        = 0,
        CRAWLER      = 1,
        SHADOW       = 2,
        SPIKED_BALL  = 3,
    };

    static constexpr float WIDTH  = 24.0f;
    static constexpr float HEIGHT = 40.0f;

    Enemy() = default;
    ~Enemy() = default;
    Enemy(Enemy&&) = default;
    Enemy& operator=(Enemy&&) = default;
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;

    void init(Type type, float x, float y);
    void update(const Room& room, const Player& player, float dt);
    void render() const;

    bool isAlive() const { return m_aiState != AIState::DEAD; }
    bool isInAttackRange(const Rectangle& targetHitbox) const;

    Rectangle getHitbox() const {
        return {m_x, m_y, WIDTH, HEIGHT};
    }

    float getX() const { return m_x; }
    float getY() const { return m_y; }

    // Trigger kill — returns appropriate death animation index for player
    int getPlayerDeathAnimIndex() const;

private:
    Type     m_type     = Type::GHOST;
    AIState  m_aiState  = AIState::PATROL;
    float    m_x        = 0.0f;
    float    m_y        = 0.0f;
    float    m_velX     = 0.0f;
    float    m_velY     = 0.0f;
    bool     m_facingLeft = false;
    float    m_stateTimer = 0.0f;

    // Patrol bounds
    float m_patrolLeft  = 0.0f;
    float m_patrolRight = 0.0f;

    SpriteSheet m_sprite;

    float patrolSpeed()    const;
    float chaseSpeed()     const;
    float detectionRange() const;
    float attackRange()    const;

    void updatePatrol(const Room& room, float dt);
    void updateChase(const Player& player, float dt);
    void updateAttack(const Player& player, float dt);

    bool hasLineOfSight(const Player& player, const Room& room) const;
};
