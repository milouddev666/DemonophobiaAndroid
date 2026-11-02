#include "SaveSystem.h"
#include "../android/AndroidBridge.h"
#include <cstdio>
#include <cstring>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "SaveSys", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "SaveSys", __VA_ARGS__)

void SaveSystem::init() {
    m_savePath = AndroidBridge::getInternalPath() + "/" + SAVE_FILENAME;
    LOGI("Save path: %s", m_savePath.c_str());
}

uint32_t SaveSystem::computeChecksum(const SaveData& d) const {
    // Simple additive checksum over the data fields (excluding the checksum itself)
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&d);
    uint32_t sum = 0;
    for (size_t i = 0; i < offsetof(SaveData, checksum); ++i) {
        sum += bytes[i];
    }
    return sum ^ 0xDEADBEEF;
}

bool SaveSystem::save(const SaveData& inData) {
    SaveData data = inData;
    data.checksum = computeChecksum(data);

    FILE* f = fopen(m_savePath.c_str(), "wb");
    if (!f) {
        LOGE("Cannot open save file for writing: %s", m_savePath.c_str());
        return false;
    }

    bool ok = (fwrite(&data, sizeof(SaveData), 1, f) == 1);
    fclose(f);

    if (ok) LOGI("Game saved to %s", m_savePath.c_str());
    return ok;
}

bool SaveSystem::load(SaveData& outData) {
    FILE* f = fopen(m_savePath.c_str(), "rb");
    if (!f) {
        LOGI("No save file found at %s", m_savePath.c_str());
        return false;
    }

    SaveData data{};
    bool ok = (fread(&data, sizeof(SaveData), 1, f) == 1);
    fclose(f);

    if (!ok) {
        LOGE("Failed to read save file");
        return false;
    }
    if (data.magic != 0x44454D4F) {
        LOGE("Save file magic mismatch");
        return false;
    }
    if (data.checksum != computeChecksum(data)) {
        LOGE("Save file checksum mismatch — corrupted?");
        return false;
    }

    outData = data;
    LOGI("Save loaded: room %u, pos(%.1f,%.1f)", data.currentRoom, data.playerX, data.playerY);
    return true;
}

bool SaveSystem::hasSave() const {
    FILE* f = fopen(m_savePath.c_str(), "rb");
    if (!f) return false;
    fclose(f);
    return true;
}

void SaveSystem::deleteSave() {
    remove(m_savePath.c_str());
    LOGI("Save deleted");
}
