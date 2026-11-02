#include "TouchInput.h"
#include <raylib.h>
#include <algorithm>

TouchInput::TouchInput(int screenW, int screenH)
    : m_screenW(screenW), m_screenH(screenH)
{
    m_points.fill({});
    m_prevActive.fill(false);
}

void TouchInput::update() {
    m_touchCount = GetTouchPointCount();
    if (m_touchCount > MAX_POINTS) m_touchCount = MAX_POINTS;

    for (int i = 0; i < MAX_POINTS; ++i) {
        bool wasActive = m_prevActive[i];
        bool nowActive = (i < m_touchCount);

        TouchPoint& p = m_points[i];
        p.prevX = p.x;
        p.prevY = p.y;

        if (nowActive) {
            Vector2 pos = GetTouchPosition(i);
            p.x = pos.x;
            p.y = pos.y;
        }

        p.active       = nowActive;
        p.justPressed  = (!wasActive && nowActive);
        p.justReleased = (wasActive  && !nowActive);

        m_prevActive[i] = nowActive;
    }

    // Zero out inactive points beyond current count
    for (int i = m_touchCount; i < MAX_POINTS; ++i) {
        bool wasActive = m_prevActive[i];
        m_points[i].justReleased = wasActive;
        m_points[i].justPressed  = false;
        m_points[i].active       = false;
    }
}

void TouchInput::onTouchEvent(int /*action*/, int /*pointerId*/, float /*x*/, float /*y*/) {
    // No-op: touch is read via Raylib's GetTouchPosition in update()
}

const TouchPoint& TouchInput::getPoint(int id) const {
    static const TouchPoint empty{};
    if (id < 0 || id >= MAX_POINTS) return empty;
    return m_points[id];
}
