#pragma once
#include <memory>

// Forward declarations
class SceneManager;
class AudioManager;
class TouchInput;
class VirtualGamepad;
class Renderer;
class SaveSystem;
class GameState;

/**
 * Engine
 * ------
 * Owns all subsystems. Runs on the thread created by android_native_app_glue
 * (which IS android_main's thread).
 *
 * Lifecycle with NativeActivity + Raylib:
 *   init()  → allocates window, audio, subsystems
 *   run()   → blocks, polling Raylib's event loop
 *   shutdown() → sets flag to break run() loop
 *
 * Pause/resume:
 *   Raylib's IsWindowFocused() returns false when app is backgrounded.
 *   We poll this inside run() — no separate JNI thread needed.
 */
class Engine {
public:
    Engine(int screenWidth, int screenHeight);
    ~Engine();

    bool init();
    void run();        // blocks until shutdown or window close
    void shutdown();

    void pause();      // called if needed externally
    void resume();
    void onTouchEvent(int action, int pointerId, float x, float y);
    void onBackPressed();

    SceneManager*   getSceneManager()  const { return m_sceneManager.get(); }
    AudioManager*   getAudioManager()  const { return m_audioManager.get(); }
    TouchInput*     getTouchInput()    const { return m_touchInput.get(); }
    VirtualGamepad* getGamepad()       const { return m_gamepad.get(); }
    Renderer*       getRenderer()      const { return m_renderer.get(); }
    SaveSystem*     getSaveSystem()    const { return m_saveSystem.get(); }
    GameState*      getGameState()     const { return m_gameState.get(); }

    int getScreenWidth()  const { return m_screenWidth; }
    int getScreenHeight() const { return m_screenHeight; }

    static constexpr int TARGET_FPS = 60;

private:
    void update(float dt);
    void render();

    int  m_screenWidth;
    int  m_screenHeight;
    bool m_shutdownRequested = false;
    bool m_paused            = false;

    std::unique_ptr<SceneManager>   m_sceneManager;
    std::unique_ptr<AudioManager>   m_audioManager;
    std::unique_ptr<TouchInput>     m_touchInput;
    std::unique_ptr<VirtualGamepad> m_gamepad;
    std::unique_ptr<Renderer>       m_renderer;
    std::unique_ptr<SaveSystem>     m_saveSystem;
    std::unique_ptr<GameState>      m_gameState;
};
