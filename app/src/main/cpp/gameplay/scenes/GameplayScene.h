#pragma once
#include "../../engine/SceneManager.h"
#include "../../gameplay/Player.h"
#include "../../gameplay/Room.h"
#include "../../gameplay/Enemy.h"
#include "../../gameplay/Trap.h"
#include "../../gameplay/Inventory.h"
#include "../../rendering/Camera.h"
#include <vector>
#include <memory>
#include <cstdint>

/**
 * GameplayScene
 * -------------
 * The main gameplay scene. Hosts:
 *  - Player (movement, collision, state)
 *  - Room (tilemap, doors, pickups)
 *  - Enemies (patrol/chase/attack AI)
 *  - Traps (trigger/kill zones, death sequences)
 *  - Inventory UI overlay
 *  - Death → respawn flow
 *  - Door → room transition
 *
 * [PROTOTYPE STATUS]
 *  Room 0 is a built-in test room (no original Demonophobia map data).
 *  Player/enemy/trap sprites are placeholder rectangles until original
 *  assets are placed in assets/sprites/.
 *
 * Original game content integration:
 *  1. Place sprite sheets in assets/sprites/
 *  2. Place room files in assets/maps/room00.bin … room##.bin
 *  3. Place audio in assets/audio/
 *  The room loader (Room::load) will pick them up automatically.
 */
class GameplayScene : public Scene {
public:
    explicit GameplayScene(uint32_t roomId);
    ~GameplayScene() override;

    const char* name() const override { return "GameplayScene"; }

    void onEnter(Engine* engine) override;
    void onExit()                override;
    void update(Engine* engine, float dt) override;
    void render(Renderer& renderer)       override;
    void onBackPressed(Engine* engine)    override;

private:
    // ─── Room & objects ────────────────────────────────────────────────────
    uint32_t  m_roomId;
    Room      m_room;
    Player    m_player;
    GameCamera m_camera;

    std::vector<Enemy> m_enemies;
    std::vector<Trap>  m_traps;
    InventoryUI        m_inventoryUI;

    // ─── State ─────────────────────────────────────────────────────────────
    enum class Phase {
        PLAYING,
        DEATH_ANIM,   // player death animation running
        DEATH_PAUSE,  // brief pause after animation (0.5 s)
        RESPAWNING,   // fade back in after respawn
        TRANSITION,   // fade-out for room change
    };
    Phase m_phase         = Phase::PLAYING;
    float m_phaseTimer    = 0.0f;
    int   m_pendingRoom   = -1;
    float m_pendingSpawnX = 0.0f;
    float m_pendingSpawnY = 0.0f;

    // ─── Helpers ───────────────────────────────────────────────────────────
    Engine* m_engine = nullptr;   // back-pointer (valid during scene lifetime)

    void loadRoom(uint32_t roomId);
    void populateEnemies();
    void populateTraps();

    void updatePlaying(float dt);
    void checkDoorCollisions();
    void checkPickupCollisions();
    void checkTrapCollisions();
    void checkEnemyCollisions();

    void triggerDeath(int deathAnimIndex);
    void doRespawn();

    void saveProgress();
};
