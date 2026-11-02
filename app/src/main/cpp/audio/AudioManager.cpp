#include "AudioManager.h"
#include "../engine/AssetManager.h"
#include "../android/AndroidBridge.h"
#include <android/log.h>
#include <cstdio>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "Audio", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Audio", __VA_ARGS__)

void AudioManager::init() {
    // Raylib's InitAudioDevice() already called in Engine::init()
    LOGI("AudioManager ready");
}

void AudioManager::shutdown() {
    stopBGM();
    unloadAllSFX();
}

// ─── Asset caching ───────────────────────────────────────────────────────────

std::string AudioManager::cacheAsset(const std::string& assetPath) {
    // Write to internal storage so Raylib can load from filesystem
    const std::string& dataPath = AndroidBridge::getInternalPath();
    std::string filename = assetPath;
    // Replace / with _ to flatten the path
    for (char& c : filename) if (c == '/') c = '_';

    std::string dest = dataPath + "/audio_" + filename;

    // Only write if not already cached
    FILE* check = fopen(dest.c_str(), "rb");
    if (check) { fclose(check); return dest; }

    auto bytes = AssetManager::readBytes(assetPath);
    if (bytes.empty()) {
        LOGE("Failed to read audio asset: %s", assetPath.c_str());
        return "";
    }

    FILE* f = fopen(dest.c_str(), "wb");
    if (!f) {
        LOGE("Failed to write audio cache: %s", dest.c_str());
        return "";
    }
    fwrite(bytes.data(), 1, bytes.size(), f);
    fclose(f);
    return dest;
}

// ─── BGM ─────────────────────────────────────────────────────────────────────

void AudioManager::playBGM(const std::string& assetPath, bool loop) {
    stopBGM();

    std::string cached = cacheAsset(assetPath);
    if (cached.empty()) return;

    m_bgm = LoadMusicStream(cached.c_str());
    if (m_bgm.stream.buffer == nullptr) {
        LOGE("Failed to load BGM: %s", cached.c_str());
        return;
    }

    m_bgm.looping = loop;
    PlayMusicStream(m_bgm);
    m_bgmLoaded = true;
    m_bgmPaused = false;
    LOGI("Playing BGM: %s", assetPath.c_str());
}

void AudioManager::stopBGM() {
    if (m_bgmLoaded) {
        StopMusicStream(m_bgm);
        UnloadMusicStream(m_bgm);
        m_bgmLoaded = false;
    }
}

void AudioManager::pauseAll() {
    if (m_bgmLoaded && !m_bgmPaused) {
        PauseMusicStream(m_bgm);
        m_bgmPaused = true;
    }
}

void AudioManager::resumeAll() {
    if (m_bgmLoaded && m_bgmPaused) {
        ResumeMusicStream(m_bgm);
        m_bgmPaused = false;
    }
}

void AudioManager::setBGMVolume(float vol) {
    if (m_bgmLoaded) {
        SetMusicVolume(m_bgm, vol);
    }
}

void AudioManager::update() {
    if (m_bgmLoaded && !m_bgmPaused) {
        UpdateMusicStream(m_bgm);
    }
}

// ─── SFX ─────────────────────────────────────────────────────────────────────

bool AudioManager::loadSFX(const std::string& id, const std::string& assetPath) {
    auto it = m_sfx.find(id);
    if (it != m_sfx.end()) return true;  // already loaded

    std::string cached = cacheAsset(assetPath);
    if (cached.empty()) return false;

    Sound snd = LoadSound(cached.c_str());
    if (snd.stream.buffer == nullptr) {
        LOGE("Failed to load SFX: %s", cached.c_str());
        return false;
    }

    m_sfx[id] = snd;
    return true;
}

void AudioManager::playSFX(const std::string& id, float volume) {
    auto it = m_sfx.find(id);
    if (it == m_sfx.end()) return;
    SetSoundVolume(it->second, volume);
    PlaySound(it->second);
}

void AudioManager::unloadSFX(const std::string& id) {
    auto it = m_sfx.find(id);
    if (it != m_sfx.end()) {
        UnloadSound(it->second);
        m_sfx.erase(it);
    }
}

void AudioManager::unloadAllSFX() {
    for (auto& [id, snd] : m_sfx) {
        UnloadSound(snd);
    }
    m_sfx.clear();
}
