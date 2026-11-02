/**
 * TitleScene.cpp
 * --------------
 * [PROTOTYPE ASSET] Title screen rendered with Raylib primitives and text.
 * When original game assets are supplied, replace with original title art.
 *
 * Controls:
 *   Virtual gamepad: UP/DOWN to select, A to confirm, touch to tap directly.
 *
 * NOTE: This is NOT the original Demonophobia title screen.
 * It is a functional navigation screen to reach the gameplay prototype.
 */

#include "TitleScene.h"
#include "GameplayScene.h"
#include "../../engine/Engine.h"
#include "../../engine/SceneManager.h"
#include "../../engine/SaveSystem.h"
#include "../../gameplay/GameState.h"
#include "../../rendering/Renderer.h"
#include "../../input/VirtualGamepad.h"
#include "../../input/TouchInput.h"
#include "../../audio/AudioManager.h"

#include <raylib.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "TitleScene", __VA_ARGS__)

// ─── Layout constants (virtual 640×480 coords) ───────────────────────────────
static constexpr float TITLE_Y      = 100.0f;
static constexpr float MENU_START_Y = 230.0f;
static constexpr float MENU_STEP_Y  =  50.0f;
static constexpr float MENU_CX      = 320.0f;

// ─── Touch-tappable menu zones ────────────────────────────────────────────────
struct MenuZone { float x, y, w, h; };
static const MenuZone k_zones[] = {
    {170.0f, MENU_START_Y - 20.0f,           300.0f, 40.0f},  // NEW GAME
    {170.0f, MENU_START_Y + MENU_STEP_Y - 20.0f, 300.0f, 40.0f},  // CONTINUE
    {170.0f, MENU_START_Y + MENU_STEP_Y * 2 - 20.0f, 300.0f, 40.0f},  // QUIT
};

static bool touchInZone(const MenuZone& z, float tx, float ty) {
    return tx >= z.x && tx <= z.x + z.w && ty >= z.y && ty <= z.y + z.h;
}

// ─── Scene implementation ─────────────────────────────────────────────────────

void TitleScene::onEnter(Engine* engine) {
    m_hasSave = engine->getSaveSystem()->hasSave();
    m_selected = MenuItem::NEW_GAME;
    m_blinkTimer = 0.0f;
    m_blink = true;
    LOGI("TitleScene entered (hasSave=%d)", m_hasSave);

    // [PLACEHOLDER] Play title BGM if available
    // engine->getAudioManager()->playBGM("audio/title.ogg");
}

void TitleScene::onExit() {
    // engine->getAudioManager()->stopBGM(); — called from GameplayScene::onEnter
}

void TitleScene::update(Engine* engine, float dt) {
    m_blinkTimer += dt;
    if (m_blinkTimer >= 0.5f) {
        m_blinkTimer = 0.0f;
        m_blink = !m_blink;
    }

    VirtualGamepad* pad = engine->getGamepad();
    TouchInput*     touch = engine->getTouchInput();
    Renderer*       renderer = engine->getRenderer();

    // ── D-pad navigation ───────────────────────────────────────────────────
    if (pad->isJustDown(VirtualGamepad::BTN_JUMP)) {  // UP
        int s = static_cast<int>(m_selected);
        s = (s + 2) % 3;  // wrap up
        m_selected = static_cast<MenuItem>(s);
    }
    if (pad->isJustDown(VirtualGamepad::BTN_CROUCH)) { // DOWN
        int s = static_cast<int>(m_selected);
        s = (s + 1) % 3;
        m_selected = static_cast<MenuItem>(s);
    }

    // ── Confirm selection ──────────────────────────────────────────────────
    bool confirm = pad->isJustDown(VirtualGamepad::BTN_ACTION);

    // ── Direct touch on menu items ─────────────────────────────────────────
    // Use justPressed to avoid double-fire
    for (int i = 0; i < TouchInput::MAX_POINTS; ++i) {
        const TouchPoint& tp = touch->getPoint(i);
        if (!tp.justPressed) continue;

        // Convert screen → virtual coords
        float vx = renderer->screenToVirtualX(tp.x);
        float vy = renderer->screenToVirtualY(tp.y);

        for (int z = 0; z < 3; ++z) {
            if (touchInZone(k_zones[z], vx, vy)) {
                int prevSel = static_cast<int>(m_selected);
                m_selected = static_cast<MenuItem>(z);
                if (prevSel == z) {
                    confirm = true;  // second tap confirms
                }
            }
        }
    }

    // ── Execute selection ──────────────────────────────────────────────────
    if (confirm) {
        switch (m_selected) {
            case MenuItem::NEW_GAME: {
                engine->getGameState()->reset();
                engine->getSaveSystem()->deleteSave();
                LOGI("Starting new game");
                engine->getSceneManager()->changeSceneFade(
                    std::make_unique<GameplayScene>(0), 0.6f);
                break;
            }
            case MenuItem::CONTINUE: {
                if (m_hasSave) {
                    SaveData sd;
                    if (engine->getSaveSystem()->load(sd)) {
                        GameState* gs = engine->getGameState();
                        gs->currentRoom  = sd.currentRoom;
                        gs->respawnX     = sd.playerX;
                        gs->respawnY     = sd.playerY;
                        gs->roomVisited  = 0;
                        gs->doorOpened   = sd.roomFlags;
                        gs->deathCount   = static_cast<int>(sd.healthFlags >> 1);
                        for (int i = 0; i < GameState::INVENTORY_SIZE; ++i) {
                            gs->inventory[i] = (sd.itemFlags >> (i * 4)) & 0xF;
                        }
                        LOGI("Continuing from room %u", sd.currentRoom);
                        engine->getSceneManager()->changeSceneFade(
                            std::make_unique<GameplayScene>(sd.currentRoom), 0.6f);
                    }
                }
                break;
            }
            case MenuItem::QUIT:
                LOGI("Quit requested");
                engine->shutdown();
                break;
        }
    }
}

