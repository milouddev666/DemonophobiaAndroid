/**
 * main.cpp — android_main() entry point for Raylib NativeActivity port.
 *
 * Raylib's Android backend hooks into android_native_app_glue which calls
 * android_main() on a dedicated thread. Raylib's InitWindow() internally
 * waits for the native window and sets up OpenGL ES.
 *
 * DO NOT add JNI exports here. With NativeActivity, there is no Java/Kotlin
 * Activity — the system routes events directly to android_main.
 */

#include <android_native_app_glue.h>
#include <android/log.h>
#include <raylib.h>

#include "engine/Engine.h"
#include "android/AndroidBridge.h"

#define LOG_TAG "Demonophobia"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Required by android_native_app_glue — must be defined in exactly one TU.
// This symbol is referenced by the glue code's ANativeActivity_onCreate.
void android_main(struct android_app* app) {
    LOGI("android_main: startup");

    // Store app pointer so subsystems can reach the AAssetManager and paths.
    AndroidBridge::setApp(app);

    // Engine uses 0,0 → Raylib will use actual display size on Android.
    Engine engine(0, 0);

    if (!engine.init()) {
        LOGE("Engine::init failed — aborting");
        return;
    }

    LOGI("Engine started: %dx%d", engine.getScreenWidth(), engine.getScreenHeight());
    engine.run();   // blocks until window close or back-press shutdown
    LOGI("android_main: clean exit");
}
