#include "AndroidBridge.h"
#include <android_native_app_glue.h>
#include <android/log.h>
#include <cstring>

// Global app pointer — set before Engine::init()
struct android_app* g_androidApp = nullptr;

// Static members
struct android_app* AndroidBridge::s_app          = nullptr;
std::string         AndroidBridge::s_internalPath;
std::string         AndroidBridge::s_externalPath;

void AndroidBridge::setApp(struct android_app* app) {
    s_app = app;
    g_androidApp = app;   // also set the global for Raylib

    if (!app) return;

    // Internal storage: app->activity->internalDataPath
    if (app->activity && app->activity->internalDataPath) {
        s_internalPath = app->activity->internalDataPath;
    }
    // External storage: app->activity->externalDataPath (may be null)
    if (app->activity && app->activity->externalDataPath) {
        s_externalPath = app->activity->externalDataPath;
    } else {
        s_externalPath = s_internalPath;
    }

    __android_log_print(ANDROID_LOG_INFO, "Bridge",
        "Paths: int=%s ext=%s",
        s_internalPath.c_str(), s_externalPath.c_str());
}

struct android_app* AndroidBridge::getApp() {
    return s_app;
}

AAssetManager* AndroidBridge::getAssetManager() {
    if (!s_app || !s_app->activity) return nullptr;
    return s_app->activity->assetManager;
}

const std::string& AndroidBridge::getInternalPath() {
    return s_internalPath;
}

const std::string& AndroidBridge::getExternalPath() {
    return s_externalPath;
}
