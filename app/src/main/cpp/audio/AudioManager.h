#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>

/**
 * AudioManager
 * ------------
 * Wraps Raylib's audio system.
 * BGM streams from file; SFX are preloaded into memory.
 *
 * All paths are relative to APK assets/ folder.
 * Raylib on Android can load audio from the asset manager directly
 * when given an asset:// scheme or a direct path; we write the asset
 * to a temp file and load from there (most reliable approach on Android).
 */
class AudioManager {
public:
    void init();
    void shutdown();

    // ─── BGM ──────────────────────────────────────────────────────────────
    /** Load and immediately start playing background music. */
    void playBGM(const std::string& assetPath, bool loop = true);
    void stopBGM();
    void pauseAll();
    void resumeAll();
    void setBGMVolume(float vol);  // 0.0–1.0

    /** Must be called every frame to stream BGM. */
    void update();

    // ─── SFX ──────────────────────────────────────────────────────────────
    /** Pre-load a sound effect (call once at room load). */
    bool loadSFX(const std::string& id, const std::string& assetPath);
    void playSFX(const std::string& id, float volume = 1.0f);
    void unloadSFX(const std::string& id);
    void unloadAllSFX();

private:
    /** Copy asset to internal storage temp file; return path. */
    static std::string cacheAsset(const std::string& assetPath);

    Music m_bgm{};
    bool  m_bgmLoaded  = false;
    bool  m_bgmPaused  = false;

    std::unordered_map<std::string, Sound> m_sfx;
};
