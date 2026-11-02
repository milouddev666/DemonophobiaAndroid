#pragma once
#include "../../engine/SceneManager.h"
#include <cstdint>

class PauseScene : public Scene {
public:
    explicit PauseScene(uint32_t fromRoom);

    const char* name() const override { return "PauseScene"; }
    void onEnter(Engine* engine) override;
    void onExit()                override;
    void update(Engine* engine, float dt) override;
    void render(Renderer& renderer)       override;
    void onBackPressed(Engine* engine)    override;

private:
    uint32_t m_fromRoom;
    int      m_selected  = 0;
    float    m_blinkTimer = 0.0f;
    bool     m_blink     = true;
};
