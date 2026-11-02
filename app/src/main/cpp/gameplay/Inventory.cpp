#include "Inventory.h"
#include <cstring>

void InventoryUI::render(const GameState& state, int selectedSlot) const {
    if (!m_open) return;

    // Semi-transparent panel
    DrawRectangle(
        static_cast<int>(PANEL_X - 10), static_cast<int>(PANEL_Y - 10),
        static_cast<int>(PANEL_W + 20), static_cast<int>(PANEL_H + 20),
        {0, 0, 0, 200}
    );
    DrawRectangleLinesEx(
        {PANEL_X - 10, PANEL_Y - 10, PANEL_W + 20, PANEL_H + 20},
        2.0f, {180, 180, 180, 255}
    );

    DrawText("INVENTORY", static_cast<int>(PANEL_X), static_cast<int>(PANEL_Y - 8),
             10, {200, 200, 200, 255});

    // 4-slot top row, 4-slot bottom row
    for (int i = 0; i < GameState::INVENTORY_SIZE; ++i) {
        int col = i % 4;
        int row = i / 4;
        float sx = PANEL_X + col * (SLOT_SIZE + SLOT_MARGIN);
        float sy = PANEL_Y + 20.0f + row * (SLOT_SIZE + SLOT_MARGIN);

        Color border = (i == selectedSlot) ? YELLOW : GRAY;
        DrawRectangle(
            static_cast<int>(sx), static_cast<int>(sy),
            static_cast<int>(SLOT_SIZE), static_cast<int>(SLOT_SIZE),
            {40, 40, 40, 220}
        );
        DrawRectangleLinesEx({sx, sy, SLOT_SIZE, SLOT_SIZE}, 2.0f, border);

        uint8_t itemId = state.inventory[i];
        if (itemId != ItemID::NONE) {
            Color ic = itemColor(itemId);
            DrawRectangle(
                static_cast<int>(sx + 4), static_cast<int>(sy + 4),
                static_cast<int>(SLOT_SIZE - 8), static_cast<int>(SLOT_SIZE - 8),
                ic
            );
            // Item name below icon
            const char* name = itemName(itemId);
            DrawText(name,
                     static_cast<int>(sx + 2),
                     static_cast<int>(sy + SLOT_SIZE - 12),
                     7, WHITE);
        }
    }

    // Info text for selected item
    if (selectedSlot >= 0 && selectedSlot < GameState::INVENTORY_SIZE) {
        uint8_t sel = state.inventory[selectedSlot];
        if (sel != ItemID::NONE) {
            DrawText(itemName(sel),
                     static_cast<int>(PANEL_X),
                     static_cast<int>(PANEL_Y + PANEL_H - 30),
                     12, WHITE);
        }
    }
}

int InventoryUI::hitTest(float vx, float vy) const {
    if (!m_open) return -1;
    for (int i = 0; i < GameState::INVENTORY_SIZE; ++i) {
        int col = i % 4;
        int row = i / 4;
        float sx = PANEL_X + col * (SLOT_SIZE + SLOT_MARGIN);
        float sy = PANEL_Y + 20.0f + row * (SLOT_SIZE + SLOT_MARGIN);
        if (vx >= sx && vx <= sx + SLOT_SIZE && vy >= sy && vy <= sy + SLOT_SIZE) {
            return i;
        }
    }
    return -1;
}

const char* InventoryUI::itemName(uint8_t itemId) const {
    switch (itemId) {
        case ItemID::RED_KEY:     return "Red Key";
        case ItemID::BLUE_KEY:    return "Blue Key";
        case ItemID::YELLOW_KEY:  return "Yellow Key";
        case ItemID::RUSTY_KEY:   return "Rusty Key";
        case ItemID::SCREWDRIVER: return "Screwdriver";
        case ItemID::HAMMER:      return "Hammer";
        case ItemID::WIRE_CUTTER: return "Wire Cutter";
        case ItemID::LIGHTER:     return "Lighter";
        case ItemID::KNIFE:       return "Knife";
        case ItemID::CROSS:       return "Cross";
        case ItemID::TALISMAN:    return "Talisman";
        case ItemID::MAP_PIECE_1: return "Map (1/2)";
        case ItemID::MAP_PIECE_2: return "Map (2/2)";
        default:                  return "???";
    }
}

Color InventoryUI::itemColor(uint8_t itemId) const {
    switch (itemId) {
        case ItemID::RED_KEY:     return {200, 50,  50,  255};
        case ItemID::BLUE_KEY:    return {50,  50,  200, 255};
        case ItemID::YELLOW_KEY:  return {200, 200, 50,  255};
        case ItemID::RUSTY_KEY:   return {140, 90,  50,  255};
        case ItemID::LIGHTER:     return {240, 140, 20,  255};
        case ItemID::CROSS:       return {220, 220, 220, 255};
        default:                  return {100, 100, 100, 255};
    }
}
