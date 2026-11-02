#pragma once
#include <raylib.h>
#include "TouchInput.h"

/**
 * VirtualButton
 * -------------
 * Circular or rectangular zone on the screen that maps to a logical button.
 */
struct VirtualButton {
    float cx, cy;   // centre in screen pixels
    float radius;
    bool  pressed  = false;
    bool  justDown = false;
    bool  justUp   = false;
    Color color    = {255, 255, 255, 80};
    Color pressedColor = {255, 255, 255, 160};
    const char* label = "";
};

/**
 * VirtualGamepad
 * --------------
 * Renders and processes the on-screen touch controls.
 *
 * Layout (landscape, screen coords — not virtual):
 *   Left side:  D-pad (Left / Right / Up≡Jump / Down≡Crouch)
 *   Right side: A=Action/Interact, B=Run, C=Inventory, Start=Pause
 *
 * All positions are computed as fractions of screen size so they scale
 * to any screen resolution.
 */
class VirtualGamepad {
public:
    enum Button {
        BTN_LEFT      = 0,
        BTN_RIGHT     = 1,
        BTN_JUMP      = 2,
        BTN_CROUCH    = 3,
        BTN_ACTION    = 4,
        BTN_RUN       = 5,
        BTN_INVENTORY = 6,
        BTN_PAUSE     = 7,
        BTN_COUNT
    };

    VirtualGamepad(int screenW, int screenH);

    void update(const TouchInput& touch);
    void render() const;

    bool isPressed(Button btn)   const { return m_buttons[btn].pressed; }
    bool isJustDown(Button btn)  const { return m_buttons[btn].justDown; }
    bool isJustUp(Button btn)    const { return m_buttons[btn].justUp; }

    // Opacity control (0.0–1.0), saved to preferences
    void setOpacity(float alpha);
    float getOpacity() const { return m_opacity; }

private:
    int m_screenW, m_screenH;
    float m_opacity = 0.7f;
    VirtualButton m_buttons[BTN_COUNT];

    void layoutButtons();
    bool hitTest(const VirtualButton& btn, float sx, float sy) const;
    void drawButton(const VirtualButton& btn) const;
};
