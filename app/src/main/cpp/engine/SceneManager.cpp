#include "SceneManager.h"
#include "Engine.h"
#include "../rendering/Renderer.h"
#include "../gameplay/GameState.h"

// Concrete scenes
#include "../gameplay/scenes/TitleScene.h"
#include "../gameplay/scenes/GameplayScene.h"
#include "../gameplay/scenes/PauseScene.h"
#include "../gameplay/scenes/GameOverScene.h"

#include <raylib.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "SceneMgr", __VA_ARGS__)

SceneManager::SceneManager(Engine* engine) : m_engine(engine) {}
SceneManager::~SceneManager() = default;

void SceneManager::init() {
    // Start with title screen
    changeScene(std::make_unique<TitleScene>());
}

void SceneManager::update(float dt) {
    updateFade(dt);

    if (m_current) {
        m_current->update(m_engine, dt);
    }
}

void SceneManager::render(Renderer& renderer) {
    if (m_current) {
        m_current->render(renderer);
    }

    // Draw fade overlay
    if (m_fadeDir != FadeDir::NONE && m_fadeAlpha > 0.0f) {
        unsigned char a = static_cast<unsigned char>(m_fadeAlpha * 255.0f);
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0,0,0,a});
    }
}

void SceneManager::onBackPressed() {
    if (m_current) {
        m_current->onBackPressed(m_engine);
    }
}

void SceneManager::changeScene(std::unique_ptr<Scene> next) {
    if (m_current) m_current->onExit();
    m_current = std::move(next);
    if (m_current) {
        LOGI("Enter scene: %s", m_current->name());
        m_current->onEnter(m_engine);
    }
    m_fadeDir = FadeDir::NONE;
    m_fadeAlpha = 0.0f;
}

void SceneManager::changeSceneFade(std::unique_ptr<Scene> next, float fadeDuration) {
    m_pending   = std::move(next);
    m_fadeDur   = fadeDuration;
    m_fadeTimer = 0.0f;
    m_fadeDir   = FadeDir::OUT;  // fade to black first
}

void SceneManager::updateFade(float dt) {
    if (m_fadeDir == FadeDir::NONE) return;

    m_fadeTimer += dt;
    float t = m_fadeTimer / (m_fadeDur * 0.5f);
    if (t > 1.0f) t = 1.0f;

    if (m_fadeDir == FadeDir::OUT) {
        m_fadeAlpha = t;
        if (t >= 1.0f) {
            // Switch scenes at peak black
            doSceneChange();
            m_fadeDir   = FadeDir::IN;
            m_fadeTimer = 0.0f;
        }
    } else if (m_fadeDir == FadeDir::IN) {
        m_fadeAlpha = 1.0f - t;
        if (t >= 1.0f) {
            m_fadeDir   = FadeDir::NONE;
            m_fadeAlpha = 0.0f;
        }
    }
}

void SceneManager::doSceneChange() {
    if (m_current) m_current->onExit();
    m_current = std::move(m_pending);
    if (m_current) {
        LOGI("Enter scene (fade): %s", m_current->name());
        m_current->onEnter(m_engine);
    }
}
