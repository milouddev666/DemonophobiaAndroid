#include <android_native_app_glue.h>
#include <android/log.h>
#include <raylib.h>

#include "engine/Engine.h"
#include "android/AndroidBridge.h"

#define LOG_TAG "Demonophobia"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Provided by Raylib's Android backend.
extern "C" struct android_app* GetAndroidApp(void);

int main(int argc, char* argv[]) {
    LOGI("Demonophobia startup");

    struct android_app* app = GetAndroidApp();

    if (!app) {
        LOGE("GetAndroidApp returned null");
        return 1;
    }

    AndroidBridge::setApp(app);

    Engine engine(0, 0);

    if (!engine.init()) {
        LOGE("Engine initialization failed");
        return 1;
    }

    LOGI("Engine started: %dx%d",
         engine.getScreenWidth(),
         engine.getScreenHeight());

    engine.run();

    LOGI("Demonophobia shutdown");
    return 0;
}
