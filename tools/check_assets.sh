#!/usr/bin/env bash
# check_assets.sh — verify required assets are in place before building
ASSETS=app/src/main/assets
MISSING=0

check() {
    local path="$1"
    local note="$2"
    if [ ! -f "${ASSETS}/${path}" ]; then
        echo "  MISSING  ${path}  (${note})"
        MISSING=$((MISSING+1))
    else
        echo "  OK       ${path}"
    fi
}

echo "=== Asset check ==="
check "sprites/player.png"    "Player sprite sheet — 32x48 frames, 8 cols, 8+ rows"
check "sprites/enemy_0.png"   "Ghost enemy — 32x40 frames, 4+ cols"
check "sprites/enemy_1.png"   "Crawler enemy"
check "sprites/enemy_2.png"   "Shadow enemy"
check "audio/bgm_room00.ogg"  "Room 0 BGM (or .mp3)"
check "audio/sfx_death.wav"   "Death sound effect"
check "audio/sfx_pickup.wav"  "Item pickup sound"
check "maps/room00.bin"       "Room 0 map data (see docs/map_format.md)"

echo ""
if [ $MISSING -eq 0 ]; then
    echo "All assets present. Run: ./gradlew assembleDebug"
else
    echo "${MISSING} asset(s) missing."
    echo "The build will succeed but the game will use placeholder graphics/audio."
    echo "See README.md for asset requirements."
fi
