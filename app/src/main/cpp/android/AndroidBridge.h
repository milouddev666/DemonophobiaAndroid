#pragma once
#include <android/asset_manager.h>
#include <string>

// Forward declare to avoid including android_native_app_glue.h everywhere
struct android_app;

/**
 * AndroidBridge
 * -------------
 * Provides access to Android-specific resources.
 * Set once from android_main before Engine::init(); read-only afterwards.
 *
 * Unlike the JNI approach, we get the AAssetManager from the android_app
 * struct rather than from a JNI AssetManager jobject.
 */
class AndroidBridge {
public:
    static void               setApp(struct android_app* app);
    static struct android_app* getApp();

    static AAssetManager*     getAssetManager();
    static const std::string& getInternalPath();
    static const std::string& getExternalPath();

private:
    static struct android_app* s_app;
    static std::string         s_internalPath;
    static std::string         s_externalPath;
};
