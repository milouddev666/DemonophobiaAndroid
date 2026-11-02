/**
 * GameplayScene.cpp
 * -----------------
 * Main in-game scene.
 *
 * [PROTOTYPE] All game objects are populated from hardcoded test data
 * until the original Demonophobia map/enemy/trap files are provided.
 * Nothing in this file claims to reproduce original game content.
 */

#include "GameplayScene.h"
#include "PauseScene.h"
#include "GameOverScene.h"
#include "../../engine/Engine.h"
#include "../../engine/SceneManager.h"
#include "../../engine/SaveSystem.h"
#include "../../audio/AudioManager.h"
#include "../../input/VirtualGamepad.h"
#include "../../input/TouchInput.h"
#include "../../rendering/Renderer.h"

#include <raylib.h>
#include <android/log.h>
#include <algorithm>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "GameScene", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "GameScene", __VA_ARGS__)

// ─── Death respawn timings ────────────────────────────────────────────────────
static constexpr float DEATH_ANIM_TIMEOUT = 3.0f;  // max seconds for death anim
static constexpr float DEATH_PAUSE_DUR    = 0.8f;  // pause after anim ends
static constexpr float RESPAWN_FADE_DUR   = 0.5f;

// ─── Test enemy placements for room 0 ────────────────────────────────────────
// [PROTOTYPE DATA] — NOT original Demonophobia enemy positions
struct EnemySpec { Enemy::Type type; float x, y; };
static const EnemySpec k_room0_enemies[] = {
    { Enemy::Type::GHOST,   420.0f, 384.0f },
    { Enemy::Type::CRAWLER, 250.0f, 416.0f },
};

// ─── Test trap placements for room 0 ─────────────────────────────────────────
// [PROTOTYPE DATA] — NOT original Demonophobia trap layout
struct TrapSpec {
    TrapType type;
    Rectangle trigger;
    Rectangle kill;
    bool resettable;
    float armDelay;
};
static const TrapSpec k_room0_traps[] = {
    {
        TrapType::SPIKE_FLOOR,
        {200.0f, 416.0f, 48.0f, 16.0f},   // trigger: step on this area
        {200.0f, 416.0f, 48.0f, 16.0f},   // kill:    same zone
        false, 0.0f
    },
    {
        TrapType::BLADE_PENDULUM,
        {460.0f, 300.0f, 40.0f, 80.0f},   // trigger: walk under
        {455.0f, 290.0f, 50.0f, 100.0f},  // kill:    wider swing zone
        true, 0.0f
    },
};

// ─────────────────────────────────────────────────────────────────────────────

GameplayScene::GameplayScene(uint32_t roomId) : m_roomId(roomId) {}
GameplayScene::~GameplayScene() = default;

void GameplayScene::onEnter(Engine* engine) {
    m_engine = engine;
    LOGI("Entering room %u", m_roomId);

    loadRoom(m_roomId);

    // Spawn player at respawn point from GameState (set by save load or reset)
    GameState* gs = engine->getGameState();
    float spawnX = (gs->respawnX > 0.0f) ? gs->respawnX : 64.0f;
    float spawnY = (gs->respawnY > 0.0f) ? gs->respawnY : 400.0f;

    m_player.init(engine);
    m_player.setPosition(spawnX, spawnY);

    m_camera.init(m_room.getPixelWidth(), m_room.getPixelHeight());
    m_camera.follow(m_player.getCX(), m_player.getCY());

    m_phase      = Phase::PLAYING;
    m_phaseTimer = 0.0f;
    m_inventoryUI.close();

    // BGM
    const std::string& bgm = m_room.getBGMPath();
    if (!bgm.empty()) {
        engine->getAudioManager()->playBGM(bgm);
    }

    gs->markRoomVisited(m_roomId);
}

void GameplayScene::onExit() {
    // Audio continues — stopped/changed by the next scene's onEnter
}

void GameplayScene::loadRoom(uint32_t roomId) {
    m_enemies.clear();
    m_traps.clear();

    m_room.load(roomId);  // uses test room if map file not found
    populateEnemies();
    populateTraps();
    LOGI("Room %u loaded: %dx%d tiles", roomId,
         m_room.getPixelWidth() / Room::TILE_SIZE,
         m_room.getPixelHeight() / Room::TILE_SIZE);
}

void GameplayScene::populateEnemies() {
    if (m_roomId == 0) {
        for (auto& spec : k_room0_enemies) {
            m_enemies.emplace_back();
            m_enemies.back().init(spec.type, spec.x, spec.y);
        }
    }
    // [FUTURE] Parse enemy data from room binary file once format is known
}

