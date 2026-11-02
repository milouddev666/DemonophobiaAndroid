#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <cstdint>

/**
 * TileType
 * --------
 * Tile flags stored as uint8 per cell.
 */
enum TileType : uint8_t {
    TILE_EMPTY  = 0,
    TILE_SOLID  = 1,
    TILE_LADDER = 2,
    TILE_HAZARD = 3,   // instant kill on contact (spikes, acid, etc.)
    TILE_WATER  = 4,
};

/**
 * Door
 * ----
 * A region that transitions to another room when the player overlaps it
 * (optionally requiring a key).
 */
struct Door {
    uint32_t  id        = 0;
    Rectangle rect      = {};     // world coords
    uint32_t  targetRoom= 0;
    float     spawnX    = 0.0f;
    float     spawnY    = 0.0f;
    uint8_t   keyRequired = 0;    // ItemID, 0 = no key needed
    bool      locked    = false;
};

/**
 * PickupItem
 * ----------
 * A collectable item placed in the room.
 */
struct PickupItem {
    uint32_t  instanceId = 0;  // unique within room (for state tracking)
    Rectangle rect       = {};
    uint8_t   itemId     = 0;
    bool      collected  = false;
};

/**
 * Room
 * ----
 * Represents a single screen/area of the game.
 * Loaded from binary map data or procedurally defined.
 */
class Room {
public:
    static constexpr int TILE_SIZE = 16;  // virtual pixels

    Room() = default;
    ~Room() = default;

    /**
     * Load from binary format stored in APK assets.
     * Format documented in docs/map_format.md
     * Returns false if file not found; initialises a fallback test room.
     */
    bool load(uint32_t roomId);

    void update(float dt);
    void render() const;

    /**
     * Returns list of solid tile rectangles overlapping with 'query'.
     * Used by Player and Enemy collision resolution.
     */
    std::vector<Rectangle> getSolidTiles(const Rectangle& query) const;

    /**
     * Returns true if 'rect' overlaps any TILE_HAZARD cell.
     */
    bool isHazardAt(const Rectangle& rect) const;

    const std::vector<Door>&       getDoors()   const { return m_doors; }
    const std::vector<PickupItem>& getPickups() const { return m_pickups; }

    int getPixelWidth()  const { return m_cols * TILE_SIZE; }
    int getPixelHeight() const { return m_rows * TILE_SIZE; }

    // BGM track for this room (asset path, empty = silence)
    const std::string& getBGMPath() const { return m_bgmPath; }

    // Background sprite path
    const std::string& getBGPath() const { return m_bgPath; }

    uint32_t getId() const { return m_id; }

private:
    uint32_t m_id   = 0;
    int      m_cols = 0;
    int      m_rows = 0;

    std::vector<uint8_t>    m_tiles;  // row-major: [row*cols + col]
    std::vector<Door>       m_doors;
    std::vector<PickupItem> m_pickups;

    std::string m_bgmPath;
    std::string m_bgPath;

    Texture2D m_bgTexture{};
    bool      m_bgLoaded = false;

    // Tile visual (simple colour mapping when no sprite loaded)
    Texture2D m_tileset{};
    bool      m_tilesetLoaded = false;

    TileType getTile(int col, int row) const;
    void buildTestRoom();   // fallback when no map file
    void loadBackground();
};
