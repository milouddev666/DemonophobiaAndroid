#pragma once
#include "../../engine/SceneManager.h"

class TitleScene : public Scene {
public:
    const char* name() const override { return "TitleScene"; }
    void onEnter(Engine* engine) override;
    void onExit() override;
    void update(Engine* engine, float dt) override;
    void render(Renderer& renderer) override;
    void onBackPressed(Engine* engine) override;

private:
    enum class MenuItem { NEW_GAME, CONTINUE, QUIT };
    MenuItem  m_selected = MenuItem::NEW_GAME;
    bool      m_hasSave  = false;
    float     m_blinkTimer = 0.0f;
    bool      m_blink    = true;
};