void GameplayScene::populateTraps() {
    GameState* gs = m_engine ? m_engine->getGameState() : nullptr;

    if (m_roomId == 0) {
        for (uint32_t i = 0; i < std::size(k_room0_traps); ++i) {
            const TrapSpec& spec = k_room0_traps[i];

            // Skip disarmed traps (persistent across respawns)
            if (gs && gs->isTrapDisarmed(i)) continue;

            Trap t;
            t.instanceId  = i;
            t.type        = spec.type;
            t.triggerZone = spec.trigger;
            t.killZone    = spec.kill;
            t.resettable  = spec.resettable;
            t.armDelay    = spec.armDelay;
            t.resetDelay  = 3.0f;
            t.init();

            // Connect kill callback
            t.onKillPlayer = [this](int deathAnim) {
                triggerDeath(deathAnim);
            };

            m_traps.push_back(std::move(t));
        }
    }
}

// ─── Update ───────────────────────────────────────────────────────────────────

void GameplayScene::update(Engine* engine, float dt) {
    VirtualGamepad* pad = engine->getGamepad();

    // Inventory toggle (C button)
    if (pad->isJustDown(VirtualGamepad::BTN_INVENTORY)) {
        m_inventoryUI.toggle();
    }

    if (m_inventoryUI.isOpen()) {
        // Handle inventory touch selection
        TouchInput* touch = engine->getTouchInput();
        Renderer*   ren   = engine->getRenderer();
        for (int i = 0; i < TouchInput::MAX_POINTS; ++i) {
            const TouchPoint& tp = touch->getPoint(i);
            if (!tp.justPressed) continue;
            float vx = ren->screenToVirtualX(tp.x);
            float vy = ren->screenToVirtualY(tp.y);
            int slot = m_inventoryUI.hitTest(vx, vy);
            if (slot >= 0) m_inventoryUI.setSelectedSlot(slot);
        }
        return;  // gameplay paused while inventory open
    }

    switch (m_phase) {
        case Phase::PLAYING:
            updatePlaying(dt);
            break;

        case Phase::DEATH_ANIM:
            m_phaseTimer += dt;
            m_player.update(engine, m_room, dt);  // plays death animation
            if (m_player.isDead() || m_phaseTimer >= DEATH_ANIM_TIMEOUT) {
                m_phase      = Phase::DEATH_PAUSE;
                m_phaseTimer = 0.0f;
                LOGI("Death anim complete — pausing");
            }
            break;

        case Phase::DEATH_PAUSE:
            m_phaseTimer += dt;
            if (m_phaseTimer >= DEATH_PAUSE_DUR) {
                // Show game-over or respawn
                engine->getGameState()->deathCount++;
                engine->getSceneManager()->changeSceneFade(
                    std::make_unique<GameOverScene>(m_roomId), 0.5f);
            }
            break;

        case Phase::RESPAWNING:
            m_phaseTimer += dt;
            if (m_phaseTimer >= RESPAWN_FADE_DUR) {
                m_phase = Phase::PLAYING;
            }
            break;

        case Phase::TRANSITION:
            // Room transition handled by SceneManager fade
            break;
    }
}

void GameplayScene::updatePlaying(float dt) {
    // Update player
    m_player.update(m_engine, m_room, dt);

    // Update enemies
    for (auto& enemy : m_enemies) {
        enemy.update(m_room, m_player, dt);
    }

    // Update traps (they fire their own kill callback)
    Rectangle playerHB = m_player.getHitbox();
    for (auto& trap : m_traps) {
        trap.update(playerHB, dt);
    }

    // Update room (animated tiles etc.)
    m_room.update(dt);

    // Collision checks
    if (!m_player.isDead()) {
        checkTrapCollisions();     // hazard tiles (separate from Trap objects)
        checkEnemyCollisions();
        checkDoorCollisions();
        checkPickupCollisions();
    }

    // Camera follow
    m_camera.follow(m_player.getCX(), m_player.getCY());
}

void GameplayScene::checkDoorCollisions() {
    Rectangle phb = m_player.getHitbox();
    GameState* gs = m_engine->getGameState();

    for (const Door& door : m_room.getDoors()) {
        if (!CheckCollisionRecs(phb, door.rect)) continue;

        // Require action button press OR automatic (keyRequired == 0)
        VirtualGamepad* pad = m_engine->getGamepad();
        bool useKey = pad->isJustDown(VirtualGamepad::BTN_ACTION);

        if (door.keyRequired != 0) {
            if (!useKey) continue;
            if (!gs->hasItem(door.keyRequired)) {
                // [TODO] Show "requires key" message
                continue;
            }
            gs->removeItem(door.keyRequired);
            gs->setDoorOpened(door.id);
        }

        // Trigger room transition
        LOGI("Door %u → room %u (%.0f,%.0f)",
             door.id, door.targetRoom, door.spawnX, door.spawnY);

        gs->currentRoom = door.targetRoom;
        gs->respawnX    = door.spawnX;
        gs->respawnY    = door.spawnY;

        saveProgress();

        m_engine->getSceneManager()->changeSceneFade(
            std::make_unique<GameplayScene>(door.targetRoom), 0.5f);
        m_phase = Phase::TRANSITION;
        return;
    }
}

