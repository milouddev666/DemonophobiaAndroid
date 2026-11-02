#include "Trap.h"
#include <cmath>

void Trap::init() {
    m_state    = (armDelay > 0.0f) ? TrapState::IDLE : TrapState::ARMED;
    m_timer    = 0.0f;
    m_disarmed = false;
}

void Trap::update(const Rectangle& playerHitbox, float dt) {
    if (m_disarmed) return;

    m_timer += dt;

    // Update visual animation
    m_animAngle  = std::fmod(m_animAngle  + 120.0f * dt, 360.0f);
    m_animOffset = std::sin(m_timer * 3.0f) * 12.0f;

    switch (m_state) {
        case TrapState::IDLE:
            if (m_timer >= armDelay) {
                m_timer = 0.0f;
                m_state = TrapState::ARMED;
            }
            break;

        case TrapState::ARMED:
            // Check player entry into trigger zone
            if (CheckCollisionRecs(playerHitbox, triggerZone)) {
                m_state = TrapState::TRIGGERED;
                m_timer = 0.0f;
            }
            break;

        case TrapState::TRIGGERED:
            // Check if player is in kill zone
            if (m_timer < 0.15f) {  // grace period before kill check
                break;
            }
            if (CheckCollisionRecs(playerHitbox, killZone)) {
                if (onKillPlayer) {
                    onKillPlayer(static_cast<int>(type) % 6);
                }
            }
            // Transition to done
            if (m_timer >= resetDelay) {
                m_state = resettable ? TrapState::ARMED : TrapState::DONE;
                m_timer = 0.0f;
            }
            break;

        case TrapState::DONE:
            break;
    }
}

void Trap::render() const {
    if (m_disarmed) return;

    switch (type) {
        case TrapType::SPIKE_FLOOR:
        case TrapType::SPIKE_CEILING:
        case TrapType::NEEDLE_WALL:
            renderSpike();
            break;
        case TrapType::BLADE_PENDULUM:
            renderPendulum();
            break;
        case TrapType::CRUSHING_WALL:
        case TrapType::PRESS:
            renderPress();
            break;
        default:
            renderGeneric();
            break;
    }

    // Debug: show trigger zone when armed
#ifdef DEBUG_BUILD
    Color tc = (m_state == TrapState::ARMED) ? RED : YELLOW;
    tc.a = 60;
    DrawRectangleRec(triggerZone, tc);
    DrawRectangleRec(killZone, {255, 0, 0, 40});
#endif
}

void Trap::renderSpike() const {
    Color col = (m_state == TrapState::TRIGGERED) ? RED : Color{120, 120, 120, 255};

    // Draw spike triangles across the kill zone
    float spikeW = 8.0f;
    int count = static_cast<int>(killZone.width / spikeW);
    for (int i = 0; i < count; ++i) {
        float bx = killZone.x + i * spikeW;
        float by = killZone.y + killZone.height;
        float tx = bx + spikeW * 0.5f;
        float ty = killZone.y;
        DrawTriangle(
            {bx, by}, {bx + spikeW, by}, {tx, ty},
            col
        );
    }
}

void Trap::renderPendulum() const {
    // Pivot at top-centre of kill zone, swinging blade
    float px = killZone.x + killZone.width  * 0.5f;
    float py = killZone.y;
    float len = killZone.height;
    float angle = std::sin(m_timer * 2.5f) * 45.0f * DEG2RAD;
    float bx = px + std::sin(angle) * len;
    float by = py + std::cos(angle) * len;

    DrawLine(static_cast<int>(px), static_cast<int>(py),
             static_cast<int>(bx), static_cast<int>(by),
             {150, 150, 150, 255});
    DrawCircle(static_cast<int>(bx), static_cast<int>(by), 10,
               (m_state == TrapState::TRIGGERED) ? RED : GRAY);
}

void Trap::renderPress() const {
    float yOff = (m_state == TrapState::TRIGGERED) ? m_animOffset * 2.0f : 0.0f;
    Color col = (m_state == TrapState::TRIGGERED)
                ? Color{200, 50, 50, 255} : Color{100, 100, 100, 255};
    DrawRectangle(
        static_cast<int>(killZone.x),
        static_cast<int>(killZone.y + yOff),
        static_cast<int>(killZone.width),
        static_cast<int>(killZone.height * 0.3f),
        col
    );
}

void Trap::renderGeneric() const {
    Color col = (m_state == TrapState::TRIGGERED)
                ? Color{255, 100, 0, 200} : Color{180, 80, 0, 150};
    DrawRectangleRec(killZone, col);
    DrawRectangleLinesEx(killZone, 2.0f, Color{255, 150, 0, 255});
}
