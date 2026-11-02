#pragma once
#include <string>
#include <vector>
#include <cstdint>

/**
 * AssetManager
 * ------------
 * Wraps AAssetManager to read files from the APK assets/ folder.
 * Raylib on Android is configured to use the same AAssetManager, but we
 * expose our own interface for raw binary and text reads (map data, JSON, etc.)
 */
class AssetManager {
public:
    /**
     * Read entire file into a byte buffer.
     * Returns empty vector on failure.
     */
    static std::vector<uint8_t> readBytes(const std::string& path);

    /**
     * Read entire text file as a string.
     */
    static std::string readText(const std::string& path);

    /**
     * Check whether a path exists in assets.
     */
    static bool exists(const std::string& path);

    /**
     * List files in an asset directory.
     * Returned paths are relative to assets/.
     */
    static std::vector<std::string> listDir(const std::string& dirPath);
};
