#pragma once
#include <raylib.h>

/**
 * Camera2D wrapper for room scrolling.
 * Follows the player with screen-edge clamping to the room bounds.
 */
class GameCamera {
public:
    GameCamera() = default;

    void init(int roomPixelW, int roomPixelH);
    void follow(float targetX, float targetY);

    void beginDraw() const;
    void endDraw()   const;

    float getOffsetX() const { return m_cam.offset.x; }
    float getOffsetY() const { return m_cam.offset.y; }

    // World-to-screen
    Vector2 worldToScreen(float wx, float wy) const;
    // Screen-to-world
    Vector2 screenToWorld(float sx, float sy) const;

private:
    Camera2D m_cam{};
    int m_roomW = 0;
    int m_roomH = 0;
};
