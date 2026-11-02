#include "VirtualGamepad.h"
#include <cmath>
#include <algorithm>

VirtualGamepad::VirtualGamepad(int screenW, int screenH)
    : m_screenW(screenW), m_screenH(screenH) {
    layoutButtons();
}

void VirtualGamepad::layoutButtons() {
    float w = static_cast<float>(m_screenW);
    float h = static_cast<float>(m_screenH);

    // Button size: ~12% of screen height
    float r = h * 0.10f;
    float pad = r * 1.5f;

    // ─── Left side: D-pad ─────────────────────────────────────────────────
    float dpadCX = w * 0.12f;
    float dpadCY = h * 0.70f;

    // LEFT
    m_buttons[BTN_LEFT].cx     = dpadCX - pad;
    m_buttons[BTN_LEFT].cy     = dpadCY;
    m_buttons[BTN_LEFT].radius = r;
    m_buttons[BTN_LEFT].label  = "<";

    // RIGHT
    m_buttons[BTN_RIGHT].cx     = dpadCX + pad;
    m_buttons[BTN_RIGHT].cy     = dpadCY;
    m_buttons[BTN_RIGHT].radius = r;
    m_buttons[BTN_RIGHT].label  = ">";

    // UP (jump)
    m_buttons[BTN_JUMP].cx     = dpadCX;
    m_buttons[BTN_JUMP].cy     = dpadCY - pad;
    m_buttons[BTN_JUMP].radius = r;
    m_buttons[BTN_JUMP].label  = "^";

    // DOWN (crouch)
    m_buttons[BTN_CROUCH].cx     = dpadCX;
    m_buttons[BTN_CROUCH].cy     = dpadCY + pad;
    m_buttons[BTN_CROUCH].radius = r;
    m_buttons[BTN_CROUCH].label  = "v";

    // ─── Right side: action buttons ───────────────────────────────────────
    float abcX = w * 0.85f;
    float abcY = h * 0.70f;

    // A = Action / Interact
    m_buttons[BTN_ACTION].cx     = abcX + pad;
    m_buttons[BTN_ACTION].cy     = abcY;
    m_buttons[BTN_ACTION].radius = r;
    m_buttons[BTN_ACTION].label  = "A";
    m_buttons[BTN_ACTION].color  = {80, 200, 80, 100};

    // B = Run
    m_buttons[BTN_RUN].cx     = abcX;
    m_buttons[BTN_RUN].cy     = abcY + pad * 0.5f;
    m_buttons[BTN_RUN].radius = r;
    m_buttons[BTN_RUN].label  = "B";
    m_buttons[BTN_RUN].color  = {200, 80, 80, 100};

    // C = Inventory
    m_buttons[BTN_INVENTORY].cx     = abcX - pad;
    m_buttons[BTN_INVENTORY].cy     = abcY;
    m_buttons[BTN_INVENTORY].radius = r;
    m_buttons[BTN_INVENTORY].label  = "C";
    m_buttons[BTN_INVENTORY].color  = {80, 80, 200, 100};

    // Start = Pause (top-centre)
    m_buttons[BTN_PAUSE].cx     = w * 0.50f;
    m_buttons[BTN_PAUSE].cy     = h * 0.06f;
    m_buttons[BTN_PAUSE].radius = r * 0.7f;
    m_buttons[BTN_PAUSE].label  = "||";
    m_buttons[BTN_PAUSE].color  = {200, 200, 200, 80};
}

bool VirtualGamepad::hitTest(const VirtualButton& btn, float sx, float sy) const {
    float dx = sx - btn.cx;
    float dy = sy - btn.cy;
    return (dx*dx + dy*dy) <= (btn.radius * btn.radius);
}

void VirtualGamepad::update(const TouchInput& touch) {
    // Clear one-shot flags
    for (auto& b : m_buttons) {
        b.justDown = false;
        b.justUp   = false;
    }

    // For each button, check all active touch points
    for (int btn = 0; btn < BTN_COUNT; ++btn) {
        bool wasPressed = m_buttons[btn].pressed;
        bool nowPressed = false;

        for (int i = 0; i < TouchInput::MAX_POINTS; ++i) {
            const TouchPoint& tp = touch.getPoint(i);
            if (tp.active && hitTest(m_buttons[btn], tp.x, tp.y)) {
                nowPressed = true;
                break;
            }
        }

        m_buttons[btn].pressed = nowPressed;
        if (!wasPressed && nowPressed) m_buttons[btn].justDown = true;
        if (wasPressed  && !nowPressed) m_buttons[btn].justUp  = true;
    }
}

void VirtualGamepad::render() const {
    for (int i = 0; i < BTN_COUNT; ++i) {
        drawButton(m_buttons[i]);
    }
}

void VirtualGamepad::drawButton(const VirtualButton& btn) const {
    Color col = btn.pressed ? btn.pressedColor : btn.color;
    // Apply global opacity
    col.a = static_cast<unsigned char>(col.a * m_opacity);

    DrawCircle(
        static_cast<int>(btn.cx),
        static_cast<int>(btn.cy),
        btn.radius,
        col
    );
    // Border
    Color border = {255, 255, 255, static_cast<unsigned char>(120 * m_opacity)};
    DrawCircleLines(
        static_cast<int>(btn.cx),
        static_cast<int>(btn.cy),
        btn.radius,
        border
    );
    // Label
    if (btn.label && btn.label[0]) {
        Color txtCol = {255, 255, 255, static_cast<unsigned char>(200 * m_opacity)};
        int fontSize = static_cast<int>(btn.radius * 0.6f);
        int textW = MeasureText(btn.label, fontSize);
        DrawText(
            btn.label,
            static_cast<int>(btn.cx) - textW / 2,
            static_cast<int>(btn.cy) - fontSize / 2,
            fontSize,
            txtCol
        );
    }
}

void VirtualGamepad::setOpacity(float alpha) {
    m_opacity = std::clamp(alpha, 0.1f, 1.0f);
}
