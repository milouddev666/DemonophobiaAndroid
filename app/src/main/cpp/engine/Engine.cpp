#include "Engine.h"
#include "SceneManager.h"
#include "SaveSystem.h"
#include "../audio/AudioManager.h"
#include "../input/TouchInput.h"
#include "../input/VirtualGamepad.h"
#include "../rendering/Renderer.h"
#include "../gameplay/GameState.h"
#include "../android/AndroidBridge.h"

#include <raylib.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "Engine", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Engine", __VA_ARGS__)

Engine::Engine(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{}

Engine::~Engine() = default;

bool Engine::init() {
    LOGI("Engine::init");

    // On Android with NativeActivity, InitWindow uses the app's NativeWindow.
    // Pass 0,0 — Raylib will use the actual display size on Android.
    InitWindow(0, 0, "DEMONOPHOBIA");
    SetTargetFPS(TARGET_FPS);

    if (!IsWindowReady()) {
        LOGE("Raylib window not ready");
        return false;
    }

    // Get actual screen dimensions after window init
    m_screenWidth  = GetScreenWidth();
    m_screenHeight = GetScreenHeight();
    LOGI("Screen size: %dx%d", m_screenWidth, m_screenHeight);

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        LOGE("Audio device not ready — continuing without audio");
    }

    // Construct and initialise subsystems
    m_touchInput   = std::make_unique<TouchInput>(m_screenWidth, m_screenHeight);
    m_gamepad      = std::make_unique<VirtualGamepad>(m_screenWidth, m_screenHeight);
    m_renderer     = std::make_unique<Renderer>(m_screenWidth, m_screenHeight);
    m_audioManager = std::make_unique<AudioManager>();
    m_saveSystem   = std::make_unique<SaveSystem>();
    m_gameState    = std::make_unique<GameState>();
    m_sceneManager = std::make_unique<SceneManager>(this);

    m_renderer->init();
    m_audioManager->init();
    m_saveSystem->init();
    m_gameState->init();
    m_sceneManager->init();

    LOGI("Engine::init complete");
    return true;
}

void Engine::run() {
    LOGI("Engine::run — game loop starting");

    while (!m_shutdownRequested && !WindowShouldClose()) {

        // Raylib handles pause/resume via focus events on Android.
        // IsWindowFocused() returns false when the app is in background.
        if (!IsWindowFocused()) {
            if (!m_paused) {
                m_paused = true;
                m_audioManager->pauseAll();
                LOGI("App backgrounded — paused");
            }
            WaitTime(0.033);  // ~30Hz idle when paused
            continue;
        }

        if (m_paused) {
            m_paused = false;
            m_audioManager->resumeAll();
            LOGI("App foregrounded — resumed");
        }

        float dt = GetFrameTime();
        if (dt > 0.05f)  dt = 0.05f;
        if (dt <= 0.0f)  dt = 1.0f / TARGET_FPS;

        update(dt);
        render();
    }

    LOGI("Engine::run — game loop ended");
    CloseAudioDevice();
    CloseWindow();
}

void Engine::update(float dt) {
    m_touchInput->update();           // gather Raylib touch state
    m_gamepad->update(*m_touchInput); // map to virtual buttons
    m_audioManager->update();         // stream BGM (MUST be called every frame)
    m_sceneManager->update(dt);
}

void Engine::render() {
    BeginDrawing();
    ClearBackground(BLACK);

    // Game world rendered into 640×480 virtual canvas, then letterboxed
    m_renderer->beginFrame();
    m_sceneManager->render(*m_renderer);
    m_renderer->endFrame();

    // Virtual gamepad rendered in screen space (on top of letterbox)
    m_gamepad->render();

    // Debug FPS
#ifdef DEBUG_BUILD
    DrawFPS(4, 4);
#endif

    EndDrawing();
}

void Engine::shutdown() {
    m_shutdownRequested = true;
}

// These remain for compatibility; with NativeActivity they're rarely needed
void Engine::pause()  { m_paused = true;  m_audioManager->pauseAll(); }
void Engine::resume() { m_paused = false; m_audioManager->resumeAll(); }

void Engine::onTouchEvent(int, int, float, float) {
    // Not used: NativeActivity approach reads touch via Raylib's GetTouch* APIs
}

void Engine::onBackPressed() {
    if (m_sceneManager) {
        m_sceneManager->onBackPressed();
    }
}
