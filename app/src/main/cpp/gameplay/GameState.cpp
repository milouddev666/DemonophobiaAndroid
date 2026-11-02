#include "GameState.h"
#include <algorithm>

void GameState::init() {
    reset();
}

void GameState::reset() {
    currentRoom   = 0;
    respawnX      = 64.0f;
    respawnY      = 400.0f;
    roomVisited   = 0;
    doorOpened    = 0;
    trapDisarmed  = 0;
    isAlive       = true;
    deathCount    = 0;
    inventory.fill(ItemID::NONE);
}

bool GameState::hasItem(uint8_t itemId) const {
    for (auto id : inventory) {
        if (id == itemId) return true;
    }
    return false;
}

bool GameState::addItem(uint8_t itemId) {
    for (auto& slot : inventory) {
        if (slot == ItemID::NONE) {
            slot = itemId;
            return true;
        }
    }
    return false;  // inventory full
}

bool GameState::removeItem(uint8_t itemId) {
    for (auto& slot : inventory) {
        if (slot == itemId) {
            slot = ItemID::NONE;
            return true;
        }
    }
    return false;
}

void GameState::markRoomVisited(uint32_t roomId) {
    if (roomId < 64) roomVisited |= (1ULL << roomId);
}

bool GameState::isRoomVisited(uint32_t roomId) const {
    if (roomId >= 64) return false;
    return (roomVisited & (1ULL << roomId)) != 0;
}

void GameState::setDoorOpened(uint32_t doorId) {
    if (doorId < 64) doorOpened |= (1ULL << doorId);
}

bool GameState::isDoorOpened(uint32_t doorId) const {
    if (doorId >= 64) return false;
    return (doorOpened & (1ULL << doorId)) != 0;
}

void GameState::setTrapDisarmed(uint32_t trapId) {
    if (trapId < 64) trapDisarmed |= (1ULL << trapId);
}

bool GameState::isTrapDisarmed(uint32_t trapId) const {
    if (trapId >= 64) return false;
    return (trapDisarmed & (1ULL << trapId)) != 0;
}
