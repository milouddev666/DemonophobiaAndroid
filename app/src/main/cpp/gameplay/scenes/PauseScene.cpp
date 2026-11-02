#include "PauseScene.h"
#include "GameplayScene.h"
#include "TitleScene.h"
#include "../../engine/Engine.h"
#include "../../engine/SceneManager.h"
#include "../../input/VirtualGamepad.h"
#include "../../input/TouchInput.h"
#include "../../rendering/Renderer.h"
#include "../../audio/AudioManager.h"
#include <raylib.h>

PauseScene::PauseScene(uint32_t fromRoom) : m_fromRoom(fromRoom) {}

void PauseScene::onEnter(Engine* engine) {
    engine->getAudioManager()->pauseAll();
}

void PauseScene::onExit() {
    // audio resumed by whatever scene follows
}

void PauseScene::update(Engine* engine, float dt) {
    m_blinkTimer += dt;
    if (m_blinkTimer >= 0.5f) { m_blinkTimer = 0.0f; m_blink = !m_blink; }

    VirtualGamepad* pad   = engine->getGamepad();
    TouchInput*     touch = engine->getTouchInput();
    Renderer*       ren   = engine->getRenderer();

    // Navigation
    if (pad->isJustDown(VirtualGamepad::BTN_JUMP))   m_selected = (m_selected + 2) % 3;
    if (pad->isJustDown(VirtualGamepad::BTN_CROUCH)) m_selected = (m_selected + 1) % 3;

    // Touch
    static const float MENU_X = 220.0f, MENU_Y = 200.0f, STEP = 50.0f;
    for (int i = 0; i < TouchInput::MAX_POINTS; ++i) {
        const TouchPoint& tp = touch->getPoint(i);
        if (!tp.justPressed) continue;
        float vx = ren->screenToVirtualX(tp.x);
        float vy = ren->screenToVirtualY(tp.y);
        for (int z = 0; z < 3; ++z) {
            float zy = MENU_Y + STEP * z;
            if (vx >= MENU_X && vx <= MENU_X + 200.0f && vy >= zy - 15 && vy <= zy + 25) {
                if (m_selected == z) {
                    // confirm
                    switch (m_selected) {
                        case 0: // Resume
                            engine->getAudioManager()->resumeAll();
                            engine->getSceneManager()->changeScene(
                                std::make_unique<GameplayScene>(m_fromRoom));
                            return;
                        case 1: // Title
                            engine->getAudioManager()->stopBGM();
                            engine->getSceneManager()->changeSceneFade(
                                std::make_unique<TitleScene>(), 0.5f);
                            return;
                        case 2: // Quit
                            engine->shutdown();
                            return;
                    }
                }
                m_selected = z;
            }
        }
    }

    // Confirm
    if (pad->isJustDown(VirtualGamepad::BTN_ACTION)) {
        switch (m_selected) {
            case 0:
                engine->getAudioManager()->resumeAll();
                engine->getSceneManager()->changeScene(
                    std::make_unique<GameplayScene>(m_fromRoom));
                break;
            case 1:
                engine->getAudioManager()->stopBGM();
                engine->getSceneManager()->changeSceneFade(
                    std::make_unique<TitleScene>(), 0.5f);
                break;
            case 2:
                engine->shutdown();
                break;
        }
    }
}

void PauseScene::render(Renderer& /*renderer*/) {
    // Semi-transparent overlay
    DrawRectangle(0, 0, 640, 480, {0, 0, 0, 160});

    // Panel
    DrawRectangle(180, 160, 280, 180, {20, 20, 20, 220});
    DrawRectangleLinesEx({180.0f, 160.0f, 280.0f, 180.0f}, 2.0f, {140, 140, 140, 255});

    int pw = MeasureText("PAUSED", 28);
    DrawText("PAUSED", 320 - pw/2, 170, 28, {200, 200, 200, 255});

    const char* labels[] = {"RESUME", "TITLE SCREEN", "QUIT"};
    static const float MENU_Y = 215.0f, STEP = 42.0f;
    for (int i = 0; i < 3; ++i) {
        float iy = MENU_Y + STEP * i;
        Color tc = (m_selected == i) ? Color{255, 220, 100, 255}
                                     : Color{160, 160, 160, 255};
        int lw = MeasureText(labels[i], 18);
        DrawText(labels[i], 320 - lw/2, static_cast<int>(iy), 18, tc);
        if (m_selected == i && m_blink) {
            DrawText(">", 320 - lw/2 - 18, static_cast<int>(iy), 18, {255,180,0,255});
        }
    }
}

void PauseScene::onBackPressed(Engine* engine) {
    // Back again = resume
    engine->getAudioManager()->resumeAll();
    engine->getSceneManager()->changeScene(
        std::make_unique<GameplayScene>(m_fromRoom));
}
