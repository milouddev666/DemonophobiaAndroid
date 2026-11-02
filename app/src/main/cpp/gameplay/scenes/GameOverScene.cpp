#include "GameOverScene.h"
#include "GameplayScene.h"
#include "TitleScene.h"
#include "../../engine/Engine.h"
#include "../../engine/SceneManager.h"
#include "../../engine/SaveSystem.h"
#include "../../gameplay/GameState.h"
#include "../../rendering/Renderer.h"
#include "../../input/VirtualGamepad.h"
#include "../../input/TouchInput.h"
#include "../../audio/AudioManager.h"
#include <raylib.h>

GameOverScene::GameOverScene(uint32_t fromRoom) : m_fromRoom(fromRoom) {}

void GameOverScene::onEnter(Engine* engine) {
    m_selected   = 0;
    m_blinkTimer = 0.0f;
    m_blink      = true;
    m_fadeIn     = 0.0f;
    engine->getAudioManager()->stopBGM();
    // [PLACEHOLDER] Play death-screen BGM if present
    // engine->getAudioManager()->playBGM("audio/gameover.ogg", false);
}

void GameOverScene::onExit() {}

void GameOverScene::update(Engine* engine, float dt) {
    m_fadeIn = (m_fadeIn < 1.0f) ? m_fadeIn + dt / 0.8f : 1.0f;
    m_blinkTimer += dt;
    if (m_blinkTimer >= 0.5f) { m_blinkTimer = 0.0f; m_blink = !m_blink; }

    if (m_fadeIn < 0.6f) return;   // block input during fade-in

    VirtualGamepad* pad   = engine->getGamepad();
    TouchInput*     touch = engine->getTouchInput();
    Renderer*       ren   = engine->getRenderer();

    if (pad->isJustDown(VirtualGamepad::BTN_LEFT)  ||
        pad->isJustDown(VirtualGamepad::BTN_JUMP))   m_selected = 0;
    if (pad->isJustDown(VirtualGamepad::BTN_RIGHT) ||
        pad->isJustDown(VirtualGamepad::BTN_CROUCH)) m_selected = 1;

    // Touch
    static const float BTN_Y = 320.0f, BTN_W = 120.0f, BTN_H = 36.0f;
    static const float BTN_TRY  = 110.0f, BTN_TITLE = 410.0f;
    for (int i = 0; i < TouchInput::MAX_POINTS; ++i) {
        const TouchPoint& tp = touch->getPoint(i);
        if (!tp.justPressed) continue;
        float vx = ren->screenToVirtualX(tp.x);
        float vy = ren->screenToVirtualY(tp.y);
        bool inTry   = vx>=BTN_TRY   && vx<=BTN_TRY+BTN_W   && vy>=BTN_Y && vy<=BTN_Y+BTN_H;
        bool inTitle = vx>=BTN_TITLE  && vx<=BTN_TITLE+BTN_W && vy>=BTN_Y && vy<=BTN_Y+BTN_H;
        if (inTry) {
            if (m_selected == 0) { engine->getSceneManager()->changeSceneFade(
                std::make_unique<GameplayScene>(m_fromRoom), 0.5f); return; }
            m_selected = 0;
        }
        if (inTitle) {
            if (m_selected == 1) { engine->getSceneManager()->changeSceneFade(
                std::make_unique<TitleScene>(), 0.5f); return; }
            m_selected = 1;
        }
    }

    if (pad->isJustDown(VirtualGamepad::BTN_ACTION)) {
        if (m_selected == 0)
            engine->getSceneManager()->changeSceneFade(
                std::make_unique<GameplayScene>(m_fromRoom), 0.5f);
        else
            engine->getSceneManager()->changeSceneFade(
                std::make_unique<TitleScene>(), 0.5f);
    }
}

void GameOverScene::render(Renderer& /*renderer*/) {
    unsigned char fade = static_cast<unsigned char>(m_fadeIn * 255.0f);

    DrawRectangle(0, 0, 640, 480, {0, 0, 0, 255});

    // [PROTOTYPE] Text-only game over. Replace with original death screen art.
    const char* msg = "YOU DIED";
    int mw = MeasureText(msg, 48);
    DrawText(msg, 320 - mw/2, 160, 48, {fade, 0, 0, fade});

    GameState* gs = nullptr; // accessed via engine in update; use death count from HUD
    const char* sub = "[PROTOTYPE - not original Demonophobia content]";
    int sw = MeasureText(sub, 8);
    DrawText(sub, 320-sw/2, 215, 8, {80,80,80, fade});

    if (m_fadeIn >= 0.6f) {
        const char* labels[] = {"TRY AGAIN", "TITLE SCREEN"};
        float bx[] = {110.0f, 410.0f};
        for (int i = 0; i < 2; ++i) {
            Color tc = (m_selected == i && m_blink)
                        ? Color{255,220,100,fade} : Color{160,160,160,fade};
            int lw = MeasureText(labels[i], 18);
            DrawText(labels[i], static_cast<int>(bx[i]) + 60 - lw/2,
                     323, 18, tc);
            Color bc = (m_selected == i) ? Color{100,80,20,fade}
                                         : Color{40,40,40,fade};
            DrawRectangleLinesEx({bx[i], 320.0f, 120.0f, 36.0f}, 2.0f, bc);
        }
    }
}

void GameOverScene::onBackPressed(Engine* engine) {
    engine->getSceneManager()->changeSceneFade(
        std::make_unique<GameplayScene>(m_fromRoom), 0.5f);
}