void TitleScene::render(Renderer& /*renderer*/) {
    // ── Background ──────────────────────────────────────────────────────────
    // [PROTOTYPE] Solid dark background.
    // Replace with: DrawTexture(m_titleBg, 0, 0, WHITE);
    DrawRectangle(0, 0, 640, 480, {8, 4, 12, 255});

    // Decorative blood-red vignette
    for (int i = 0; i < 20; ++i) {
        unsigned char a = static_cast<unsigned char>(i * 3);
        DrawRectangle(i, i, 640 - i*2, 480 - i*2, {80, 0, 0, a});
    }

    // ── Title text ──────────────────────────────────────────────────────────
    // [PROTOTYPE] Text rendering. Replace with original title logo sprite.
    const char* titleLine1 = "DEMONOPHOBIA";
    const char* titleLine2 = "Android Port";
    const char* notice     = "[PROTOTYPE - Original assets not loaded]";

    int t1w = MeasureText(titleLine1, 36);
    int t2w = MeasureText(titleLine2, 16);
    int nw  = MeasureText(notice, 8);

    DrawText(titleLine1, static_cast<int>(MENU_CX) - t1w/2,
             static_cast<int>(TITLE_Y), 36, {180, 20, 20, 255});
    DrawText(titleLine2, static_cast<int>(MENU_CX) - t2w/2,
             static_cast<int>(TITLE_Y) + 44, 16, {140, 140, 140, 255});
    DrawText(notice,     static_cast<int>(MENU_CX) - nw/2,
             460, 8, {80, 80, 80, 255});

    // ── Menu items ──────────────────────────────────────────────────────────
    const char* labels[] = {"NEW GAME", "CONTINUE", "QUIT"};
    for (int i = 0; i < 3; ++i) {
        float iy = MENU_START_Y + MENU_STEP_Y * i;
        bool  sel = (static_cast<int>(m_selected) == i);
        bool  grey = (i == 1 && !m_hasSave);  // CONTINUE greyed out without save

        Color tc = grey ? Color{60, 60, 60, 255}
                 : sel  ? Color{255, 220, 220, 255}
                 :        Color{160, 160, 160, 255};

        int lw = MeasureText(labels[i], 24);
        DrawText(labels[i], static_cast<int>(MENU_CX) - lw/2,
                 static_cast<int>(iy), 24, tc);

        // Selection indicator
        if (sel && m_blink && !grey) {
            DrawText(">",
                     static_cast<int>(MENU_CX) - lw/2 - 20,
                     static_cast<int>(iy), 24, {255, 80, 80, 255});
        }
    }

    // ── Touch hint ──────────────────────────────────────────────────────────
    if (m_blink) {
        const char* hint = "Tap a menu item to select, tap again to confirm";
        int hw = MeasureText(hint, 9);
        DrawText(hint, 320 - hw/2, 430, 9, {100, 100, 100, 200});
    }
}

void TitleScene::onBackPressed(Engine* engine) {
    // Back on title = quit
    engine->shutdown();
}
