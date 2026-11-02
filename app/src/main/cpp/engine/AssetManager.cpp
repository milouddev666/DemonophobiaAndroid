#include "AssetManager.h"
#include "../android/AndroidBridge.h"
#include <android/asset_manager.h>
#include <android/log.h>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "AssetMgr", __VA_ARGS__)

std::vector<uint8_t> AssetManager::readBytes(const std::string& path) {
    AAssetManager* mgr = AndroidBridge::getAssetManager();
    if (!mgr) return {};

    AAsset* asset = AAssetManager_open(mgr, path.c_str(), AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("Asset not found: %s", path.c_str());
        return {};
    }

    off_t size = AAsset_getLength(asset);
    std::vector<uint8_t> buf(static_cast<size_t>(size));
    AAsset_read(asset, buf.data(), static_cast<size_t>(size));
    AAsset_close(asset);
    return buf;
}

std::string AssetManager::readText(const std::string& path) {
    auto bytes = readBytes(path);
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

bool AssetManager::exists(const std::string& path) {
    AAssetManager* mgr = AndroidBridge::getAssetManager();
    if (!mgr) return false;
    AAsset* asset = AAssetManager_open(mgr, path.c_str(), AASSET_MODE_UNKNOWN);
    if (!asset) return false;
    AAsset_close(asset);
    return true;
}

std::vector<std::string> AssetManager::listDir(const std::string& dirPath) {
    AAssetManager* mgr = AndroidBridge::getAssetManager();
    if (!mgr) return {};

    AAssetDir* dir = AAssetManager_openDir(mgr, dirPath.c_str());
    if (!dir) return {};

    std::vector<std::string> result;
    const char* name;
    while ((name = AAssetDir_getNextFileName(dir)) != nullptr) {
        result.push_back(dirPath + "/" + name);
    }
    AAssetDir_close(dir);
    return result;
}
