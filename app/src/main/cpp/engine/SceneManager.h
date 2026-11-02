#pragma once
#include <memory>
#include <functional>
#include <string>

class Engine;
class Renderer;

/**
 * Scene
 * -----
 * Abstract base for all game scenes: Title, Gameplay, PauseMenu, GameOver, etc.
 */
class Scene {
public:
    virtual ~Scene() = default;
    virtual void onEnter(Engine* engine) = 0;
    virtual void onExit()  = 0;
    virtual void update(Engine* engine, float dt) = 0;
    virtual void render(Renderer& renderer) = 0;
    virtual void onBackPressed(Engine* engine) { (void)engine; }
    virtual const char* name() const = 0;
};

/**
 * SceneManager
 * ------------
 * Single active scene + optional fade transition.
 */
class SceneManager {
public:
    enum class FadeDir { IN, OUT, NONE };

    explicit SceneManager(Engine* engine);
    ~SceneManager();

    void init();
    void update(float dt);
    void render(Renderer& renderer);
    void onBackPressed();

    /** Replace current scene immediately (no fade). */
    void changeScene(std::unique_ptr<Scene> next);

    /** Replace with a crossfade transition (seconds). */
    void changeSceneFade(std::unique_ptr<Scene> next, float fadeDuration = 0.5f);

    Scene* currentScene() const { return m_current.get(); }

private:
    Engine* m_engine;

    std::unique_ptr<Scene> m_current;
    std::unique_ptr<Scene> m_pending;   // queued during fade-out

    FadeDir m_fadeDir   = FadeDir::NONE;
    float   m_fadeTimer = 0.0f;
    float   m_fadeDur   = 0.5f;
    float   m_fadeAlpha = 0.0f;   // 0=transparent 1=black

    void doSceneChange();
    void updateFade(float dt);
};
