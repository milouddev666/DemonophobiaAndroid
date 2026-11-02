#pragma once
#include <string>
#include <cstdint>
#include <cstddef>  // offsetof

/**
 * SaveData
 * --------
 * Serialised game state written to Android internal storage.
 * Kept small and POD so we can fwrite/fread it directly.
 */
#pragma pack(push, 1)
struct SaveData {
    uint32_t magic       = 0x44454D4F;  // "DEMO"
    uint32_t version     = 1;
    uint32_t currentRoom = 0;
    float    playerX     = 0.0f;
    float    playerY     = 0.0f;
    uint32_t healthFlags = 0;           // bitmask: alive(0), deathCount(bits 1-8)
    uint32_t itemFlags   = 0;           // bitmask per inventory slot
    uint32_t roomFlags   = 0;           // bitmask: doors opened, traps triggered, etc.
    uint32_t checksum    = 0;
};
#pragma pack(pop)

class SaveSystem {
public:
    static constexpr const char* SAVE_FILENAME = "save.dat";

    void init();

    bool save(const SaveData& data);
    bool load(SaveData& outData);
    bool hasSave() const;
    void deleteSave();

private:
    std::string m_savePath;
    uint32_t computeChecksum(const SaveData& d) const;
};
