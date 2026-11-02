#include "Room.h"
#include "../engine/AssetManager.h"
#include <android/log.h>
#include <cstring>
#include <algorithm>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "Room", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Room", __VA_ARGS__)

bool Room::load(uint32_t roomId) {
    m_id = roomId;

    // Attempt to load from binary map file
    char path[64];
    snprintf(path, sizeof(path), "maps/room%02u.bin", roomId);

    auto bytes = AssetManager::readBytes(path);

    if (bytes.size() < 8) {
        LOGI("No map file for room %u — using test room", roomId);
        buildTestRoom();
        return true;   // not a fatal error
    }

    // Binary map format (little-endian):
    // [0-1] uint16 cols
    // [2-3] uint16 rows
    // [4-5] uint16 door_count
    // [6-7] uint16 pickup_count
    // [8..] tile data (cols*rows bytes)
    // [..] door records (20 bytes each)
    // [..] pickup records (12 bytes each)
    // [..] null-terminated BGM path
    // [..] null-terminated BG path

    size_t offset = 0;
    auto readU16 = [&]() -> uint16_t {
        uint16_t v = static_cast<uint16_t>(bytes[offset]) |
                    (static_cast<uint16_t>(bytes[offset+1]) << 8);
        offset += 2;
        return v;
    };
    auto readU32 = [&]() -> uint32_t {
        uint32_t v = bytes[offset]
                   | (static_cast<uint32_t>(bytes[offset+1]) << 8)
                   | (static_cast<uint32_t>(bytes[offset+2]) << 16)
                   | (static_cast<uint32_t>(bytes[offset+3]) << 24);
        offset += 4;
        return v;
    };
    auto readF32 = [&]() -> float {
        float v;
        memcpy(&v, bytes.data() + offset, 4);
        offset += 4;
        return v;
    };

    m_cols = readU16();
    m_rows = readU16();
    uint16_t doorCount   = readU16();
    uint16_t pickupCount = readU16();

    size_t tileBytes = static_cast<size_t>(m_cols) * m_rows;
    if (bytes.size() < offset + tileBytes) {
        LOGE("Map file truncated at tiles: %s", path);
        buildTestRoom();
        return false;
    }

    m_tiles.assign(bytes.begin() + offset,
                   bytes.begin() + offset + tileBytes);
    offset += tileBytes;

    // Doors
    m_doors.resize(doorCount);
    for (int i = 0; i < doorCount; ++i) {
        Door& d = m_doors[i];
        d.id          = readU32();
        d.rect.x      = readF32();
        d.rect.y      = readF32();
        d.rect.width  = readF32();
        d.rect.height = readF32();
        d.targetRoom  = readU32();
        d.spawnX      = readF32();
        d.spawnY      = readF32();
        d.keyRequired = static_cast<uint8_t>(bytes[offset++]);
    }

    // Pickups
    m_pickups.resize(pickupCount);
    for (int i = 0; i < pickupCount; ++i) {
        PickupItem& p = m_pickups[i];
        p.instanceId   = readU32();
        p.rect.x       = readF32();
        p.rect.y       = readF32();
        p.rect.width   = 16.0f;
        p.rect.height  = 16.0f;
        p.itemId       = static_cast<uint8_t>(bytes[offset++]);
    }

    // BGM and BG paths (null-terminated strings)
    auto readStr = [&]() -> std::string {
        std::string s;
        while (offset < bytes.size() && bytes[offset] != 0) {
            s += static_cast<char>(bytes[offset++]);
        }
        offset++;  // consume null
        return s;
    };
    m_bgmPath = readStr();
    m_bgPath  = readStr();

    loadBackground();
    LOGI("Loaded room %u: %dx%d tiles, %u doors, %u pickups",
         roomId, m_cols, m_rows, doorCount, pickupCount);
    return true;
}

void Room::buildTestRoom() {
    // 40 × 30 tile room (640 × 480 virtual px — exactly one screen)
    m_cols = 40;
    m_rows = 30;
    m_tiles.assign(m_cols * m_rows, TILE_EMPTY);
    m_bgmPath = "";
    m_bgPath  = "";

    // Floor
    for (int c = 0; c < m_cols; ++c) {
        m_tiles[27 * m_cols + c] = TILE_SOLID;
        m_tiles[28 * m_cols + c] = TILE_SOLID;
        m_tiles[29 * m_cols + c] = TILE_SOLID;
    }
    // Walls
    for (int r = 0; r < m_rows; ++r) {
        m_tiles[r * m_cols + 0]           = TILE_SOLID;
        m_tiles[r * m_cols + m_cols - 1]  = TILE_SOLID;
    }
    // Ceiling
    for (int c = 0; c < m_cols; ++c) {
        m_tiles[0 * m_cols + c] = TILE_SOLID;
    }
    // A platform
    for (int c = 10; c < 20; ++c) {
        m_tiles[20 * m_cols + c] = TILE_SOLID;
    }
    // Door placeholder (right wall gap → room 1)
    Door testDoor;
    testDoor.id          = 0;
    testDoor.rect        = {static_cast<float>((m_cols-1) * TILE_SIZE),
                             22.0f * TILE_SIZE, 16.0f, 96.0f};
    testDoor.targetRoom  = 1;
    testDoor.spawnX      = 32.0f;
    testDoor.spawnY      = static_cast<float>(22 * TILE_SIZE);
    testDoor.keyRequired = 0;
    testDoor.locked      = false;
    m_doors.push_back(testDoor);
}

