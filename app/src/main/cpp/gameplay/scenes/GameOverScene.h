#pragma once
#include "../../engine/SceneManager.h"
#include <cstdint>

/**
 * GameOverScene
 * Shown after the player dies and the death animation finishes.
 * Options: Try Again (respawn at room entry) | Title Screen
 */
class GameOverScene : public Scene {
public:
    explicit GameOverScene(uint32_t fromRoom);

    const char* name() const override { return "GameOverScene"; }
    void onEnter(Engine* engine) override;
    void onExit()                override;
    void update(Engine* engine, float dt) override;
    void render(Renderer& renderer)       override;
    void onBackPressed(Engine* engine)    override;

private:
    uint32_t m_fromRoom;
    int      m_selected    = 0;  // 0=TryAgain 1=Title
    float    m_blinkTimer  = 0.0f;
    bool     m_blink       = true;
    float    m_fadeIn      = 0.0f;   // 0→1 over 0.8s
};