void GameplayScene::checkPickupCollisions() {
    Rectangle phb = m_player.getHitbox();
    GameState* gs = m_engine->getGameState();

    for (PickupItem& item : const_cast<std::vector<PickupItem>&>(m_room.getPickups())) {
        if (item.collected) continue;
        if (!CheckCollisionRecs(phb, item.rect)) continue;
        if (gs->addItem(item.itemId)) {
            item.collected = true;
            LOGI("Picked up item %u", item.itemId);
            m_engine->getAudioManager()->playSFX("pickup", 0.8f);
            // [TODO] Display "item acquired" message overlay
        }
    }
}

void GameplayScene::checkTrapCollisions() {
    // Check TILE_HAZARD contact (separate from Trap objects)
    if (m_room.isHazardAt(m_player.getHitbox())) {
        triggerDeath(0);
    }
}

void GameplayScene::checkEnemyCollisions() {
    for (const Enemy& e : m_enemies) {
        if (!e.isAlive()) continue;
        if (e.isInAttackRange(m_player.getHitbox())) {
            triggerDeath(e.getPlayerDeathAnimIndex());
            return;
        }
    }
}

void GameplayScene::triggerDeath(int deathAnimIndex) {
    if (m_phase != Phase::PLAYING) return;  // already dying
    LOGI("Player killed — death anim %d", deathAnimIndex);
    m_player.kill(deathAnimIndex);
    m_phase      = Phase::DEATH_ANIM;
    m_phaseTimer = 0.0f;
    m_engine->getAudioManager()->playSFX("death", 1.0f);
}

void GameplayScene::doRespawn() {
    GameState* gs = m_engine->getGameState();
    m_player.setPosition(gs->respawnX, gs->respawnY);
    m_phase      = Phase::RESPAWNING;
    m_phaseTimer = 0.0f;
    // Rearm resettable traps
    for (auto& trap : m_traps) {
        if (trap.resettable) trap.init();
    }
}

void GameplayScene::saveProgress() {
    GameState* gs = m_engine->getGameState();
    SaveData sd;
    sd.currentRoom = gs->currentRoom;
    sd.playerX     = gs->respawnX;
    sd.playerY     = gs->respawnY;
    sd.healthFlags = static_cast<uint32_t>(gs->deathCount) << 1;
    sd.roomFlags   = static_cast<uint32_t>(gs->doorOpened & 0xFFFFFFFF);
    sd.itemFlags   = 0;
    for (int i = 0; i < GameState::INVENTORY_SIZE; ++i) {
        sd.itemFlags |= (static_cast<uint32_t>(gs->inventory[i]) << (i * 4));
    }
    m_engine->getSaveSystem()->save(sd);
}

// ─── Render ───────────────────────────────────────────────────────────────────

void GameplayScene::render(Renderer& /*renderer*/) {
    // Camera begins — world-space draw calls
    m_camera.beginDraw();

    m_room.render();

    for (const auto& trap : m_traps) {
        trap.render();
    }

    for (const auto& enemy : m_enemies) {
        enemy.render();
    }

    m_player.render();

    m_camera.endDraw();

    // ── HUD (screen-space / virtual canvas, no camera transform) ──────────

    // Death count (prototype HUD)
    GameState* gs = m_engine ? m_engine->getGameState() : nullptr;
    if (gs) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Deaths: %d", gs->deathCount);
        DrawText(buf, 8, 8, 10, {160, 160, 160, 200});
    }

    // Room indicator (prototype, not in original game)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Room: %u [PROTOTYPE]", m_roomId);
        DrawText(buf, 8, 20, 8, {80, 80, 80, 180});
    }

    // Inventory overlay
    if (gs) {
        m_inventoryUI.render(*gs, m_inventoryUI.getSelectedSlot());
    }

    // Death-phase fade overlay
    if (m_phase == Phase::DEATH_PAUSE) {
        float t = m_phaseTimer / DEATH_PAUSE_DUR;
        unsigned char a = static_cast<unsigned char>(t * 180);
        DrawRectangle(0, 0, Renderer::VIRTUAL_W, Renderer::VIRTUAL_H, {0, 0, 0, a});
    }
    if (m_phase == Phase::RESPAWNING) {
        float t = 1.0f - (m_phaseTimer / RESPAWN_FADE_DUR);
        unsigned char a = static_cast<unsigned char>(t * 255);
        DrawRectangle(0, 0, Renderer::VIRTUAL_W, Renderer::VIRTUAL_H, {0, 0, 0, a});
    }
}

void GameplayScene::onBackPressed(Engine* engine) {
    // Back button → pause menu
    if (m_phase == Phase::PLAYING) {
        engine->getSceneManager()->changeScene(
            std::make_unique<PauseScene>(m_roomId));
    }
}
