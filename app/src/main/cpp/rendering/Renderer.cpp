#include "Renderer.h"
#include <algorithm>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Renderer", __VA_ARGS__)

Renderer::Renderer(int screenW, int screenH)
    : m_screenW(screenW), m_screenH(screenH)
{
    // Compute letterbox scale: fit VIRTUAL_W×VIRTUAL_H into screenW×screenH
    float scaleX = static_cast<float>(screenW) / VIRTUAL_W;
    float scaleY = static_cast<float>(screenH) / VIRTUAL_H;
    m_scale = std::min(scaleX, scaleY);

    int scaledW = static_cast<int>(VIRTUAL_W * m_scale);
    int scaledH = static_cast<int>(VIRTUAL_H * m_scale);
    m_offsetX = (screenW - scaledW) / 2;
    m_offsetY = (screenH - scaledH) / 2;

    LOGI("Renderer: screen=%dx%d virtual=%dx%d scale=%.2f offset=(%d,%d)",
         screenW, screenH, VIRTUAL_W, VIRTUAL_H,
         m_scale, m_offsetX, m_offsetY);
}

Renderer::~Renderer() {
    if (m_renderTarget.id != 0) {
        UnloadRenderTexture(m_renderTarget);
    }
}

void Renderer::init() {
    m_renderTarget = LoadRenderTexture(VIRTUAL_W, VIRTUAL_H);
    SetTextureFilter(m_renderTarget.texture, TEXTURE_FILTER_POINT);
    LOGI("Render target created: %dx%d", VIRTUAL_W, VIRTUAL_H);
}

void Renderer::beginFrame() {
    BeginTextureMode(m_renderTarget);
    ClearBackground(BLACK);
}

void Renderer::endFrame() {
    EndTextureMode();

    // Draw the render target to screen, flipping Y (Raylib RenderTexture is flipped)
    Rectangle src = {
        0.0f,
        0.0f,
        static_cast<float>(VIRTUAL_W),
        static_cast<float>(-VIRTUAL_H)   // negative = flip Y
    };
    Rectangle dst = {
        static_cast<float>(m_offsetX),
        static_cast<float>(m_offsetY),
        static_cast<float>(VIRTUAL_W) * m_scale,
        static_cast<float>(VIRTUAL_H) * m_scale
    };
    DrawTexturePro(m_renderTarget.texture, src, dst, {0,0}, 0.0f, WHITE);
}

float Renderer::virtualToScreenX(float vx) const {
    return vx * m_scale + m_offsetX;
}

float Renderer::virtualToScreenY(float vy) const {
    return vy * m_scale + m_offsetY;
}

float Renderer::screenToVirtualX(float sx) const {
    return (sx - m_offsetX) / m_scale;
}

float Renderer::screenToVirtualY(float sy) const {
    return (sy - m_offsetY) / m_scale;
}
