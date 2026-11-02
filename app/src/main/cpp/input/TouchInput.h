#pragma once
#include <array>

/**
 * TouchPoint — state for one active touch pointer.
 */
struct TouchPoint {
    bool  active       = false;
    float x            = 0.0f;
    float y            = 0.0f;
    float prevX        = 0.0f;
    float prevY        = 0.0f;
    bool  justPressed  = false;
    bool  justReleased = false;
};

/**
 * TouchInput
 * ----------
 * Polls Raylib's built-in touch API each frame.
 * Raylib on Android reads touch from AInputQueue natively.
 *
 * We track up to MAX_POINTS simultaneous touches, computing
 * justPressed/justReleased from frame-to-frame active changes.
 *
 * All coordinates are in SCREEN pixels (not virtual canvas pixels).
 * VirtualGamepad maps these to its button positions.
 * The Renderer provides screenToVirtual() for gameplay use.
 */
class TouchInput {
public:
    static constexpr int MAX_POINTS = 10;

    // Legacy constants kept for compatibility (not used with Raylib touch API)
    static constexpr int ACTION_DOWN = 0;
    static constexpr int ACTION_UP   = 1;
    static constexpr int ACTION_MOVE = 2;

    TouchInput(int screenW, int screenH);

    /**
     * Must be called once per frame (from render thread).
     * Reads Raylib touch state and updates all TouchPoints.
     */
    void update();

    /**
     * Legacy JNI path — no-op when using Raylib touch API.
     * Kept so Engine::onTouchEvent() compiles without changes.
     */
    void onTouchEvent(int action, int pointerId, float x, float y);

    const TouchPoint& getPoint(int id) const;
    int  getTouchCount()  const { return m_touchCount; }
    bool isAnyActive()    const { return m_touchCount > 0; }

    int screenWidth()  const { return m_screenW; }
    int screenHeight() const { return m_screenH; }

private:
    int m_screenW, m_screenH;
    int m_touchCount = 0;

    std::array<TouchPoint, MAX_POINTS> m_points{};
    std::array<bool,       MAX_POINTS> m_prevActive{};
};
