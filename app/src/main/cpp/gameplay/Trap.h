#pragma once
#include <raylib.h>
#include <cstdint>
#include <functional>

/**
 * TrapType
 * --------
 * Each type triggers a specific death animation and sound effect.
 * Mirrored from the original game's trap system.
 */
enum class TrapType : uint8_t {
    SPIKE_FLOOR    = 0,   // step on floor spikes
    SPIKE_CEILING  = 1,   // ceiling drops
    BLADE_PENDULUM = 2,   // swinging blade
    ACID_PIT       = 3,   // fall into acid
    CRUSHING_WALL  = 4,   // walls close in
    WIRE_SNARE     = 5,   // wire pulls tight
    FIRE_JET       = 6,   // timed flame
    FALLING_WEIGHT = 7,   // heavy object falls
    SAW_BLADE      = 8,   // horizontal saw
    NEEDLE_WALL    = 9,   // sudden needle array
    PRESS          = 10,  // hydraulic press
    ELECTROCUTION  = 11,  // electric floor/wire
    COUNT          = 12
};

/**
 * TrapState
 * ---------
 * Trap activity lifecycle.
 */
enum class TrapState {
    IDLE,       // inactive / waiting
    ARMED,      // trigger zone active
    TRIGGERED,  // animation playing
    DONE        // finished (non-resetting traps)
};

/**
 * Trap
 * ----
 * A single trap instance in a room.
 *
 * Traps have:
 *  - A trigger zone (player must enter to activate)
 *  - An optional arm delay (e.g. timed fire jets)
 *  - An execution zone (player must be inside when triggered to die)
 *  - A death animation index (passed to Player::kill())
 */
class Trap {
public:
    Trap() = default;
    ~Trap() = default;
    Trap(Trap&&) = default;
    Trap& operator=(Trap&&) = default;
    Trap(const Trap&) = default;
    Trap& operator=(const Trap&) = default;

    using KillCallback = std::function<void(int deathAnimIndex)>;

    uint32_t  instanceId   = 0;
    TrapType  type         = TrapType::SPIKE_FLOOR;
    Rectangle triggerZone  = {};
    Rectangle killZone     = {};
    bool      resettable   = false;   // can re-trigger after reset
    float     armDelay     = 0.0f;    // seconds before trigger activates
    float     resetDelay   = 3.0f;    // seconds until trap resets

    // Set by GameplayScene
    KillCallback onKillPlayer;

    void init();
    void update(const Rectangle& playerHitbox, float dt);
    void render() const;

    bool isDisarmed() const { return m_disarmed; }
    void disarm() { m_disarmed = true; }

    TrapState getState() const { return m_state; }

private:
    TrapState m_state     = TrapState::IDLE;
    float     m_timer     = 0.0f;
    bool      m_disarmed  = false;

    // Animation helpers
    float m_animAngle   = 0.0f;  // for pendulum
    float m_animOffset  = 0.0f;  // for moving parts

    void renderSpike()     const;
    void renderPendulum()  const;
    void renderPress()     const;
    void renderGeneric()   const;
};
