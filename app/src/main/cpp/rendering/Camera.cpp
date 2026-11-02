#include "Camera.h"
#include <algorithm>

void GameCamera::init(int roomPixelW, int roomPixelH) {
    m_roomW = roomPixelW;
    m_roomH = roomPixelH;

    m_cam.zoom   = 1.0f;
    m_cam.rotation = 0.0f;
    // Centre of virtual screen
    m_cam.offset = {320.0f, 240.0f};
    m_cam.target = {320.0f, 240.0f};
}

void GameCamera::follow(float targetX, float targetY) {
    // Clamp target so camera doesn't show outside room
    float halfW = 320.0f / m_cam.zoom;
    float halfH = 240.0f / m_cam.zoom;

    float cx = std::clamp(targetX, halfW, static_cast<float>(m_roomW) - halfW);
    float cy = std::clamp(targetY, halfH, static_cast<float>(m_roomH) - halfH);

    // Smooth follow (lerp)
    float alpha = 0.12f;
    m_cam.target.x += (cx - m_cam.target.x) * alpha;
    m_cam.target.y += (cy - m_cam.target.y) * alpha;
}

void GameCamera::beginDraw() const {
    BeginMode2D(m_cam);
}

void GameCamera::endDraw() const {
    EndMode2D();
}

Vector2 GameCamera::worldToScreen(float wx, float wy) const {
    return GetWorldToScreen2D({wx, wy}, m_cam);
}

Vector2 GameCamera::screenToWorld(float sx, float sy) const {
    return GetScreenToWorld2D({sx, sy}, m_cam);
}
