#include "SpriteSheet.h"
#include "../engine/AssetManager.h"
#include <android/log.h>
#include <cstring>
#include <algorithm>

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "Sprite", __VA_ARGS__)

SpriteSheet::~SpriteSheet() {
    unload();
}

bool SpriteSheet::load(const std::string& path, int frameW, int frameH) {
    // Read raw bytes from APK assets
    auto bytes = AssetManager::readBytes(path);
    if (bytes.empty()) {
        LOGE("Failed to load sprite: %s", path.c_str());
        return false;
    }

    // Determine file extension for Raylib's image loader
    Image img = LoadImageFromMemory(
        ".png",  // treat all as PNG; adjust if needed
        bytes.data(),
        static_cast<int>(bytes.size())
    );

    if (img.data == nullptr) {
        LOGE("Failed to decode image: %s", path.c_str());
        return false;
    }

    m_texture = LoadTextureFromImage(img);
    UnloadImage(img);

    m_frameW = frameW;
    m_frameH = frameH;
    m_cols   = m_texture.width / frameW;
    if (m_cols < 1) m_cols = 1;

    m_loaded = true;
    return true;
}

void SpriteSheet::unload() {
    if (m_loaded) {
        UnloadTexture(m_texture);
        m_loaded = false;
    }
}

void SpriteSheet::addAnimation(const Animation& anim) {
    m_animations.push_back(anim);
}

void SpriteSheet::setAnimation(const std::string& name) {
    for (int i = 0; i < static_cast<int>(m_animations.size()); ++i) {
        if (m_animations[i].name == name) {
            if (m_currentAnim == i) return;  // already playing
            m_currentAnim  = i;
            m_currentFrame = m_animations[i].startFrame;
            m_frameTimer   = 0.0f;
            m_animDone     = false;
            return;
        }
    }
    LOGE("Animation not found: %s", name.c_str());
}

void SpriteSheet::update(float dt) {
    if (m_currentAnim < 0 || m_animDone) return;

    const Animation& anim = m_animations[m_currentAnim];
    m_frameTimer += dt;

    float frameDur = 1.0f / anim.fps;
    while (m_frameTimer >= frameDur) {
        m_frameTimer -= frameDur;

        int localFrame = m_currentFrame - anim.startFrame + 1;
        if (localFrame >= anim.frameCount) {
            if (anim.loop) {
                m_currentFrame = anim.startFrame;
            } else {
                m_animDone = true;
                return;
            }
        } else {
            m_currentFrame++;
        }
    }
}

bool SpriteSheet::isAnimationDone() const {
    return m_animDone;
}

Rectangle SpriteSheet::frameRect(int absFrame) const {
    int col = absFrame % m_cols;
    int row = absFrame / m_cols;
    return {
        static_cast<float>(col * m_frameW),
        static_cast<float>(row * m_frameH),
        static_cast<float>(m_frameW),
        static_cast<float>(m_frameH)
    };
}

void SpriteSheet::draw(float x, float y, bool flipX, Color tint) const {
    drawFrame(m_currentFrame, x, y, flipX, tint);
}

void SpriteSheet::drawFrame(int frame, float x, float y, bool flipX, Color tint) const {
    if (!m_loaded) return;

    Rectangle src = frameRect(frame);
    if (flipX) src.width = -src.width;

    Rectangle dst = {x, y,
                     static_cast<float>(m_frameW),
                     static_cast<float>(m_frameH)};
    DrawTexturePro(m_texture, src, dst, {0,0}, 0.0f, tint);
}

const Animation* SpriteSheet::findAnim(const std::string& name) const {
    for (auto& a : m_animations) {
        if (a.name == name) return &a;
    }
    return nullptr;
}

// ─── Move semantics ───────────────────────────────────────────────────────────

SpriteSheet::SpriteSheet(SpriteSheet&& o) noexcept
    : m_texture(o.m_texture)
    , m_loaded(o.m_loaded)
    , m_frameW(o.m_frameW)
    , m_frameH(o.m_frameH)
    , m_cols(o.m_cols)
    , m_animations(std::move(o.m_animations))
    , m_currentAnim(o.m_currentAnim)
    , m_currentFrame(o.m_currentFrame)
    , m_frameTimer(o.m_frameTimer)
    , m_animDone(o.m_animDone)
{
    // Source no longer owns the texture
    o.m_loaded = false;
    o.m_texture = {};
}

SpriteSheet& SpriteSheet::operator=(SpriteSheet&& o) noexcept {
    if (this == &o) return *this;
    unload();  // release current texture
    m_texture     = o.m_texture;
    m_loaded      = o.m_loaded;
    m_frameW      = o.m_frameW;
    m_frameH      = o.m_frameH;
    m_cols        = o.m_cols;
    m_animations  = std::move(o.m_animations);
    m_currentAnim = o.m_currentAnim;
    m_currentFrame= o.m_currentFrame;
    m_frameTimer  = o.m_frameTimer;
    m_animDone    = o.m_animDone;
    o.m_loaded    = false;
    o.m_texture   = {};
    return *this;
}