TileType Room::getTile(int col, int row) const {
    if (col < 0 || col >= m_cols || row < 0 || row >= m_rows) return TILE_SOLID;
    return static_cast<TileType>(m_tiles[row * m_cols + col]);
}

std::vector<Rectangle> Room::getSolidTiles(const Rectangle& query) const {
    std::vector<Rectangle> result;

    int minCol = static_cast<int>(query.x) / TILE_SIZE;
    int maxCol = static_cast<int>(query.x + query.width)  / TILE_SIZE + 1;
    int minRow = static_cast<int>(query.y) / TILE_SIZE;
    int maxRow = static_cast<int>(query.y + query.height) / TILE_SIZE + 1;

    minCol = std::max(0, minCol);
    maxCol = std::min(m_cols - 1, maxCol);
    minRow = std::max(0, minRow);
    maxRow = std::min(m_rows - 1, maxRow);

    for (int r = minRow; r <= maxRow; ++r) {
        for (int c = minCol; c <= maxCol; ++c) {
            TileType t = getTile(c, r);
            if (t == TILE_SOLID) {
                result.push_back({
                    static_cast<float>(c * TILE_SIZE),
                    static_cast<float>(r * TILE_SIZE),
                    static_cast<float>(TILE_SIZE),
                    static_cast<float>(TILE_SIZE)
                });
            }
        }
    }
    return result;
}

bool Room::isHazardAt(const Rectangle& rect) const {
    int minCol = static_cast<int>(rect.x) / TILE_SIZE;
    int maxCol = static_cast<int>(rect.x + rect.width)  / TILE_SIZE;
    int minRow = static_cast<int>(rect.y) / TILE_SIZE;
    int maxRow = static_cast<int>(rect.y + rect.height) / TILE_SIZE;

    for (int r = minRow; r <= maxRow; ++r) {
        for (int c = minCol; c <= maxCol; ++c) {
            if (getTile(c, r) == TILE_HAZARD) return true;
        }
    }
    return false;
}

void Room::loadBackground() {
    if (m_bgPath.empty()) return;
    auto bytes = AssetManager::readBytes(m_bgPath);
    if (bytes.empty()) return;
    Image img = LoadImageFromMemory(".png", bytes.data(), static_cast<int>(bytes.size()));
    if (img.data) {
        m_bgTexture = LoadTextureFromImage(img);
        UnloadImage(img);
        m_bgLoaded = true;
    }
}

void Room::update(float /*dt*/) {
    // Future: animated tiles, moving platforms, etc.
}

void Room::render() const {
    // Background
    if (m_bgLoaded) {
        DrawTexture(m_bgTexture, 0, 0, WHITE);
    } else {
        DrawRectangle(0, 0, getPixelWidth(), getPixelHeight(), {20, 10, 10, 255});
    }

    // Tiles
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            TileType t = getTile(c, r);
            if (t == TILE_EMPTY) continue;

            Color col;
            switch (t) {
                case TILE_SOLID:  col = {80, 60, 60, 255}; break;
                case TILE_LADDER: col = {100, 80, 40, 255}; break;
                case TILE_HAZARD: col = {200, 0, 0, 255};  break;
                case TILE_WATER:  col = {0, 50, 200, 180}; break;
                default:          col = GRAY;               break;
            }
            DrawRectangle(c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE, col);
        }
    }

    // Door outlines (debug / visual hint)
    for (const auto& door : m_doors) {
        Color dc = door.keyRequired ? RED : YELLOW;
        dc.a = 100;
        DrawRectangleRec(door.rect, dc);
        DrawRectangleLinesEx(door.rect, 2.0f,
            {255, 255, 0, static_cast<unsigned char>(door.keyRequired ? 100 : 200)});
    }

    // Pickups
    for (const auto& item : m_pickups) {
        if (!item.collected) {
            DrawRectangleRec(item.rect, {200, 200, 0, 200});
        }
    }
}
