#pragma once
#include <cstdint>
#include <array>
#include <string>

/**
 * GameState
 * ---------
 * Single source of truth for all mutable game-wide state.
 * Serialised to/from SaveData by SaveSystem.
 */
class GameState {
public:
    static constexpr int MAX_ROOMS     = 64;
    static constexpr int MAX_ITEMS     = 32;
    static constexpr int INVENTORY_SIZE = 8;

    void init();
    void reset();   // new game

    // ─── Room ────────────────────────────────────────────────────────────
    uint32_t currentRoom   = 0;
    float    respawnX      = 0.0f;
    float    respawnY      = 0.0f;

    // Per-room flags: bit N = room N has been visited / door opened / etc.
    uint64_t roomVisited   = 0;
    uint64_t doorOpened    = 0;
    uint64_t trapDisarmed  = 0;

    // ─── Player ──────────────────────────────────────────────────────────
    bool  isAlive      = true;
    int   deathCount   = 0;

    // ─── Inventory ───────────────────────────────────────────────────────
    // Each slot holds an item ID (0 = empty)
    std::array<uint8_t, INVENTORY_SIZE> inventory{};

    bool hasItem(uint8_t itemId) const;
    bool addItem(uint8_t itemId);
    bool removeItem(uint8_t itemId);

    // ─── Helpers ─────────────────────────────────────────────────────────
    void markRoomVisited(uint32_t roomId);
    bool isRoomVisited(uint32_t roomId) const;
    void setDoorOpened(uint32_t doorId);
    bool isDoorOpened(uint32_t doorId) const;
    void setTrapDisarmed(uint32_t trapId);
    bool isTrapDisarmed(uint32_t trapId) const;
};

// ─── Item IDs ────────────────────────────────────────────────────────────────
namespace ItemID {
    constexpr uint8_t NONE         = 0;
    constexpr uint8_t RED_KEY      = 1;
    constexpr uint8_t BLUE_KEY     = 2;
    constexpr uint8_t YELLOW_KEY   = 3;
    constexpr uint8_t RUSTY_KEY    = 4;
    constexpr uint8_t SCREWDRIVER  = 5;
    constexpr uint8_t HAMMER       = 6;
    constexpr uint8_t WIRE_CUTTER  = 7;
    constexpr uint8_t LIGHTER      = 8;
    constexpr uint8_t KNIFE        = 9;
    constexpr uint8_t CROSS        = 10;
    constexpr uint8_t TALISMAN     = 11;
    constexpr uint8_t MAP_PIECE_1  = 12;
    constexpr uint8_t MAP_PIECE_2  = 13;
}
