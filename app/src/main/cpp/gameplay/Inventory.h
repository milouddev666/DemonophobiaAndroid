#pragma once
#include <raylib.h>
#include "GameState.h"

/**
 * InventoryUI
 * -----------
 * Renders the inventory grid overlay.
 * Draw on top of gameplay scene when open.
 */
class InventoryUI {
public:
    InventoryUI() = default;

    void render(const GameState& state, int selectedSlot) const;

    // Returns slot index at screen position (virtual coords), or -1 if none
    int hitTest(float vx, float vy) const;

    bool isOpen() const { return m_open; }
    void open()   { m_open = true; }
    void close()  { m_open = false; }
    void toggle() { m_open = !m_open; }

    int  getSelectedSlot() const { return m_selectedSlot; }
    void setSelectedSlot(int s)  { m_selectedSlot = s; }

private:
    bool m_open          = false;
    int  m_selectedSlot  = -1;

    // Grid layout (virtual coords)
    static constexpr float PANEL_X     = 160.0f;
    static constexpr float PANEL_Y     = 120.0f;
    static constexpr float PANEL_W     = 320.0f;
    static constexpr float PANEL_H     = 240.0f;
    static constexpr float SLOT_SIZE   = 48.0f;
    static constexpr float SLOT_MARGIN = 8.0f;

    const char* itemName(uint8_t itemId) const;
    Color       itemColor(uint8_t itemId) const;
};
