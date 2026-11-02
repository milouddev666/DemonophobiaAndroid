#include "Enemy.h"
#include "Room.h"
#include "Player.h"
#include <cmath>
#include <algorithm>

void Enemy::init(Type type, float x, float y) {
    m_type   = type;
    m_x      = x;
    m_y      = y;
    m_aiState = AIState::PATROL;
    m_patrolLeft  = x - 80.0f;
    m_patrolRight = x + 80.0f;
    m_velX = patrolSpeed();
    m_facingLeft = false;

    // Load sprite based on type
    char spritePath[64];
    snprintf(spritePath, sizeof(spritePath), "sprites/enemy_%d.png",
             static_cast<int>(type));
    m_sprite.load(spritePath, 32, 40);

    m_sprite.addAnimation({"patrol", 0, 4, 8.0f,  true});
    m_sprite.addAnimation({"chase",  4, 6, 12.0f, true});
    m_sprite.addAnimation({"attack", 10, 4, 10.0f, false});
    m_sprite.setAnimation("patrol");
}

float Enemy::patrolSpeed()    const { return 40.0f  + static_cast<int>(m_type) * 10.0f; }
float Enemy::chaseSpeed()     const { return 90.0f  + static_cast<int>(m_type) * 20.0f; }
float Enemy::detectionRange() const { return 120.0f + static_cast<int>(m_type) * 30.0f; }
float Enemy::attackRange()    const { return 28.0f; }

void Enemy::update(const Room& room, const Player& player, float dt) {
    if (m_aiState == AIState::DEAD) return;

    m_stateTimer -= dt;

    switch (m_aiState) {
        case AIState::PATROL:
            updatePatrol(room, dt);
            if (hasLineOfSight(player, room)) {
                m_aiState = AIState::CHASE;
                m_sprite.setAnimation("chase");
            }
            break;
        case AIState::CHASE:
            updateChase(player, dt);
            if (isInAttackRange(player.getHitbox())) {
                m_aiState = AIState::ATTACK;
                m_stateTimer = 0.5f;
                m_velX = 0.0f;
                m_sprite.setAnimation("attack");
            }
            break;
        case AIState::ATTACK:
            updateAttack(player, dt);
            break;
        case AIState::STUNNED:
            if (m_stateTimer <= 0.0f) {
                m_aiState = AIState::PATROL;
                m_sprite.setAnimation("patrol");
            }
            break;
        default:
            break;
    }

    // Gravity
    m_velY = std::min(m_velY + 700.0f * dt, 500.0f);
    m_y += m_velY * dt;

    // Simple vertical collision (enemies stay on ground)
    Rectangle hb = getHitbox();
    for (const Rectangle& tile : room.getSolidTiles(hb)) {
        float bottom = hb.y + hb.height;
        float tileTop = tile.y;
        if (bottom > tileTop && hb.y < tileTop && m_velY > 0) {
            m_y    -= (bottom - tileTop);
            m_velY  = 0.0f;
        }
    }

    m_sprite.update(dt);
}

void Enemy::updatePatrol(const Room& room, float dt) {
    m_x += m_velX * dt;

    // Flip at patrol boundaries or wall
    bool hitWall = false;
    Rectangle hb = getHitbox();
    for (const Rectangle& tile : room.getSolidTiles({m_x + (m_velX > 0 ? WIDTH : 0), m_y, 4, HEIGHT})) {
        hitWall = true;
        break;
    }

    if (m_x <= m_patrolLeft || m_x + WIDTH >= m_patrolRight || hitWall) {
        m_velX     = -m_velX;
        m_facingLeft = (m_velX < 0);
    }
}

void Enemy::updateChase(const Player& player, float dt) {
    float dx = player.getCX() - (m_x + WIDTH * 0.5f);
    m_facingLeft = (dx < 0);
    float spd = chaseSpeed();

    if (std::fabs(dx) > 4.0f) {
        m_velX = (dx < 0) ? -spd : spd;
    } else {
        m_velX = 0.0f;
    }
    m_x += m_velX * dt;

    // Give up chase if player is very far
    float dist = std::fabs(player.getCX() - (m_x + WIDTH * 0.5f))
               + std::fabs(player.getCY() - (m_y + HEIGHT * 0.5f));
    if (dist > detectionRange() * 2.0f) {
        m_aiState = AIState::PATROL;
        m_sprite.setAnimation("patrol");
    }
}

void Enemy::updateAttack(const Player& player, float dt) {
    (void)player; (void)dt;
    // Attack animation plays; GameplayScene checks isInAttackRange to kill player
}

bool Enemy::hasLineOfSight(const Player& player, const Room& /*room*/) const {
    float dx = player.getCX() - (m_x + WIDTH * 0.5f);
    float dy = player.getCY() - (m_y + HEIGHT * 0.5f);
    float dist = std::sqrt(dx*dx + dy*dy);

    // Simple range check; a proper LOS ray would iterate tiles
    return dist < detectionRange() && std::fabs(dy) < HEIGHT * 2.0f;
}

bool Enemy::isInAttackRange(const Rectangle& targetHitbox) const {
    Rectangle hb = getHitbox();
    return CheckCollisionRecs(
        {hb.x - attackRange(), hb.y, hb.width + attackRange()*2, hb.height},
        targetHitbox
    );
}

int Enemy::getPlayerDeathAnimIndex() const {
    // Each enemy type maps to a distinct death animation
    return static_cast<int>(m_type) % 6;
}

void Enemy::render() const {
    if (m_aiState == AIState::DEAD) return;

    if (m_sprite.isLoaded()) {
        m_sprite.draw(m_x, m_y, m_facingLeft);
    } else {
        Color col = (m_aiState == AIState::CHASE) ? RED : MAROON;
        DrawRectangle(
            static_cast<int>(m_x),
            static_cast<int>(m_y),
            static_cast<int>(WIDTH),
            static_cast<int>(HEIGHT),
            col
        );
        // Eye indicator
        DrawCircle(
            static_cast<int>(m_x + (m_facingLeft ? 6 : 18)),
            static_cast<int>(m_y + 10),
            4, WHITE
        );
    }
}
